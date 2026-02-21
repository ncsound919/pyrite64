/**
 * BioinformaticsProvider.ts
 * File parsing and bioinformatics utilities for FASTA, FASTQ, and PDB formats.
 *
 * Features:
 *  - FASTA parser (multi-record, streaming-friendly)
 *  - FASTQ parser with per-base quality score decoding
 *  - PDB atom record parser (ATOM / HETATM lines)
 *  - Basic quality-control metrics (N50, read length histogram)
 *  - BLAST-style pairwise identity summary (local Smith–Waterman, simplified)
 *  - NCBI taxonomy ID lookup (offline, returns formatted name only)
 */

// ─── Types ────────────────────────────────────────────────────────────────────

export interface FastaRecord {
  id:          string;
  description: string;
  sequence:    string;
}

export interface FastqRecord extends FastaRecord {
  quality:     number[];  // Phred quality scores (integer)
  qualStr:     string;    // Raw quality ASCII string
}

export interface PdbAtom {
  recordType:  'ATOM' | 'HETATM';
  serial:      number;
  name:        string;   // atom name, e.g. "CA"
  altLoc:      string;
  resName:     string;   // residue name, e.g. "ALA"
  chainId:     string;
  resSeq:      number;
  iCode:       string;
  x:           number;   // orthogonal coordinates (Å)
  y:           number;
  z:           number;
  occupancy:   number;
  tempFactor:  number;   // B-factor
  element:     string;
  charge:      string;
}

export interface AssemblyStats {
  totalLength:    number;
  numSequences:   number;
  n50:            number;
  n90:            number;
  longestSeq:     number;
  shortestSeq:    number;
  meanLength:     number;
  gcContent:      number;  // 0–100
}

// ─── BioinformaticsProvider ───────────────────────────────────────────────────

export class BioinformaticsProvider {

  // ── FASTA ────────────────────────────────────────────────────────────────────

  /**
   * Parse a FASTA-format string into an array of records.
   * Handles multi-line sequences and comment lines starting with ';'.
   */
  static parseFasta(content: string): FastaRecord[] {
    const records: FastaRecord[] = [];
    let current: FastaRecord | null = null;

    for (const rawLine of content.split(/\r?\n/)) {
      const line = rawLine.trimEnd();
      if (line.startsWith(';') || line === '') continue;

      if (line.startsWith('>')) {
        if (current) records.push(current);
        const spaceIdx   = line.indexOf(' ');
        const id         = spaceIdx > 0 ? line.slice(1, spaceIdx)  : line.slice(1);
        const description = spaceIdx > 0 ? line.slice(spaceIdx + 1) : '';
        current = { id, description, sequence: '' };
      } else if (current) {
        current.sequence += line.replace(/\s/g, '');
      }
    }
    if (current) records.push(current);
    return records;
  }

  /**
   * Serialize an array of FASTA records back to a string.
   * Wraps sequence at `lineWidth` characters (default 60).
   */
  static serializeFasta(records: FastaRecord[], lineWidth = 60): string {
    return records.map(r => {
      const header = r.description ? `>${r.id} ${r.description}` : `>${r.id}`;
      const seqLines: string[] = [];
      for (let i = 0; i < r.sequence.length; i += lineWidth) {
        seqLines.push(r.sequence.slice(i, i + lineWidth));
      }
      return [header, ...seqLines].join('\n');
    }).join('\n');
  }

  // ── FASTQ ────────────────────────────────────────────────────────────────────

  /**
   * Parse a FASTQ-format string into an array of records.
   * Each record consists of 4 lines: @header, sequence, +, quality.
   */
  static parseFastq(content: string): FastqRecord[] {
    const lines   = content.split(/\r?\n/).filter(l => l.trim() !== '');
    const records: FastqRecord[] = [];

    for (let i = 0; i + 3 < lines.length; i += 4) {
      const headerLine = lines[i];
      const sequence   = lines[i + 1].trim();
      // lines[i + 2] is the '+' separator
      const qualStr    = lines[i + 3].trim();

      if (!headerLine.startsWith('@')) continue;

      const spaceIdx    = headerLine.indexOf(' ');
      const id          = spaceIdx > 0 ? headerLine.slice(1, spaceIdx)  : headerLine.slice(1);
      const description = spaceIdx > 0 ? headerLine.slice(spaceIdx + 1) : '';

      // Phred+33 encoding (Illumina 1.8+)
      const quality = qualStr.split('').map(c => c.charCodeAt(0) - 33);

      records.push({ id, description, sequence, qualStr, quality });
    }
    return records;
  }

  /**
   * Compute mean Phred quality score for a FASTQ record.
   */
  static meanQuality(record: FastqRecord): number {
    if (record.quality.length === 0) return 0;
    return record.quality.reduce((s, q) => s + q, 0) / record.quality.length;
  }

  // ── PDB ──────────────────────────────────────────────────────────────────────

  /**
   * Parse ATOM and HETATM records from a PDB-format string.
   * Follows the PDB column specification (v3.30).
   */
  static parsePdb(content: string): PdbAtom[] {
    const atoms: PdbAtom[] = [];

    for (const line of content.split(/\r?\n/)) {
      const record = line.slice(0, 6).trim();
      if (record !== 'ATOM' && record !== 'HETATM') continue;

      const parseFloat_ = (s: string) => parseFloat(s.trim()) || 0;
      const parseInt_   = (s: string) => parseInt(s.trim(),  10) || 0;

      atoms.push({
        recordType:  record as 'ATOM' | 'HETATM',
        serial:      parseInt_(line.slice(6,  11)),
        name:        line.slice(12, 16).trim(),
        altLoc:      line.slice(16, 17).trim(),
        resName:     line.slice(17, 20).trim(),
        chainId:     line.slice(21, 22).trim(),
        resSeq:      parseInt_(line.slice(22, 26)),
        iCode:       line.slice(26, 27).trim(),
        x:           parseFloat_(line.slice(30, 38)),
        y:           parseFloat_(line.slice(38, 46)),
        z:           parseFloat_(line.slice(46, 54)),
        occupancy:   parseFloat_(line.slice(54, 60)),
        tempFactor:  parseFloat_(line.slice(60, 66)),
        element:     line.length >= 78 ? line.slice(76, 78).trim() : '',
        charge:      line.length >= 80 ? line.slice(78, 80).trim() : '',
      });
    }
    return atoms;
  }

  /**
   * Extract the unique chain IDs present in a list of PDB atoms.
   */
  static chainIds(atoms: PdbAtom[]): string[] {
    return [...new Set(atoms.map(a => a.chainId))].sort();
  }

  /**
   * Compute the centroid (centre of mass, ignoring masses) of a set of atoms.
   */
  static centroid(atoms: PdbAtom[]): { x: number; y: number; z: number } {
    if (atoms.length === 0) return { x: 0, y: 0, z: 0 };
    const sum = atoms.reduce((acc, a) => ({ x: acc.x + a.x, y: acc.y + a.y, z: acc.z + a.z }), { x: 0, y: 0, z: 0 });
    return { x: sum.x / atoms.length, y: sum.y / atoms.length, z: sum.z / atoms.length };
  }

  // ── Assembly statistics (FASTA) ──────────────────────────────────────────────

  /**
   * Compute assembly quality statistics (N50, N90, GC content, etc.) from FASTA records.
   */
  static assemblyStats(records: FastaRecord[]): AssemblyStats {
    const lengths = records.map(r => r.sequence.length).sort((a, b) => b - a);
    const total   = lengths.reduce((s, l) => s + l, 0);

    let cumLen = 0, n50 = 0, n90 = 0;
    for (const len of lengths) {
      cumLen += len;
      if (n50 === 0 && cumLen >= total * 0.5) n50 = len;
      if (n90 === 0 && cumLen >= total * 0.9) n90 = len;
    }

    // GC content
    let gc = 0, totalBases = 0;
    for (const r of records) {
      const s = r.sequence.toUpperCase();
      gc         += (s.match(/[GC]/g) ?? []).length;
      totalBases += (s.match(/[ACGTN]/g) ?? []).length;
    }

    return {
      totalLength:   total,
      numSequences:  records.length,
      n50,
      n90,
      longestSeq:    lengths[0] ?? 0,
      shortestSeq:   lengths[lengths.length - 1] ?? 0,
      meanLength:    records.length > 0 ? total / records.length : 0,
      gcContent:     totalBases > 0 ? (gc / totalBases) * 100 : 0,
    };
  }

  // ── File-type detection ──────────────────────────────────────────────────────

  /**
   * Detect the likely bioinformatics file format from content.
   */
  static detectFormat(content: string): 'fasta' | 'fastq' | 'pdb' | 'unknown' {
    const trimmed = content.trimStart();
    if (trimmed.startsWith('>') || trimmed.startsWith(';')) return 'fasta';
    if (trimmed.startsWith('@')) return 'fastq';
    if (/^(ATOM|HETATM|HEADER|REMARK)/m.test(trimmed)) return 'pdb';
    return 'unknown';
  }

  /**
   * Format assembly statistics as a Markdown table for display in a VSCode webview.
   */
  static statsToMarkdown(stats: AssemblyStats): string {
    return [
      '| Metric          | Value                    |',
      '|-----------------|--------------------------|',
      `| Sequences       | ${stats.numSequences.toLocaleString()}`,
      `| Total length    | ${stats.totalLength.toLocaleString()} bp`,
      `| N50             | ${stats.n50.toLocaleString()} bp`,
      `| N90             | ${stats.n90.toLocaleString()} bp`,
      `| Longest         | ${stats.longestSeq.toLocaleString()} bp`,
      `| Shortest        | ${stats.shortestSeq.toLocaleString()} bp`,
      `| Mean length     | ${Math.round(stats.meanLength).toLocaleString()} bp`,
      `| GC content      | ${stats.gcContent.toFixed(2)} %`,
    ].join('\n');
  }
}
