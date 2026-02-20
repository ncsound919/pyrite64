import { NodeGraphNode, NodeGraphEdge, NodeGraphConfig } from './VibeNode.js';

/**
 * A Game Component is a pre-packaged set of logic (Node Graph) and properties
 * that can be applied to an entity to give it specific behavior instantly.
 */
export interface GameComponentTemplate {
  id:          string;
  label:       string;
  category:    'Movement' | 'Logic' | 'Combat' | 'Interaction' | 'UI';
  icon:        string; // Material symbol name
  description: string;
  
  /**
   * Generates the node graph patch to apply to the entity.
   * @param entityName The target entity name.
   */
  buildGraph: (entityName: string) => NodeGraphConfig;

  /**
   * Technical constraints or requirements (e.g. "Requires a Collision Body").
   */
  requirements?: string[];
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

let _idCounter = 1000;
const nextId = (): string => `node_${_idCounter++}`;

/** Creates a standard node config. */
const node = (type: string, x: number, y: number, data: Record<string, any> = {}): NodeGraphNode => ({
  id: nextId(),
  type,
  position: [x, y],
  data
});

/** Creates an edge between two nodes. */
const connect = (from: NodeGraphNode, fromPort: string, to: NodeGraphNode, toPort: string): NodeGraphEdge => ({
  from: from.id,
  fromPort: fromPort,
  to: to.id,
  toPort: toPort
});

// ─── Component Definitions ───────────────────────────────────────────────────

export const GAME_COMPONENTS: GameComponentTemplate[] = [

  // ─── MOVEMENT ──────────────────────────────────────────────────────────────
  {
    id: 'platformer-controller',
    label: 'Platformer Controller',
    category: 'Movement',
    icon: 'directions_run',
    description: 'Basic 2D/3D platformer movement with jump and gravity.',
    requirements: ['Collision Body (Dynamic)', 'Freeze Rotation'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];
      
      const start = node('OnTick', 50, 50);
      const stick = node('ReadStick', 250, 50, { stick: 'Left' });
      const move  = node('MoveDirection', 500, 50, { speed: 10, relative: 'Camera' });
      
      const btn   = node('OnButtonPress', 50, 200, { button: 'A' });
      const jump  = node('SetVelocity', 300, 200, { y: 15, mode: 'Add' }); // naive jump
      
      nodes.push(start, stick, move, btn, jump);
      
      edges.push(connect(start, 'out', stick, 'in'));
      edges.push(connect(stick, 'vec', move, 'direction'));
      edges.push(connect(stick, 'out', move, 'in'));
      edges.push(connect(btn, 'out', jump, 'in'));
      
      return { nodes, edges };
    }
  },

  {
    id: 'fps-controller',
    label: 'FPS Controller',
    category: 'Movement',
    icon: 'api',
    description: 'First-person movement with camera look.',
    requirements: ['Camera (Child)', 'Collision Body'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      // Movement
      const tick = node('OnTick', 50, 50);
      const move = node('MoveDirection', 300, 50, { speed: 8, relative: 'Self' });
      const look = node('ReadStick', 300, 200, { stick: 'Right' });
      const rot  = node('SetRotation', 550, 200, { axis: 'Y', speed: 2 });

      nodes.push(tick, move, look, rot);
      
      edges.push(connect(tick, 'out', move, 'in'));
      edges.push(connect(tick, 'out', look, 'in'));
      edges.push(connect(look, 'out', rot, 'in'));
      
      return { nodes, edges };
    }
  },

  // ─── INTERACTION ───────────────────────────────────────────────────────────
  {
    id: 'door-logic',
    label: 'Simple Door',
    category: 'Interaction',
    icon: 'sensor_door',
    description: 'Opens when player is near and presses interact.',
    requirements: ['IsTrigger (Proximity)'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const onEnter = node('OnCollide', 50, 50, { type: 'Enter', tag: 'Player' });
      const check   = node('OnButtonPress', 300, 50, { button: 'B' }); // Interact
      const anim    = node('PlayAnim', 550, 50, { anim: 'Open', loop: false });
      const sound   = node('PlaySound', 550, 150, { sound: 'DoorOpen' });

      nodes.push(onEnter, check, anim, sound);
      
      edges.push(connect(onEnter, 'out', check, 'in'));
      
      return { nodes, edges };
    }
  },

  {
    id: 'collectible-health',
    label: 'Health Pickup',
    category: 'Interaction',
    icon: 'favorite',
    description: 'Restores health and destroys self on touch.',
    requirements: ['IsTrigger'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const hit  = node('OnCollide', 50, 50, { tag: 'Player' });
      const heal = node('SetHealth', 300, 50, { amount: 25, mode: 'Add', target: 'Other' });
      const sfx  = node('PlaySound', 300, 150, { sound: 'Pickup' });
      const kill = node('Destroy', 550, 50, { target: 'Self', delay: 0.1 });

      nodes.push(hit, heal, sfx, kill);

      edges.push(connect(hit, 'out', heal, 'in'));
      edges.push(connect(hit, 'out', sfx, 'in'));
      edges.push(connect(heal, 'out', kill, 'in'));

      return { nodes, edges };
    }
  },

  // ─── LOGIC ─────────────────────────────────────────────────────────────────
  {
    id: 'patrol-ai',
    label: 'Patrol AI',
    category: 'Logic',
    icon: 'visibility',
    description: 'Moves between waypoints.',
    requirements: ['Waypoints (Children)'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const tick = node('OnTick', 50, 50);
      const move = node('MoveDirection', 300, 50, { speed: 4, direction: 'Forward' });
      
      const hit  = node('OnCollide', 50, 200, { tag: 'Wall' });
      const turn = node('SetRotation', 300, 200, { y: 180, mode: 'Add' });

      nodes.push(tick, move, hit, turn);
      edges.push(connect(tick, 'out', move, 'in'));
      edges.push(connect(hit, 'out', turn, 'in'));

      return { nodes, edges };
    }
  },

  // ─── UNITY-INSPIRED ────────────────────────────────────────────────────────

  {
    id: 'animator-state-machine',
    label: 'Animator (State Machine)',
    category: 'Movement',
    icon: 'account_tree',
    description: 'Unity Mecanim-style state machine: Idle → Walk → Run → Jump → Fall. Driven by speed and grounded state.',
    requirements: ['Animations: Idle, Walk, Run, Jump, Fall'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      // Read inputs every tick
      const tick   = node('OnTick', 50, 50);
      const stick  = node('ReadStick', 250, 50, { stick: 'Left' });
      const speed  = node('GetMagnitude', 450, 50);

      // State branches: grounded?
      const ground = node('IsGrounded', 250, 200);
      const bGnd   = node('Branch', 450, 200);

      // Ground states: idle or moving
      const bMove  = node('Branch', 650, 200, { threshold: 0.1 });
      const bRun   = node('Branch', 850, 200, { threshold: 0.6 });
      const idle   = node('PlayAnim', 1050, 100, { anim: 'Idle',  loop: true, blend: 0.15 });
      const walk   = node('PlayAnim', 1050, 200, { anim: 'Walk',  loop: true, blend: 0.15 });
      const run    = node('PlayAnim', 1050, 300, { anim: 'Run',   loop: true, blend: 0.15 });

      // Air states
      const vel    = node('GetVelocityY', 650, 350);
      const bAir   = node('Branch', 850, 350, { threshold: 0 });
      const jump   = node('PlayAnim', 1050, 380, { anim: 'Jump',  loop: false, blend: 0.1 });
      const fall   = node('PlayAnim', 1050, 460, { anim: 'Fall',  loop: true,  blend: 0.1 });

      // Jump input
      const btn    = node('OnButtonPress', 50, 480, { button: 'A' });
      const doJump = node('SetVelocity', 300, 480, { y: 14, mode: 'Set' });

      nodes.push(tick, stick, speed, ground, bGnd, bMove, bRun, idle, walk, run, vel, bAir, jump, fall, btn, doJump);

      edges.push(connect(tick,  'out', stick,  'in'));
      edges.push(connect(stick, 'vec', speed,  'vec'));
      edges.push(connect(tick,  'out', ground, 'in'));
      edges.push(connect(ground,'bool',bGnd,   'cond'));
      edges.push(connect(speed, 'f',   bMove,  'value'));
      edges.push(connect(bGnd,  'true',bMove,  'in'));
      edges.push(connect(bMove, 'false',idle,  'in'));
      edges.push(connect(speed, 'f',   bRun,   'value'));
      edges.push(connect(bMove, 'true', bRun,  'in'));
      edges.push(connect(bRun,  'false',walk,  'in'));
      edges.push(connect(bRun,  'true', run,   'in'));
      edges.push(connect(bGnd,  'false',vel,   'in'));
      edges.push(connect(vel,   'f',    bAir,  'value'));
      edges.push(connect(bAir,  'true', jump,  'in'));
      edges.push(connect(bAir,  'false',fall,  'in'));
      edges.push(connect(btn,   'out',  doJump,'in'));

      return { nodes, edges };
    }
  },

  {
    id: 'rigidbody-push',
    label: 'Rigidbody Interaction',
    category: 'Interaction',
    icon: 'open_with',
    description: 'Unity-style: push physics objects on contact. Applies an impulse proportional to the player\'s velocity.',
    requirements: ['Collision Body (Dynamic)', 'Tag: Pushable on target'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const hit    = node('OnCollide', 50, 50, { tag: 'Pushable', type: 'Stay' });
      const myVel  = node('GetVelocity', 300, 50, { target: 'Self' });
      const scale  = node('ScaleVec', 500, 50, { scale: 0.6 });
      const impulse= node('AddForce', 700, 50, { target: 'Other', mode: 'Impulse' });

      nodes.push(hit, myVel, scale, impulse);
      edges.push(connect(hit,    'out', myVel,   'in'));
      edges.push(connect(myVel,  'vec', scale,   'vec'));
      edges.push(connect(hit,    'out', impulse, 'in'));
      edges.push(connect(scale,  'vec', impulse, 'force'));

      return { nodes, edges };
    }
  },

  // ─── UNREAL-INSPIRED ───────────────────────────────────────────────────────

  {
    id: 'ai-perception-sight',
    label: 'AI Perception (Sight)',
    category: 'Logic',
    icon: 'remove_red_eye',
    description: 'Unreal AI Perception: line-of-sight check triggers chase or alert behaviors.',
    requirements: ['Tag: Player exists in scene'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const tick   = node('OnTick', 50, 50);
      const los    = node('CheckLineOfSight', 280, 50, { target: 'Player', maxDist: 12, fovDeg: 90 });
      const bSee   = node('Branch', 500, 50);

      // Seen: chase
      const chase  = node('MoveToward', 700, 50, { target: 'Player', speed: 5 });
      const atkAnim= node('PlayAnim', 900, 50, { anim: 'Chase', loop: true, blend: 0.2 });
      const sfxAlert=node('PlaySound', 900, 150, { sound: 'Alert', oneshot: true });

      // Not seen: resume patrol
      const patrol = node('MoveDirection', 700, 200, { speed: 2, direction: 'Forward' });
      const idleA  = node('PlayAnim', 900, 250, { anim: 'Walk', loop: true, blend: 0.2 });

      nodes.push(tick, los, bSee, chase, atkAnim, sfxAlert, patrol, idleA);

      edges.push(connect(tick,    'out',   los,    'in'));
      edges.push(connect(los,     'bool',  bSee,   'cond'));
      edges.push(connect(bSee,    'true',  chase,  'in'));
      edges.push(connect(chase,   'out',   atkAnim,'in'));
      edges.push(connect(bSee,    'true',  sfxAlert,'in'));
      edges.push(connect(bSee,    'false', patrol, 'in'));
      edges.push(connect(patrol,  'out',   idleA,  'in'));

      return { nodes, edges };
    }
  },

  {
    id: 'blueprint-spawner',
    label: 'Blueprint Spawner',
    category: 'Logic',
    icon: 'add_circle',
    description: 'Unreal Blueprint-style: spawn an entity prefab at a socket on a trigger event.',
    requirements: ['SpawnSocket (Child node named "SpawnPoint")'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const btn    = node('OnButtonPress', 50, 50, { button: 'B' });
      const timer  = node('Timer', 250, 50, { duration: 0.5, repeat: false });
      const spawn  = node('Spawn', 500, 50, { prefab: 'Projectile', socket: 'SpawnPoint', pooled: true });
      const vel    = node('SetVelocity', 750, 50, { z: 20, mode: 'Set', target: 'Spawned' });
      const sfx    = node('PlaySound', 750, 150, { sound: 'Shoot' });
      const lim    = node('Timer', 750, 250, { duration: 3.0, repeat: false });
      const dest   = node('Destroy', 950, 250, { target: 'Spawned', delay: 0 });

      nodes.push(btn, timer, spawn, vel, sfx, lim, dest);
      edges.push(connect(btn,   'out',     timer,  'in'));
      edges.push(connect(timer, 'elapsed', spawn,  'in'));
      edges.push(connect(spawn, 'out',     vel,    'in'));
      edges.push(connect(spawn, 'out',     sfx,    'in'));
      edges.push(connect(spawn, 'out',     lim,    'in'));
      edges.push(connect(lim,   'elapsed', dest,   'in'));

      return { nodes, edges };
    }
  },

  // ─── GODOT-INSPIRED ────────────────────────────────────────────────────────

  {
    id: 'area-detector',
    label: 'Area Detector (Signals)',
    category: 'Logic',
    icon: 'radar',
    description: 'Godot Area3D-style: emit "entered" / "exited" signals when entities overlap this zone. Other nodes can subscribe.',
    requirements: ['IsTrigger (Volume)'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const enter  = node('OnCollide', 50, 50,  { type: 'Enter' });
      const exit   = node('OnCollide', 50, 200, { type: 'Exit'  });
      const sigIn  = node('EmitSignal', 300, 50,  { signal: `${ent}.area_entered`, payload: 'Other' });
      const sigOut = node('EmitSignal', 300, 200, { signal: `${ent}.area_exited`,  payload: 'Other' });
      const sfxIn  = node('PlaySound', 550, 50,  { sound: 'AreaEnter' });
      const sfxOut = node('PlaySound', 550, 200, { sound: 'AreaExit'  });

      nodes.push(enter, exit, sigIn, sigOut, sfxIn, sfxOut);
      edges.push(connect(enter, 'out', sigIn,  'in'));
      edges.push(connect(sigIn, 'out', sfxIn,  'in'));
      edges.push(connect(exit,  'out', sigOut, 'in'));
      edges.push(connect(sigOut,'out', sfxOut, 'in'));

      return { nodes, edges };
    }
  },

  {
    id: 'tween-mover',
    label: 'Tween Mover',
    category: 'Movement',
    icon: 'swap_horiz',
    description: 'Godot Tween-style: smoothly interpolate position between two points on a trigger, with configurable easing.',
    requirements: ['Two child nodes named "PointA" and "PointB"'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const btn    = node('OnButtonPress', 50, 50, { button: 'B' });
      const getPosA= node('GetChildPos', 280, 50,  { child: 'PointA' });
      const getPosB= node('GetChildPos', 280, 150, { child: 'PointB' });
      const tween  = node('Tween', 520, 100, { duration: 1.2, easing: 'EaseInOut', property: 'position' });
      const sfxDone= node('PlaySound', 760, 100, { sound: 'TweenDone' });

      nodes.push(btn, getPosA, getPosB, tween, sfxDone);
      edges.push(connect(btn,    'out',  getPosA, 'in'));
      edges.push(connect(btn,    'out',  getPosB, 'in'));
      edges.push(connect(getPosA,'pos',  tween,   'from'));
      edges.push(connect(getPosB,'pos',  tween,   'to'));
      edges.push(connect(btn,    'out',  tween,   'in'));
      edges.push(connect(tween,  'done', sfxDone, 'in'));

      return { nodes, edges };
    }
  },

  // ─── COMBAT ────────────────────────────────────────────────────────────────

  {
    id: 'melee-attack',
    label: 'Melee Attack',
    category: 'Combat',
    icon: 'sports_martial_arts',
    description: 'Press attack button → play swing animation → hitbox active mid-swing → apply damage on hit.',
    requirements: ['Animations: Attack', 'HitboxVolume (Child)'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const btn    = node('OnButtonPress', 50, 50, { button: 'B' });
      const anim   = node('PlayAnim', 280, 50, { anim: 'Attack', loop: false, blend: 0.05 });
      const wait   = node('WaitFrames', 500, 50, { frames: 8 });
      const hbOn   = node('SetCollider', 700, 50,  { child: 'Hitbox', enabled: true });
      const hit    = node('OnCollide', 50, 200, { tag: 'Enemy', source: 'Hitbox' });
      const dmg    = node('SetHealth', 300, 200, { amount: -20, mode: 'Add', target: 'Other' });
      const sfx    = node('PlaySound', 300, 300, { sound: 'Hit' });
      const vfx    = node('PlayVFX', 500, 200, { vfx: 'HitSpark', socket: 'Other' });
      const waitOff= node('WaitFrames', 700, 200, { frames: 6 });
      const hbOff  = node('SetCollider', 900, 200, { child: 'Hitbox', enabled: false });

      nodes.push(btn, anim, wait, hbOn, hit, dmg, sfx, vfx, waitOff, hbOff);
      edges.push(connect(btn,     'out',     anim,    'in'));
      edges.push(connect(anim,    'out',     wait,    'in'));
      edges.push(connect(wait,    'elapsed', hbOn,    'in'));
      edges.push(connect(hit,     'out',     dmg,     'in'));
      edges.push(connect(hit,     'out',     sfx,     'in'));
      edges.push(connect(dmg,     'out',     vfx,     'in'));
      edges.push(connect(anim,    'out',     waitOff, 'in'));
      edges.push(connect(waitOff, 'elapsed', hbOff,   'in'));

      return { nodes, edges };
    }
  },

  {
    id: 'health-damage-system',
    label: 'Health & Damage System',
    category: 'Combat',
    icon: 'monitor_heart',
    description: 'Full health system with damage intake, invincibility frames, death handling, and respawn.',
    requirements: ['Tag: Enemy or Player on self', 'Animations: Hurt, Die'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      // Take damage path
      const hit     = node('OnCollide', 50, 50,  { tag: 'DamageSource' });
      const checkInvincible = node('CheckFlag', 280, 50,  { flag: 'Invincible', expected: false });
      const dmg     = node('SetHealth', 500, 50,  { amount: -10, mode: 'Add', target: 'Self' });
      const hurt    = node('PlayAnim', 700, 50,   { anim: 'Hurt', loop: false, blend: 0.05 });
      const sfxHurt = node('PlaySound', 700, 150, { sound: 'Hurt' });
      const setInv  = node('SetFlag', 900, 50,    { flag: 'Invincible', value: true });
      const timer   = node('Timer', 900, 150,     { duration: 1.5, repeat: false });
      const clrInv  = node('SetFlag', 1100, 150,  { flag: 'Invincible', value: false });

      // Death path
      const hp      = node('GetHealth', 50, 300,  { target: 'Self' });
      const dead    = node('Branch', 280, 300,    { threshold: 0, op: 'LessEqual' });
      const dieAnim = node('PlayAnim', 500, 300,  { anim: 'Die', loop: false, blend: 0.05 });
      const sfxDie  = node('PlaySound', 500, 400, { sound: 'Die' });
      const respawn = node('Timer', 700, 300,     { duration: 3.0, repeat: false });
      const reload  = node('LoadScene', 900, 300, { scene: 'Current', resetHealth: true });

      nodes.push(hit, checkInvincible, dmg, hurt, sfxHurt, setInv, timer, clrInv, hp, dead, dieAnim, sfxDie, respawn, reload);
      edges.push(connect(hit,            'out',     checkInvincible, 'in'));
      edges.push(connect(checkInvincible,'out',     dmg,     'in'));
      edges.push(connect(dmg,     'out',     hurt,    'in'));
      edges.push(connect(dmg,     'out',     sfxHurt, 'in'));
      edges.push(connect(dmg,     'out',     setInv,  'in'));
      edges.push(connect(setInv,  'out',     timer,   'in'));
      edges.push(connect(timer,   'elapsed', clrInv,  'in'));
      edges.push(connect(dmg,     'out',     hp,      'in'));
      edges.push(connect(hp,      'f',       dead,    'value'));
      edges.push(connect(dead,    'true',    dieAnim, 'in'));
      edges.push(connect(dead,    'true',    sfxDie,  'in'));
      edges.push(connect(dieAnim, 'out',     respawn, 'in'));
      edges.push(connect(respawn, 'elapsed', reload,  'in'));

      return { nodes, edges };
    }
  },

  // ─── UI ────────────────────────────────────────────────────────────────────

  {
    id: 'hud-health-bar',
    label: 'HUD Health Bar',
    category: 'UI',
    icon: 'health_and_safety',
    description: 'Overlay health bar that updates whenever the entity\'s health changes. Shows numeric value and color-coded bar.',
    requirements: ['Health system on entity'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      const tick   = node('OnTick', 50, 50);
      const hp     = node('GetHealth', 280, 50, { target: ent });
      const maxHp  = node('GetProperty', 480, 50, { target: ent, prop: 'maxHealth' });
      const ratio  = node('Divide', 680, 50);
      const hudBar = node('SetHUDBar', 880, 50, { element: 'HealthBar', colorLow: '#e84040', colorHigh: '#40e860' });
      const hudNum = node('SetHUDText', 880, 150, { element: 'HealthNum', format: '{0}/{1}' });

      nodes.push(tick, hp, maxHp, ratio, hudBar, hudNum);
      edges.push(connect(tick,  'out', hp,     'in'));
      edges.push(connect(tick,  'out', maxHp,  'in'));
      edges.push(connect(hp,    'f',   ratio,  'a'));
      edges.push(connect(maxHp, 'f',   ratio,  'b'));
      edges.push(connect(ratio, 'f',   hudBar, 'ratio'));
      edges.push(connect(hp,    'f',   hudNum, 'arg0'));
      edges.push(connect(maxHp, 'f',   hudNum, 'arg1'));

      return { nodes, edges };
    }
  },

  {
    id: 'hud-score-counter',
    label: 'Score Counter',
    category: 'UI',
    icon: 'stars',
    description: 'Increments score on item pickup or enemy defeat and refreshes the HUD score display.',
    requirements: ['ScoreManager singleton in scene'],
    buildGraph: (ent) => {
      const nodes: NodeGraphNode[] = [];
      const edges: NodeGraphEdge[] = [];

      // Triggered by collecting or defeating enemies
      const collect = node('OnCollide', 50, 50,  { tag: 'Collectible' });
      const defeat  = node('OnSignal',  50, 200, { signal: 'enemy.defeated' });
      const pts1    = node('SetProperty', 300, 50,  { target: 'ScoreManager', prop: 'score', amount: 10, mode: 'Add' });
      const pts2    = node('SetProperty', 300, 200, { target: 'ScoreManager', prop: 'score', amount: 100, mode: 'Add' });
      const score   = node('GetProperty', 550, 100, { target: 'ScoreManager', prop: 'score' });
      const hud     = node('SetHUDText',  750, 100, { element: 'ScoreLabel', format: 'SCORE {0}' });
      const sfx1    = node('PlaySound', 300, 150,   { sound: 'Coin' });
      const sfx2    = node('PlaySound', 300, 300,   { sound: 'Defeat' });

      nodes.push(collect, defeat, pts1, pts2, score, hud, sfx1, sfx2);
      edges.push(connect(collect, 'out', pts1,  'in'));
      edges.push(connect(collect, 'out', sfx1,  'in'));
      edges.push(connect(defeat,  'out', pts2,  'in'));
      edges.push(connect(defeat,  'out', sfx2,  'in'));
      edges.push(connect(pts1,    'out', score, 'in'));
      edges.push(connect(pts2,    'out', score, 'in'));
      edges.push(connect(score,   'f',   hud,   'arg0'));

      return { nodes, edges };
    }
  }
];

export function getGroupedComponents(): Record<string, GameComponentTemplate[]> {
  const groups: Record<string, GameComponentTemplate[]> = {};
  for (const c of GAME_COMPONENTS) {
    if (!groups[c.category]) groups[c.category] = [];
    groups[c.category].push(c);
  }
  return groups;
}
