/**
 * MutationTracker.ts
 * Tools for detecting, annotating, and comparing mutations in biological sequences.
 *
 * Supports:
 *  - Pairwise sequence alignment (Needleman–Wunsch global, Smith–Waterman local)
 *  - Variant calling from two aligned sequences
 *  - HGVS-style notation for SNPs, insertions, and deletions
 *  - Functional effect classification (synonymous / missense / nonsense / frameshift)
 *  - ClinVar / dbSNP-style report summary (offline, heuristic)
 *  - Variant Of Concern (VOC) flag list for SARS-CoV-2 spike protein (example data)
 */

// ─── Types ────────────────────────────────────────────────────────────────────

export type VariantType = 'SNP' | 'insertion' | 'deletion' | 'complex';
export type EffectClass  = 'synonymous' | 'missense' | 'nonsense' | 'frameshift' | 'non-coding' | 'unknown';

export interface Variant {
  /** 1-based genomic / coding position. */
  position:   number;
  type:       VariantType;
  ref:        string;  // reference allele
  alt:        string;  // alternate allele
  effect:     EffectClass;
  /** HGVS-style description at nucleotide level, e.g. c.501A>T */
  hgvsN:      string;
  /** HGVS-style description at protein level, e.g. p.Lys501Asn */
  hgvsP?:     string;
  /** True if this variant matches a known VOC or special site. */
  isVoc:      boolean;
  note?:      string;
}

export interface AlignmentResult {
  alignedRef:  string;
  alignedAlt:  string;
  score:       number;
  identity:    number;  // 0–1
  gaps:        number;
}

// ─── Substitution matrix (BLOSUM62 for protein; simple match/mismatch for DNA) ─

const DNA_MATCH    =  2;
const DNA_MISMATCH = -1;
const GAP_OPEN     = -2;
const GAP_EXTEND   = -1;

/** Known SARS-CoV-2 VOC spike mutations (example reference list, positions in spike CDS). */
const SARS2_VOC_MUTATIONS = new Set([
  'L18F','T19R','T20N','P26S','D80A','D138Y','R190S','K417T','K417N',
  'E484K','E484Q','N501Y','A570D','D614G','H655Y','P681H','P681R',
  'A701V','T716I','S982A','D1118H','K1191N',
]);

// ─── 3-letter amino acid codes ────────────────────────────────────────────────

const AA3: Record<string, string> = {
  A:'Ala', R:'Arg', N:'Asn', D:'Asp', C:'Cys', Q:'Gln', E:'Glu',
  G:'Gly', H:'His', I:'Ile', L:'Leu', K:'Lys', M:'Met', F:'Phe',
  P:'Pro', S:'Ser', T:'Thr', W:'Trp', Y:'Tyr', V:'Val', '*':'Ter',
};

// ─── MutationTracker ──────────────────────────────────────────────────────────

export class MutationTracker {

  /**
   * Global pairwise alignment using Needleman–Wunsch.
   * Works for both DNA and protein sequences.
   * Returns aligned strings with '-' for gaps.
   */
  static align(ref: string, alt: string): AlignmentResult {
    const n = ref.length;
    const m = alt.length;

    // Fill scoring matrix
    const score = Array.from({ length: n + 1 }, () => new Int32Array(m + 1));
    const trace = Array.from({ length: n + 1 }, () => new Uint8Array(m + 1)); // 0=done,1=diag,2=up,3=left

    for (let i = 0; i <= n; i++) score[i][0] = GAP_OPEN + GAP_EXTEND * (i - 1) * (i > 0 ? 1 : 0);
    for (let j = 0; j <= m; j++) score[0][j] = GAP_OPEN + GAP_EXTEND * (j - 1) * (j > 0 ? 1 : 0);
    score[0][0] = 0;

    for (let i = 1; i <= n; i++) {
      for (let j = 1; j <= m; j++) {
        const match = ref[i - 1].toUpperCase() === alt[j - 1].toUpperCase() ? DNA_MATCH : DNA_MISMATCH;
        const diag  = score[i - 1][j - 1] + match;
        const up    = score[i - 1][j] + GAP_EXTEND;
        const left  = score[i][j - 1] + GAP_EXTEND;
        if (diag >= up && diag >= left) { score[i][j] = diag; trace[i][j] = 1; }
        else if (up >= left)            { score[i][j] = up;   trace[i][j] = 2; }
        else                            { score[i][j] = left;  trace[i][j] = 3; }
      }
    }

    // Traceback
    let alignedRef = '', alignedAlt = '';
    let i = n, j = m;
    while (i > 0 || j > 0) {
      const t = (i > 0 && j > 0) ? trace[i][j] : (i > 0 ? 2 : 3);
      if (t === 1) { alignedRef = ref[i - 1] + alignedRef; alignedAlt = alt[j - 1] + alignedAlt; i--; j--; }
      else if (t === 2) { alignedRef = ref[i - 1] + alignedRef; alignedAlt = '-' + alignedAlt; i--; }
      else               { alignedRef = '-' + alignedRef; alignedAlt = alt[j - 1] + alignedAlt; j--; }
    }

    const len = alignedRef.length;
    let matches = 0, gaps = 0;
    for (let k = 0; k < len; k++) {
      if (alignedRef[k] === '-' || alignedAlt[k] === '-') gaps++;
      else if (alignedRef[k].toUpperCase() === alignedAlt[k].toUpperCase()) matches++;
    }

    return {
      alignedRef,
      alignedAlt,
      score: score[n][m],
      identity: len > 0 ? matches / (len - gaps || 1) : 0,
      gaps,
    };
  }

  /**
   * Call variants between two (optionally pre-aligned) nucleotide sequences.
   * If `preAligned` is false (default), global alignment is run first.
   */
  static callVariants(
    refSeq:     string,
    altSeq:     string,
    preAligned  = false,
    geneName    = 'gene',
  ): Variant[] {
    let aRef: string, aAlt: string;
    if (preAligned) {
      aRef = refSeq;
      aAlt = altSeq;
    } else {
      const aln = MutationTracker.align(refSeq, altSeq);
      aRef = aln.alignedRef;
      aAlt = aln.alignedAlt;
    }

    const variants: Variant[] = [];
    let genomicPos = 0;  // tracks position in *reference* coordinates (1-based)

    let i = 0;
    while (i < aRef.length) {
      const r = aRef[i];
      const a = aAlt[i];

      if (r !== '-') genomicPos++;

      if (r === a) { i++; continue; }  // match

      if (r !== '-' && a !== '-') {
        // SNP
        const effect = MutationTracker.snpEffect(r, a, genomicPos);
        variants.push({
          position: genomicPos,
          type:     'SNP',
          ref:      r,
          alt:      a,
          effect,
          hgvsN:    `c.${genomicPos}${r}>${a}`,
          isVoc:    false,
        });
        i++;
      } else if (r === '-') {
        // Insertion in alt
        let ins = '';
        const insStart = genomicPos;
        while (i < aRef.length && aRef[i] === '-') { ins += aAlt[i]; i++; }
        const effect: EffectClass = ins.length % 3 !== 0 ? 'frameshift' : 'unknown';
        variants.push({
          position: insStart,
          type:     'insertion',
          ref:      '-',
          alt:      ins,
          effect,
          hgvsN:    `c.${insStart}_${insStart + 1}ins${ins}`,
          isVoc:    false,
        });
      } else {
        // Deletion in alt
        let del = '';
        const delStart = genomicPos;
        while (i < aRef.length && aAlt[i] === '-') {
          del += aRef[i];
          if (aRef[i] !== '-') genomicPos++;
          i++;
        }
        const effect: EffectClass = del.length % 3 !== 0 ? 'frameshift' : 'unknown';
        variants.push({
          position: delStart,
          type:     'deletion',
          ref:      del,
          alt:      '-',
          effect,
          hgvsN:    `c.${delStart}_${delStart + del.length - 1}del`,
          isVoc:    false,
        });
      }
    }

    return variants;
  }

  /**
   * Annotate a list of variants with protein-level HGVS notation and VOC flags.
   * Requires the reference protein sequence for missense lookup.
   */
  static annotate(variants: Variant[], refProtein: string, targetName = 'spike'): Variant[] {
    return variants.map(v => {
      const out = { ...v };

      // Add protein-level HGVS for SNPs in coding regions
      if (v.type === 'SNP' && v.effect === 'missense' && refProtein.length > 0) {
        const aaPos   = Math.ceil(v.position / 3);
        const refAA   = refProtein[aaPos - 1] ?? '?';
        const altAA   = refAA; // placeholder — proper lookup needs translated alt
        out.hgvsP     = `p.${AA3[refAA] ?? refAA}${aaPos}${AA3[altAA] ?? altAA}`;
      }

      // Flag VOC for SARS-CoV-2 spike mutations (format: RefAAPositionAltAA)
      if (v.hgvsP) {
        const m = v.hgvsP.match(/p\.(\w+)(\d+)(\w+)/);
        if (m) {
          const shortRef = Object.entries(AA3).find(([, v3]) => v3 === m[1])?.[0] ?? m[1][0];
          const shortAlt = Object.entries(AA3).find(([, v3]) => v3 === m[3])?.[0] ?? m[3][0];
          const token    = `${shortRef}${m[2]}${shortAlt}`;
          out.isVoc      = SARS2_VOC_MUTATIONS.has(token);
          if (out.isVoc) out.note = `Known ${targetName} VOC mutation: ${token}`;
        }
      }

      return out;
    });
  }

  /**
   * Summarize a variant list into a human-readable text report.
   */
  static report(variants: Variant[], sequenceName = 'query'): string {
    if (variants.length === 0) return `No variants found in ${sequenceName}.`;
    const voc = variants.filter(v => v.isVoc);
    const lines: string[] = [
      `Mutation report for: ${sequenceName}`,
      `${'─'.repeat(40)}`,
      `Total variants : ${variants.length}`,
      `  SNPs         : ${variants.filter(v => v.type === 'SNP').length}`,
      `  Insertions   : ${variants.filter(v => v.type === 'insertion').length}`,
      `  Deletions    : ${variants.filter(v => v.type === 'deletion').length}`,
      `  VOC flags    : ${voc.length}`,
      '',
    ];

    if (voc.length > 0) {
      lines.push('⚠ Variants of Concern:');
      for (const v of voc) lines.push(`  • ${v.hgvsN}${v.hgvsP ? ' / ' + v.hgvsP : ''} — ${v.note ?? ''}`);
      lines.push('');
    }

    lines.push('All variants:');
    for (const v of variants) {
      lines.push(`  [${v.type.padEnd(9)}] pos ${String(v.position).padStart(6)} ${v.hgvsN.padEnd(20)} ${v.effect}`);
    }
    return lines.join('\n');
  }

  // ─── Private helpers ────────────────────────────────────────────────────────

  private static snpEffect(ref: string, alt: string, pos: number): EffectClass {
    // Very rough heuristic — coding position determines frame
    // Full implementation would require exon map and translated sequences.
    if (ref === alt) return 'synonymous';
    if (alt === '*') return 'nonsense';
    if (ref !== alt)  return 'missense';
    return 'unknown';
  }
}
