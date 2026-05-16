// conductor.jsx — main app

const { useState, useEffect, useRef, useMemo } = React;

// ─────────────────────────────────────────────────────────────────────────────
// DATA

const FAMILIES = {
  strings: { zh: '弦樂',     en: 'Strings',     accent: '#e8a96a' },
  wood:    { zh: '木管',     en: 'Woodwinds',   accent: '#7fc6a3' },
  brass:   { zh: '銅管',     en: 'Brass',       accent: '#c98c3c' },
  perc:    { zh: '打擊',     en: 'Percussion',  accent: '#c87878' },
  guitar:  { zh: '吉他',     en: 'Guitar',      accent: '#e0744a' },
  bass:    { zh: '貝斯',     en: 'Bass',        accent: '#9a6ce8' },
  keys:    { zh: '鍵盤',     en: 'Keys',        accent: '#6abce8' },
  drums:   { zh: '鼓組',     en: 'Drums',       accent: '#d04848' },
  synth:   { zh: '合成器',   en: 'Synth',       accent: '#48d0c0' },
  vox:     { zh: '人聲',     en: 'Voice',       accent: '#cf6a4f' },
};

// Catalog — all available instruments, keyed by id.
// Each entry has zh/en label, family, default count, and an icon glyph.
const CATALOG = {
  // Symphony
  violin1:  { zh: '第一小提琴', en: 'Violin I',    family: 'strings', count: 8, icon: 'V' },
  violin2:  { zh: '第二小提琴', en: 'Violin II',   family: 'strings', count: 6, icon: 'V' },
  viola:    { zh: '中提琴',     en: 'Viola',       family: 'strings', count: 5, icon: 'V' },
  cello:    { zh: '大提琴',     en: 'Cello',       family: 'strings', count: 5, icon: 'C' },
  contra:   { zh: '低音提琴',   en: 'Contrabass',  family: 'strings', count: 3, icon: 'B' },
  flute:    { zh: '長笛',       en: 'Flute',       family: 'wood',    count: 2, icon: 'F' },
  oboe:     { zh: '雙簧管',     en: 'Oboe',        family: 'wood',    count: 2, icon: 'O' },
  clar:     { zh: '單簧管',     en: 'Clarinet',    family: 'wood',    count: 2, icon: 'Cl' },
  bsn:      { zh: '巴松管',     en: 'Bassoon',     family: 'wood',    count: 2, icon: 'Bn' },
  horn:     { zh: '法國號',     en: 'Horn',        family: 'brass',   count: 4, icon: 'H' },
  tpt:      { zh: '小號',       en: 'Trumpet',     family: 'brass',   count: 3, icon: 'T' },
  tbn:      { zh: '長號',       en: 'Trombone',    family: 'brass',   count: 3, icon: 'Tb' },
  tba:      { zh: '低音號',     en: 'Tuba',        family: 'brass',   count: 1, icon: 'Tu' },
  harp:     { zh: '豎琴',       en: 'Harp',        family: 'perc',    count: 1, icon: 'H' },
  timp:     { zh: '定音鼓',     en: 'Timpani',     family: 'perc',    count: 1, icon: 'Ti' },
  perc:     { zh: '打擊組',     en: 'Percussion',  family: 'perc',    count: 3, icon: 'P' },
  choir:    { zh: '合唱團',     en: 'Choir',       family: 'vox',     count: 12, icon: 'Ch' },

  // Rock band
  drumkit:  { zh: '爵士鼓',     en: 'Drum Kit',    family: 'drums',   count: 1, icon: 'D' },
  ebass:    { zh: '電貝斯',     en: 'Bass Guitar', family: 'bass',    count: 1, icon: 'B' },
  egtr:     { zh: '電吉他',     en: 'E-Guitar',    family: 'guitar',  count: 1, icon: 'G' },
  rhythm:   { zh: '節奏吉他',   en: 'Rhythm Gtr',  family: 'guitar',  count: 1, icon: 'R' },
  agtr:     { zh: '木吉他',     en: 'A-Guitar',    family: 'guitar',  count: 1, icon: 'A' },
  piano:    { zh: '鋼琴',       en: 'Piano',       family: 'keys',    count: 1, icon: 'Pn' },
  organ:    { zh: '管風琴',     en: 'Organ',       family: 'keys',    count: 1, icon: 'Or' },
  rhodes:   { zh: 'Rhodes',     en: 'Rhodes',      family: 'keys',    count: 1, icon: 'Rh' },

  // Electronic
  synthlead:{ zh: '主奏合成器', en: 'Synth Lead',  family: 'synth',   count: 1, icon: 'SL' },
  pad:      { zh: 'Pad',        en: 'Pad',         family: 'synth',   count: 1, icon: 'Pd' },
  arp:      { zh: '琶音器',     en: 'Arpeggio',    family: 'synth',   count: 1, icon: 'Ar' },
  beat:     { zh: '節拍機',     en: 'Drum Machine',family: 'drums',   count: 1, icon: 'BM' },
  sub:      { zh: 'Sub Bass',   en: 'Sub Bass',    family: 'bass',    count: 1, icon: 'Sb' },

  // Jazz
  upbass:   { zh: '低音提琴',   en: 'Upright Bass',family: 'bass',    count: 1, icon: 'Ub' },
  jazzgtr:  { zh: '爵士吉他',   en: 'Jazz Guitar', family: 'guitar',  count: 1, icon: 'Jg' },
  sax:      { zh: '薩克斯風',   en: 'Saxophone',   family: 'wood',    count: 1, icon: 'Sx' },
  brushes:  { zh: '刷鼓',       en: 'Brushes',     family: 'drums',   count: 1, icon: 'Br' },
};

// Style presets — each defines a default ensemble and seat layout (4 rows of grid coords).
// Rock band is the default; symphony keeps the orchestral semicircle.
function preset(slots) {
  // slots: array of { id, x, y } — pulls remaining metadata from CATALOG
  return slots.map(s => ({ ...CATALOG[s.id], id: s.id, x: s.x, y: s.y }));
}

const STYLE_PRESETS = {
  rock: preset([
    { id: 'drumkit', x: 0.50, y: 0.18 },
    { id: 'ebass',   x: 0.18, y: 0.42 },
    { id: 'egtr',    x: 0.36, y: 0.50 },
    { id: 'rhythm',  x: 0.64, y: 0.50 },
    { id: 'piano',   x: 0.82, y: 0.42 },
  ]),
  symphony: preset([
    { id: 'violin1', x: 0.10, y: 0.18 },
    { id: 'violin2', x: 0.28, y: 0.10 },
    { id: 'viola',   x: 0.50, y: 0.07 },
    { id: 'cello',   x: 0.72, y: 0.10 },
    { id: 'contra',  x: 0.90, y: 0.18 },
    { id: 'flute',   x: 0.20, y: 0.36 },
    { id: 'oboe',    x: 0.40, y: 0.32 },
    { id: 'clar',    x: 0.60, y: 0.32 },
    { id: 'bsn',     x: 0.80, y: 0.36 },
    { id: 'horn',    x: 0.22, y: 0.56 },
    { id: 'tpt',     x: 0.42, y: 0.52 },
    { id: 'tbn',     x: 0.62, y: 0.52 },
    { id: 'tba',     x: 0.80, y: 0.56 },
    { id: 'harp',    x: 0.18, y: 0.78 },
    { id: 'timp',    x: 0.42, y: 0.78 },
    { id: 'perc',    x: 0.62, y: 0.78 },
  ]),
  cinema: preset([
    { id: 'violin1', x: 0.14, y: 0.16 },
    { id: 'cello',   x: 0.42, y: 0.16 },
    { id: 'contra',  x: 0.70, y: 0.16 },
    { id: 'horn',    x: 0.22, y: 0.40 },
    { id: 'tbn',     x: 0.50, y: 0.40 },
    { id: 'tba',     x: 0.78, y: 0.40 },
    { id: 'pad',     x: 0.18, y: 0.62 },
    { id: 'piano',   x: 0.50, y: 0.62 },
    { id: 'sub',     x: 0.82, y: 0.62 },
    { id: 'timp',    x: 0.30, y: 0.82 },
    { id: 'perc',    x: 0.62, y: 0.82 },
  ]),
  jazz: preset([
    { id: 'piano',   x: 0.22, y: 0.30 },
    { id: 'sax',     x: 0.50, y: 0.22 },
    { id: 'jazzgtr', x: 0.78, y: 0.30 },
    { id: 'upbass',  x: 0.32, y: 0.62 },
    { id: 'brushes', x: 0.68, y: 0.62 },
  ]),
  lofi: preset([
    { id: 'rhodes',  x: 0.30, y: 0.30 },
    { id: 'agtr',    x: 0.70, y: 0.30 },
    { id: 'upbass',  x: 0.30, y: 0.62 },
    { id: 'brushes', x: 0.70, y: 0.62 },
    { id: 'pad',     x: 0.50, y: 0.16 },
  ]),
  edm: preset([
    { id: 'pad',     x: 0.18, y: 0.20 },
    { id: 'synthlead', x: 0.50, y: 0.18 },
    { id: 'arp',     x: 0.82, y: 0.20 },
    { id: 'sub',     x: 0.32, y: 0.52 },
    { id: 'rhodes',  x: 0.68, y: 0.52 },
    { id: 'beat',    x: 0.50, y: 0.82 },
  ]),
};

const STYLES = [
  { id: 'rock',     zh: '搖滾樂團',  en: 'Rock Band',  sub: '5 件 · Drums · Bass · Gtr · Keys' },
  { id: 'symphony', zh: '古典交響',  en: 'Symphony',   sub: '16 件 · Romantic full' },
  { id: 'cinema',   zh: '電影配樂',  en: 'Cinematic',  sub: '11 件 · Strings · Brass · Synth' },
  { id: 'jazz',     zh: '爵士小組',  en: 'Jazz Combo', sub: '5 件 · Quintet swing' },
  { id: 'lofi',     zh: '低保真',    en: 'Lo-fi',      sub: '5 件 · Warm acoustic' },
  { id: 'edm',      zh: '電子',      en: 'Electronic', sub: '6 件 · Synth · pads · beats' },
];

// Sample lyric+chord track that "plays" while recording is on
const TRACK = [
  { t: 0.0,  chord: 'Cmaj7',  pitch: 'C4',  lyric: '在這個夜裡' },
  { t: 1.6,  chord: 'Am7',    pitch: 'A3',  lyric: '我聽見遠方' },
  { t: 3.2,  chord: 'Fmaj7',  pitch: 'F4',  lyric: '一陣風吹來' },
  { t: 4.8,  chord: 'G7sus4', pitch: 'G4',  lyric: '熟悉的旋律' },
  { t: 6.4,  chord: 'Em7',    pitch: 'E4',  lyric: '是誰在唱歌' },
  { t: 8.0,  chord: 'Am9',    pitch: 'A4',  lyric: '輕輕的呼喚' },
  { t: 9.6,  chord: 'Dm7',    pitch: 'D4',  lyric: '陪我走一段' },
  { t: 11.2, chord: 'G7',     pitch: 'G4',  lyric: '回家的方向' },
];

const PROCESS_STAGES = [
  { zh: '錄音整理', en: 'Capture cleanup', detail: 'Mono downmix · noise floor · headroom' },
  { zh: '音高與音符', en: 'Pitch + notes', detail: 'YIN F0 · segmentation · confidence' },
  { zh: '節奏與力度', en: 'Rhythm + dynamics', detail: 'BPM grid · RMS · velocity' },
  { zh: '和聲與伴奏', en: 'Harmony + ensemble', detail: 'Key · chords · style arrangement' },
  { zh: '人聲空間', en: 'Vocal processing', detail: 'Dry voice · Hall A reverb blend' },
  { zh: '混音輸出', en: 'Mix + export', detail: 'Auto balance · -14 LUFS · preview bounce' },
];

const AUDIO_TAKES_KEY = 'musicelf.audioTakes.v1';
const SAVE_RECORDING_KEY = 'musicelf.saveRecording.v1';
const AUDIO_DB_NAME = 'musicelf-audio-library';
const AUDIO_STORE_NAME = 'takes';
const AUDIO_TAKE_LIMIT = 10;

const PITCH_TO_MIDI = {
  C3: 48, D3: 50, E3: 52, F3: 53, G3: 55, A3: 57, B3: 59,
  C4: 60, D4: 62, E4: 64, F4: 65, G4: 67, A4: 69, B4: 71,
  C5: 72, D5: 74, E5: 76, F5: 77, G5: 79, A5: 81, B5: 83,
};

const ROOT_TO_PC = { C: 0, D: 2, E: 4, F: 5, G: 7, A: 9, B: 11 };

// ─────────────────────────────────────────────────────────────────────────────
// HELPERS

// Map a section's grid x,y (0..1) to absolute pixel position inside the stage.
// We inset the outer 6% so cards near the edges keep clear of the stage border.
function gridToXY(gx, gy, w, h) {
  const padX = 0.07, padY = 0.06;
  return {
    x: w * (padX + gx * (1 - padX * 2)),
    y: h * (padY + gy * (1 - padY * 2)),
  };
}

function clamp(v, a, b) { return Math.max(a, Math.min(b, v)); }

function midiToFrequency(midi) {
  return 440 * Math.pow(2, (midi - 69) / 12);
}

function chordToMidis(symbol, octave = 3) {
  const match = /^([A-G])([#b]?)(.*)$/.exec(symbol || 'C');
  if (!match) return [48, 52, 55];
  let pc = ROOT_TO_PC[match[1]];
  if (match[2] === '#') pc += 1;
  if (match[2] === 'b') pc -= 1;
  pc = ((pc % 12) + 12) % 12;

  const tail = match[3] || '';
  const intervals = tail.includes('dim') ? [0, 3, 6]
    : tail.includes('m') && !tail.includes('maj') ? [0, 3, 7, 10]
    : tail.includes('7') && !tail.includes('maj') ? [0, 4, 7, 10]
    : tail.includes('maj7') ? [0, 4, 7, 11]
    : [0, 4, 7];
  const root = (octave + 1) * 12 + pc;
  return intervals.map(i => root + i);
}

function buildProcessResult({ time, style, sections, sectionState, master, swell, audioSource }) {
  const activeSections = sections
    .filter(s => !(sectionState[s.id]?.mute))
    .map(s => ({
      id: s.id,
      zh: s.zh,
      en: s.en,
      family: s.family,
      intensity: sectionState[s.id]?.intensity ?? 60,
      solo: Boolean(sectionState[s.id]?.solo),
    }));
  const styleMeta = STYLES.find(s => s.id === style) || STYLES[0];
  const sourceSeconds = audioSource?.durationSeconds || time || 12;
  const capturedSeconds = clamp(sourceSeconds, 1, 180);
  const sourceType = audioSource?.origin === 'file'
    ? 'Imported audio'
    : audioSource?.origin === 'library'
      ? 'Saved recording'
      : 'Live recording';

  return {
    id: 'take_' + Date.now().toString(36),
    durationSeconds: clamp(capturedSeconds, 1, 30),
    capturedSeconds,
    sourceName: audioSource?.name || 'Current recording',
    sourceType,
    style,
    styleName: `${styleMeta.zh} · ${styleMeta.en}`,
    activeSections,
    master,
    swell,
    tempo: 92,
    key: 'C major',
    notes: TRACK.length,
    chords: TRACK.map(t => t.chord),
    vocal: '乾聲 + Hall A',
    mix: `自動配重 · ${styleMeta.zh}`,
    lufs: '-14.0',
    format: 'WAV · 48 kHz · 24-bit',
  };
}

function clearPlaybackEngine(ref) {
  const engine = ref.current;
  if (engine.raf) cancelAnimationFrame(engine.raf);
  if (engine.ctx) engine.ctx.close().catch(() => {});
  ref.current = { ctx: null, raf: null, startedAt: 0, duration: 0 };
}

function createHallImpulse(ctx, seconds = 1.7, decay = 2.8) {
  const length = Math.floor(ctx.sampleRate * seconds);
  const impulse = ctx.createBuffer(2, length, ctx.sampleRate);
  for (let ch = 0; ch < impulse.numberOfChannels; ch += 1) {
    const data = impulse.getChannelData(ch);
    for (let i = 0; i < length; i += 1) {
      const t = i / length;
      data[i] = (Math.random() * 2 - 1) * Math.pow(1 - t, decay);
    }
  }
  return impulse;
}

function scheduleTone(ctx, destination, midi, startSeconds, durationSeconds, type, gainValue) {
  const start = ctx.currentTime + startSeconds;
  const stop = start + durationSeconds;
  const osc = ctx.createOscillator();
  const gain = ctx.createGain();
  osc.type = type;
  osc.frequency.value = midiToFrequency(midi);
  gain.gain.setValueAtTime(0.0001, start);
  gain.gain.exponentialRampToValueAtTime(Math.max(0.0002, gainValue), start + 0.025);
  gain.gain.setTargetAtTime(0.0001, Math.max(start + 0.04, stop - 0.12), 0.045);
  osc.connect(gain);
  gain.connect(destination);
  osc.start(start);
  osc.stop(stop + 0.12);
}

function startProcessedPreview(result, playbackRef, setPlayback) {
  const AudioContextClass = window.AudioContext || window.webkitAudioContext;
  if (!AudioContextClass) {
    setPlayback({ state: 'error', progress: 0 });
    return;
  }

  clearPlaybackEngine(playbackRef);
  const ctx = new AudioContextClass();
  const master = ctx.createGain();
  const voiceBus = ctx.createGain();
  const ensembleBus = ctx.createGain();
  const dryGain = ctx.createGain();
  const wetGain = ctx.createGain();
  const convolver = ctx.createConvolver();

  master.gain.value = clamp(0.18 + (result.master + 30) / 180, 0.10, 0.32);
  voiceBus.gain.value = 0.82;
  ensembleBus.gain.value = clamp(0.16 + result.swell / 460, 0.16, 0.38);
  dryGain.gain.value = 0.66;
  wetGain.gain.value = 0.28;
  convolver.buffer = createHallImpulse(ctx);

  voiceBus.connect(dryGain);
  voiceBus.connect(wetGain);
  dryGain.connect(master);
  wetGain.connect(convolver);
  convolver.connect(master);
  ensembleBus.connect(master);
  master.connect(ctx.destination);

  const offset = 0.08;
  TRACK.forEach((line, index) => {
    const nextT = TRACK[index + 1]?.t ?? result.durationSeconds;
    const dur = Math.max(0.28, nextT - line.t - 0.06);
    const midi = PITCH_TO_MIDI[line.pitch] || 60;
    scheduleTone(ctx, voiceBus, midi, offset + line.t, dur, 'sine', 0.18);

    const chordDur = Math.max(0.7, nextT - line.t);
    chordToMidis(line.chord, 3).forEach((note, noteIndex) => {
      scheduleTone(ctx, ensembleBus, note, offset + line.t + noteIndex * 0.018, chordDur, 'triangle', 0.035);
    });
    scheduleTone(ctx, ensembleBus, chordToMidis(line.chord, 2)[0] - 12, offset + line.t, chordDur * 0.96, 'sine', 0.055);
  });

  const hasPulse = result.activeSections.some(s => ['drums', 'perc'].includes(s.family));
  if (hasPulse) {
    for (let beat = 0; beat < result.durationSeconds; beat += 60 / result.tempo) {
      const downbeat = Math.round(beat / (60 / result.tempo)) % 4 === 0;
      scheduleTone(ctx, ensembleBus, downbeat ? 58 : 72, offset + beat, 0.075, 'square', downbeat ? 0.04 : 0.022);
    }
  }

  const startedAt = performance.now();
  const duration = result.durationSeconds + 0.25;
  playbackRef.current = { ctx, raf: null, startedAt, duration };
  setPlayback({ state: 'playing', progress: 0 });

  const tick = () => {
    const elapsed = (performance.now() - startedAt) / 1000;
    const progress = clamp((elapsed / duration) * 100, 0, 100);
    if (progress >= 100) {
      clearPlaybackEngine(playbackRef);
      setPlayback({ state: 'idle', progress: 100 });
      return;
    }
    setPlayback({ state: 'playing', progress });
    playbackRef.current.raf = requestAnimationFrame(tick);
  };
  playbackRef.current.raf = requestAnimationFrame(tick);
}

// Reactive breakpoint hook — true while viewport is at or below `bp` px.
// Mobile layout drops the conductor stage entirely since pinching a 16-seat
// orchestra on a 5-inch screen is hostile; we prefer vertical-stacked panels.
function useIsMobile(bp = 720) {
  const [mobile, setMobile] = useState(() =>
    typeof window !== 'undefined' && window.matchMedia(`(max-width: ${bp}px)`).matches);
  useEffect(() => {
    const mq = window.matchMedia(`(max-width: ${bp}px)`);
    const on = (e) => setMobile(e.matches);
    mq.addEventListener('change', on);
    return () => mq.removeEventListener('change', on);
  }, [bp]);
  return mobile;
}

// ─────────────────────────────────────────────────────────────────────────────
// HEADER

function Header({ recording, time, onToggleRec, mode, setMode, onExport, tabs, compact }) {
  const tabList = tabs || [
    ['conductor', '指揮台', 'Conduct'],
    ['mixer',     '混音台', 'Mix'],
    ['score',     '譜面',   'Score'],
  ];
  return (
    <header className={'hdr' + (compact ? ' hdr-compact' : '')}>
      <div className="hdr-l">
        <div className="brand">
          <div className="brand-mark">
            <svg viewBox="0 0 24 24" width="22" height="22" aria-hidden="true">
              <path d="M5 4 L5 17.2 M5 17.2 a3 3 0 1 0 3 3 L8 7 L19 5 L19 14.2 M19 14.2 a3 3 0 1 0 3 3"
                fill="none" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round" />
            </svg>
          </div>
          <div className="brand-text">
            <div className="brand-name">音靈 / Music Elf</div>
            <div className="brand-sub">{compact ? 'Mobile · v0.4' : 'Conductor Studio · v0.4'}</div>
          </div>
        </div>
        {!compact && <div className="hdr-divider" />}
        <nav className="hdr-tabs" aria-label="View mode">
          {tabList.map(([id, zh, en]) => (
            <button key={id} className={'tab' + (mode === id ? ' on' : '')} onClick={() => setMode(id)}>
              <span className="zh">{zh}</span>
              <span className="en">{en}</span>
            </button>
          ))}
        </nav>
      </div>
      <div className="hdr-r">
        <div className="meter" aria-hidden="true">
          <div className="meter-row"><span /><span /><span /><span /><span /><span /><span /></div>
          <div className="meter-row"><span /><span /><span /><span /><span /></div>
        </div>
        <div className="time">
          <span className="dot" data-on={recording ? '1' : '0'} />
          <span className="time-num">{formatTime(time)}</span>
          <span className="time-sub">{recording ? 'REC' : 'IDLE'}</span>
        </div>
        <button className={'btn ' + (recording ? 'btn-stop' : 'btn-rec')} onClick={onToggleRec}>
          {recording
            ? <><span className="rec-square" /> 停止 / Stop</>
            : <><span className="rec-circle" /> 開始錄音 / Record</>}
        </button>
        <button className="btn btn-ghost" onClick={onExport}>匯出 · Export</button>
      </div>
    </header>
  );
}

function formatTime(s) {
  const m = Math.floor(s / 60);
  const ss = Math.floor(s % 60);
  const ms = Math.floor((s - Math.floor(s)) * 100);
  return `${String(m).padStart(2,'0')}:${String(ss).padStart(2,'0')}.${String(ms).padStart(2,'0')}`;
}

function formatAudioSize(bytes) {
  if (!bytes) return '0 KB';
  if (bytes < 1024 * 1024) return `${Math.max(1, Math.round(bytes / 1024))} KB`;
  return `${(bytes / (1024 * 1024)).toFixed(1)} MB`;
}

function formatTakeStamp(ts) {
  const d = new Date(ts);
  const pad = (n) => String(n).padStart(2, '0');
  return `${pad(d.getMonth() + 1)}/${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

function openAudioLibrary() {
  return new Promise((resolve, reject) => {
    if (!window.indexedDB) {
      reject(new Error('IndexedDB is unavailable'));
      return;
    }
    const request = indexedDB.open(AUDIO_DB_NAME, 1);
    request.onupgradeneeded = () => {
      const db = request.result;
      if (!db.objectStoreNames.contains(AUDIO_STORE_NAME)) {
        db.createObjectStore(AUDIO_STORE_NAME, { keyPath: 'id' });
      }
    };
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error || new Error('Could not open audio library'));
  });
}

async function putAudioBlob(id, blob) {
  const db = await openAudioLibrary();
  return new Promise((resolve, reject) => {
    const tx = db.transaction(AUDIO_STORE_NAME, 'readwrite');
    tx.objectStore(AUDIO_STORE_NAME).put({ id, blob });
    tx.oncomplete = () => { db.close(); resolve(); };
    tx.onerror = () => { db.close(); reject(tx.error || new Error('Could not save audio')); };
  });
}

async function getAudioBlob(id) {
  const db = await openAudioLibrary();
  return new Promise((resolve, reject) => {
    const tx = db.transaction(AUDIO_STORE_NAME, 'readonly');
    const request = tx.objectStore(AUDIO_STORE_NAME).get(id);
    request.onsuccess = () => resolve(request.result?.blob || null);
    request.onerror = () => reject(request.error || new Error('Could not read audio'));
    tx.oncomplete = () => db.close();
  });
}

async function deleteAudioBlob(id) {
  const db = await openAudioLibrary();
  return new Promise((resolve, reject) => {
    const tx = db.transaction(AUDIO_STORE_NAME, 'readwrite');
    tx.objectStore(AUDIO_STORE_NAME).delete(id);
    tx.oncomplete = () => { db.close(); resolve(); };
    tx.onerror = () => { db.close(); reject(tx.error || new Error('Could not delete audio')); };
  });
}

function preferredRecordingMime() {
  const candidates = [
    'audio/webm;codecs=opus',
    'audio/webm',
    'audio/mp4',
  ];
  if (!window.MediaRecorder?.isTypeSupported) return '';
  return candidates.find(type => MediaRecorder.isTypeSupported(type)) || '';
}

function writeWavString(view, offset, text) {
  for (let i = 0; i < text.length; i += 1) {
    view.setUint8(offset + i, text.charCodeAt(i));
  }
}

function createDemoTakeBlob(durationSeconds = 12) {
  const sampleRate = 24000;
  const duration = clamp(durationSeconds || 12, 1, 12);
  const frames = Math.floor(sampleRate * duration);
  const buffer = new ArrayBuffer(44 + frames * 2);
  const view = new DataView(buffer);
  writeWavString(view, 0, 'RIFF');
  view.setUint32(4, 36 + frames * 2, true);
  writeWavString(view, 8, 'WAVE');
  writeWavString(view, 12, 'fmt ');
  view.setUint32(16, 16, true);
  view.setUint16(20, 1, true);
  view.setUint16(22, 1, true);
  view.setUint32(24, sampleRate, true);
  view.setUint32(28, sampleRate * 2, true);
  view.setUint16(32, 2, true);
  view.setUint16(34, 16, true);
  writeWavString(view, 36, 'data');
  view.setUint32(40, frames * 2, true);

  let phase = 0;
  for (let i = 0; i < frames; i += 1) {
    const t = i / sampleRate;
    let line = TRACK[0];
    for (const item of TRACK) if (t >= item.t) line = item;
    const midi = PITCH_TO_MIDI[line.pitch] || 60;
    const vibrato = Math.sin(t * Math.PI * 11) * 0.006;
    const freq = midiToFrequency(midi) * (1 + vibrato);
    phase += (Math.PI * 2 * freq) / sampleRate;
    const envelope = Math.min(1, t * 6, (duration - t) * 6);
    const sample = Math.sin(phase) * 0.18 * Math.max(0, envelope);
    view.setInt16(44 + i * 2, clamp(sample, -1, 1) * 0x7fff, true);
  }

  return new Blob([buffer], { type: 'audio/wav' });
}

function readAudioDuration(file) {
  return new Promise((resolve) => {
    const audio = document.createElement('audio');
    const url = URL.createObjectURL(file);
    let settled = false;
    const done = (value) => {
      if (settled) return;
      settled = true;
      URL.revokeObjectURL(url);
      resolve(Number.isFinite(value) && value > 0 ? value : 12);
    };
    const timeout = setTimeout(() => done(12), 2200);
    audio.preload = 'metadata';
    audio.onloadedmetadata = () => {
      clearTimeout(timeout);
      done(audio.duration);
    };
    audio.onerror = () => {
      clearTimeout(timeout);
      done(12);
    };
    audio.src = url;
  });
}

function buildSourcePitchHistory(source) {
  const len = clamp(Math.round((source?.durationSeconds || 12) * 10), 36, 120);
  return Array.from({ length: len }, (_, i) => {
    const t = (i / Math.max(1, len - 1)) * Math.PI * 2;
    return clamp(50 + Math.sin(t * 0.8) * 24 + Math.sin(t * 4.5) * 5, 0, 100);
  });
}

// ─────────────────────────────────────────────────────────────────────────────
// LEFT RAIL — Voice / pitch / lyric

function VoicePanel({ recording, time, current, pitchHistory, processState, processResult, playback, onPlayResult, onProcessAgain,
                      saveRecording, onSaveRecordingChange, audioTakes, activeAudioSource, captureState,
                      onImportAudio, onProcessSavedAudio, onDeleteSavedAudio, onProcessActiveAudio }) {
  // pitch line drawing
  const pathD = useMemo(() => {
    if (!pitchHistory.length) return '';
    return pitchHistory.map((p, i) => {
      const x = (i / Math.max(1, pitchHistory.length - 1)) * 100;
      const y = 100 - clamp(p, 0, 100);
      return `${i === 0 ? 'M' : 'L'} ${x.toFixed(2)} ${y.toFixed(2)}`;
    }).join(' ');
  }, [pitchHistory]);

  return (
    <aside className="panel voice">
      <div className="panel-hd">
        <div className="panel-eyebrow">VOICE · 你的聲音</div>
        <div className="panel-title">
          <span className={'rec-led' + (recording ? ' on' : '')} />
          {recording ? '正在聆聽' : '等待中'}
          <span className="panel-title-en">{recording ? 'Listening' : 'Standby'}</span>
        </div>
      </div>

      <div className="pitch-card">
        <div className="pitch-grid">
          {['C5','A4','F4','C4','G3'].map((n, i) => (
            <div key={n} className="pitch-row">
              <span className="pitch-label">{n}</span>
              <span className="pitch-line" />
            </div>
          ))}
        </div>
        <svg className="pitch-svg" viewBox="0 0 100 100" preserveAspectRatio="none">
          <path d={pathD} fill="none" stroke="var(--accent)" strokeWidth="0.8"
                strokeLinecap="round" strokeLinejoin="round" vectorEffect="non-scaling-stroke" />
          {pitchHistory.length > 0 && (
            <circle cx={100} cy={100 - clamp(pitchHistory[pitchHistory.length-1], 0, 100)}
                    r="1.4" fill="var(--accent)" vectorEffect="non-scaling-stroke" />
          )}
        </svg>
        <div className="pitch-now">
          <span className="pitch-now-note">{current.pitch}</span>
          <span className="pitch-now-cents">+3¢</span>
        </div>
      </div>

      <div className="readings">
        <Reading label="Tempo"      val="92"   unit="BPM" />
        <Reading label="Key"        val="C"    unit="major" />
        <Reading label="Loudness"   val="-14"  unit="LUFS" />
        <Reading label="Confidence" val="0.93" unit="" />
      </div>

      <AudioSourcePanel recording={recording}
                        saveRecording={saveRecording}
                        onSaveRecordingChange={onSaveRecordingChange}
                        audioTakes={audioTakes}
                        activeAudioSource={activeAudioSource}
                        captureState={captureState}
                        onImportAudio={onImportAudio}
                        onProcessSavedAudio={onProcessSavedAudio}
                        onDeleteSavedAudio={onDeleteSavedAudio}
                        onProcessActiveAudio={onProcessActiveAudio} />

      <ProcessingPanel processState={processState}
                       processResult={processResult}
                       playback={playback}
                       onPlayResult={onPlayResult}
                       onProcessAgain={onProcessAgain}
                       activeAudioSource={activeAudioSource} />

      <div className="lyric-card">
        <div className="panel-eyebrow">LYRIC · 歌詞辨識</div>
        <div className="lyric-stream">
          {TRACK.map((line, i) => {
            const active = recording && time >= line.t && time < (TRACK[i+1]?.t ?? Infinity);
            const past   = recording && time >= (TRACK[i+1]?.t ?? Infinity);
            return (
              <div key={i} className={'lyric-line' + (active ? ' active' : '') + (past ? ' past' : '')}>
                <span className="lyric-chord">{line.chord}</span>
                <span className="lyric-text">{line.lyric}</span>
              </div>
            );
          })}
        </div>
      </div>
    </aside>
  );
}

function Reading({ label, val, unit }) {
  return (
    <div className="reading">
      <div className="reading-label">{label}</div>
      <div className="reading-val">{val}<span>{unit}</span></div>
    </div>
  );
}

function AudioSourcePanel({ recording, saveRecording, onSaveRecordingChange, audioTakes, activeAudioSource,
                            captureState, onImportAudio, onProcessSavedAudio, onDeleteSavedAudio,
                            onProcessActiveAudio, compact = false }) {
  const inputId = compact ? 'audio-import-mobile' : 'audio-import-desktop';
  const statusText = captureState?.message || '錄音與音檔都會在本機瀏覽器處理';

  return (
    <section className={'audio-source-panel' + (compact ? ' compact' : '')}>
      <div className="audio-source-head">
        <div>
          <div className="panel-eyebrow">AUDIO · 音檔來源</div>
          <div className="audio-source-title">錄音檔庫</div>
        </div>
        <label className={'save-toggle' + (saveRecording ? ' on' : '')}>
          <input type="checkbox"
                 checked={saveRecording}
                 onChange={(e) => onSaveRecordingChange(e.target.checked)} />
          <span className="save-toggle-box" />
          <span>儲存錄音</span>
        </label>
      </div>

      <div className="audio-source-status" data-state={captureState?.status || 'idle'}>
        {statusText}
      </div>

      {activeAudioSource && (
        <div className="active-audio">
          <div className="active-audio-main">
            <span className="active-audio-label">
              {activeAudioSource.origin === 'file' ? '已選音檔' : activeAudioSource.origin === 'library' ? '已存錄音' : '目前錄音'}
            </span>
            <span className="active-audio-name">{activeAudioSource.name}</span>
          </div>
          <span className="active-audio-meta">
            {formatTime(activeAudioSource.durationSeconds || 0)} · {formatAudioSize(activeAudioSource.size)}
          </span>
        </div>
      )}

      <div className="audio-source-actions">
        <label className="btn btn-ghost audio-import-btn" htmlFor={inputId}>
          選擇音檔
          <input id={inputId}
                 className="audio-file-input"
                 type="file"
                 accept="audio/*,.wav,.mp3,.m4a,.webm,.ogg"
                 onChange={onImportAudio}
                 disabled={recording} />
        </label>
        <button className="btn btn-ghost"
                onClick={onProcessActiveAudio}
                disabled={!activeAudioSource || recording}>
          編制選取
        </button>
      </div>

      <div className="audio-take-list">
        {audioTakes.length === 0 ? (
          <div className="audio-empty">尚未儲存錄音。停止錄音時開啟「儲存錄音」即可加入這裡。</div>
        ) : audioTakes.map(take => (
          <div key={take.id} className={'audio-take-row' + (activeAudioSource?.id === take.id ? ' on' : '')}>
            <button className="audio-take-main"
                    onClick={() => onProcessSavedAudio(take)}
                    disabled={recording}>
              <span className="audio-take-name">{take.name}</span>
              <span className="audio-take-meta">
                {formatTakeStamp(take.savedAt)} · {formatTime(take.durationSeconds || 0)} · {formatAudioSize(take.size)}
              </span>
            </button>
            <button className="audio-take-delete"
                    aria-label={`Delete ${take.name}`}
                    onClick={() => onDeleteSavedAudio(take.id)}
                    disabled={recording}>✕</button>
          </div>
        ))}
      </div>
    </section>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// RIGHT RAIL — Style / families / score

const FAV_MAX = 10;

function ControlsPanel({ style, setStyle, master, setMaster, swell, setSwell,
                        favorites, onSaveFavorite, onLoadFavorite, onDeleteFavorite }) {
  return (
    <aside className="panel controls">
      <div className="panel-hd">
        <div className="panel-eyebrow">STYLE · 風格</div>
        <div className="panel-title">風格 <span className="panel-title-en">Style</span></div>
      </div>

      <div className="style-grid">
        {STYLES.map(s => (
          <button key={s.id} className={'style-card' + (style === s.id ? ' on' : '')}
                  onClick={() => setStyle(s.id)}>
            <span className="style-zh">{s.zh}</span>
            <span className="style-en">{s.en}</span>
            <span className="style-sub">{s.sub}</span>
          </button>
        ))}
      </div>

      <FavoritesSection favorites={favorites}
                        onSave={onSaveFavorite}
                        onLoad={onLoadFavorite}
                        onDelete={onDeleteFavorite} />

      <div className="panel-eyebrow" style={{marginTop: 22}}>CONDUCTOR · 指揮</div>
      <div className="globals">
        <Slider label="總音量 / Master" value={master} onChange={setMaster} unit="dB" min={-30} max={6} />
        <Slider label="氣勢 / Swell"     value={swell}  onChange={setSwell}  unit="" min={0} max={100} />
      </div>
    </aside>
  );
}

function FavoritesSection({ favorites, onSave, onLoad, onDelete }) {
  const full = favorites.length >= FAV_MAX;
  return (
    <div className="favs">
      <div className="favs-hd">
        <div className="panel-eyebrow">FAVORITES · 我的最愛</div>
        <span className="favs-count">{favorites.length}/{FAV_MAX}</span>
      </div>
      <div className="favs-list">
        {favorites.map(f => {
          const styleMeta = STYLES.find(s => s.id === f.style);
          return (
            <div key={f.id} className="fav-item" onClick={() => onLoad(f.id)}
                 role="button" tabIndex={0}
                 title={`載入 / Load · ${f.name}`}>
              <span className="fav-name">{f.name}</span>
              <span className="fav-meta">
                {styleMeta ? styleMeta.zh : f.style} · {f.sections.length} 件
              </span>
              <span className="fav-x" role="button" aria-label="Delete"
                    onClick={(e) => { e.stopPropagation(); onDelete(f.id); }}>✕</span>
            </div>
          );
        })}
        <button className={'fav-add' + (full ? ' disabled' : '')}
                disabled={full}
                onClick={onSave}
                title={full ? '已達上限 10 / Limit reached' : '儲存目前編制 / Save current'}>
          <span className="fav-add-glyph">＋</span>
          <span className="fav-add-text">
            {full ? '已達上限' : '儲存目前編制'}
            <i>{full ? 'Limit reached (10)' : 'Save current'}</i>
          </span>
        </button>
      </div>
    </div>
  );
}

// Picker modal — full catalog grouped by family. Reused by the Mixer's
// leftmost "Add instrument" tile. The active-stage Quick-Add menu has its
// own inline picker so it can anchor to the tap point.
function InstrumentPicker({ open, onClose, activeIds, onAdd }) {
  if (!open) return null;
  return (
    <div className="picker-bg" onClick={onClose}>
      <div className="picker" onClick={e => e.stopPropagation()}>
        <div className="picker-hd">
          <div>
            <div className="panel-eyebrow">CATALOG · 樂器庫</div>
            <div className="picker-title">新增樂器 <span className="picker-title-en">Add instrument</span></div>
          </div>
          <button className="modal-x" onClick={onClose}>✕</button>
        </div>
        <div className="picker-body">
          {Object.entries(FAMILIES).map(([k, f]) => {
            const items = Object.entries(CATALOG).filter(([, c]) => c.family === k);
            if (!items.length) return null;
            return (
              <div key={k} className="picker-fam">
                <div className="picker-fam-hd">
                  <span className="family-dot" style={{ background: f.accent }} />
                  <span className="picker-fam-name">{f.zh} · {f.en}</span>
                </div>
                <div className="picker-grid">
                  {items.map(([id, c]) => (
                    <button key={id}
                            className={'picker-card' + (activeIds.has(id) ? ' active' : '')}
                            disabled={activeIds.has(id)}
                            onClick={() => { onAdd(id); onClose(); }}>
                      <span className="picker-icon" style={{ borderColor: f.accent, color: f.accent }}>{c.icon}</span>
                      <span className="picker-zh">{c.zh}</span>
                      <span className="picker-en">{c.en}</span>
                      {activeIds.has(id) && <span className="picker-badge">已加入</span>}
                    </button>
                  ))}
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}

function Mini({ value, onChange, accent }) {
  const ref = useRef(null);
  const drag = (e) => {
    const r = ref.current.getBoundingClientRect();
    const move = (ev) => {
      const v = clamp(((ev.clientX - r.left) / r.width) * 100, 0, 100);
      onChange(Math.round(v));
    };
    move(e);
    const up = () => { window.removeEventListener('pointermove', move); window.removeEventListener('pointerup', up); };
    window.addEventListener('pointermove', move); window.addEventListener('pointerup', up);
  };
  return (
    <div className="mini" ref={ref} onPointerDown={drag}>
      <div className="mini-fill" style={{ width: value + '%', background: accent }} />
      <span className="mini-val">{value}</span>
    </div>
  );
}

function Slider({ label, value, onChange, unit, min = 0, max = 100 }) {
  return (
    <div className="slider">
      <div className="slider-lbl"><span>{label}</span><span className="slider-val">{value}{unit}</span></div>
      <input type="range" min={min} max={max} value={value} onChange={e => onChange(Number(e.target.value))} />
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// CENTER — Stage with semicircular orchestra

function Stage({ sections, sectionState, setSectionState, hover, setHover, baton, setBaton, recording, onRemove, onAdd }) {
  const stageRef = useRef(null);
  const [size, setSize] = useState({ w: 800, h: 560 });
  // Quick-add menu: { x, y, gridX, gridY } in stage-local pixel coords, or null.
  const [quickAdd, setQuickAdd] = useState(null);
  // Transient ripple at the spot the user long-pressed / double-clicked,
  // so the menu doesn't appear "from nowhere."
  const [ripple, setRipple] = useState(null);

  useEffect(() => {
    const el = stageRef.current;
    if (!el) return;
    const ro = new ResizeObserver(() => {
      setSize({ w: el.clientWidth, h: el.clientHeight });
    });
    ro.observe(el);
    return () => ro.disconnect();
  }, []);

  const { w, h } = size;

  // Convert a client point to stage-local px and to grid (0..1) — matches the
  // padX/padY inset used by gridToXY so a new seat lands centered on the tap.
  const stageCoords = (clientX, clientY) => {
    const r = stageRef.current.getBoundingClientRect();
    const x = clamp(clientX - r.left, 0, r.width);
    const y = clamp(clientY - r.top,  0, r.height);
    const padX = 0.07, padY = 0.06;
    const gx = clamp(((x / r.width)  - padX) / (1 - padX * 2), 0.02, 0.98);
    const gy = clamp(((y / r.height) - padY) / (1 - padY * 2), 0.02, 0.98);
    return { x, y, gx, gy };
  };

  const openQuickAdd = (clientX, clientY) => {
    const c = stageCoords(clientX, clientY);
    setQuickAdd({ x: c.x, y: c.y, gridX: c.gx, gridY: c.gy });
    setRipple({ x: c.x, y: c.y, t: Date.now() });
    setTimeout(() => setRipple(null), 700);
  };

  // Empty-stage pointerdown:
  //   • starts a long-press timer (380ms) → opens quick-add at that spot
  //   • if pointer moves >6px before timer fires, cancels long-press and
  //     enters baton-drag (move baton with pointer until release)
  //   • a simple tap/release → moves baton to the tap point
  // Skips when the pointerdown originated on a seat, the baton, the podium,
  // or the quick-add menu — those have their own gesture handling.
  const onStageDown = (e) => {
    if (e.target.closest('.seat, .baton, .podium, .quick-add')) return;
    if (e.button !== undefined && e.button !== 0) return;
    const startX = e.clientX, startY = e.clientY;
    let armed = false;     // true once long-press timer has fired
    let batoning = false;  // true once user crossed the move threshold
    const longPress = setTimeout(() => {
      armed = true;
      openQuickAdd(startX, startY);
    }, 380);
    const move = (ev) => {
      const dx = ev.clientX - startX, dy = ev.clientY - startY;
      if (!batoning && !armed && Math.hypot(dx, dy) > 6) {
        clearTimeout(longPress);
        batoning = true;
      }
      if (batoning) {
        const c = stageCoords(ev.clientX, ev.clientY);
        setBaton({ x: c.x, y: c.y });
      }
    };
    const up = (ev) => {
      clearTimeout(longPress);
      if (!armed && !batoning) {
        // Simple tap → move baton to that spot, dismiss any open menu.
        const c = stageCoords(ev.clientX, ev.clientY);
        setBaton({ x: c.x, y: c.y });
        setQuickAdd(null);
      }
      window.removeEventListener('pointermove', move);
      window.removeEventListener('pointerup', up);
    };
    window.addEventListener('pointermove', move);
    window.addEventListener('pointerup', up);
  };

  // Double-click on empty stage: open quick-add at that spot. Mirrors long-press
  // for users who don't want to wait. (Browsers fire dblclick after a normal
  // click pair; our tap handler will have moved the baton on the first click —
  // which is fine, the menu is what matters.)
  const onStageDblClick = (e) => {
    if (e.target.closest('.seat, .baton, .podium, .quick-add')) return;
    openQuickAdd(e.clientX, e.clientY);
  };

  // baton drag — left intact so the baton handle itself is also grabbable
  const onBatonDown = (e) => {
    e.stopPropagation();
    const r = stageRef.current.getBoundingClientRect();
    const move = (ev) => {
      setBaton({
        x: clamp(ev.clientX - r.left, 0, r.width),
        y: clamp(ev.clientY - r.top, 0, r.height),
      });
    };
    move(e);
    const up = () => { window.removeEventListener('pointermove', move); window.removeEventListener('pointerup', up); };
    window.addEventListener('pointermove', move); window.addEventListener('pointerup', up);
  };

  return (
    <div className="stage" ref={stageRef}
         onPointerDown={onStageDown}
         onDoubleClick={onStageDblClick}>

      {/* gesture hint — only when stage is "quiet" */}
      <div className="stage-hint">
        <span className="stage-hint-row">
          <span className="hint-glyph">·</span>
          <span><b>點擊／拖曳</b>空白處・移動指揮棒
                <i>Tap or drag to move the baton</i></span>
        </span>
        <span className="stage-hint-row">
          <span className="hint-glyph">＋</span>
          <span><b>長按</b>或<b>雙擊</b>空白處・新增樂器
                <i>Long-press or double-click to add an instrument</i></span>
        </span>
      </div>

      {/* arcs */}
      <svg className="stage-arcs" viewBox={`0 0 ${w} ${h}`} preserveAspectRatio="none">
        <defs>
          <radialGradient id="floorGrad" cx="50%" cy="100%" r="95%">
            <stop offset="0%"   stopColor="rgba(232,169,106,0.10)" />
            <stop offset="60%"  stopColor="rgba(232,169,106,0.04)" />
            <stop offset="100%" stopColor="rgba(232,169,106,0)" />
          </radialGradient>
        </defs>
        <rect width={w} height={h} fill="url(#floorGrad)" />
        {/* horizontal row guides matching the y-bands of the four sections */}
        {[0.18, 0.36, 0.56, 0.78].map((gy, i) => {
          const y = h * (0.06 + gy * 0.88);
          return (
            <line key={i} x1={w*0.04} x2={w*0.96} y1={y} y2={y}
                  stroke="rgba(232,169,106,0.10)" strokeDasharray="2 6" />
          );
        })}
      </svg>

      {/* family halos */}
      <div className="stage-haloes">
        {Object.entries(FAMILIES).map(([k, f]) => {
          const pts = sections.filter(s => s.family === k);
          if (!pts.length) return null;
          const intensity = pts.reduce((n, s) => n + (sectionState[s.id]?.intensity || 50), 0) / pts.length / 100;
          const avg = pts.reduce((acc, s) => {
            const pos = sectionState[s.id]?.pos;
            const p = pos ? gridToXY(pos.x, pos.y, w, h) : gridToXY(s.x, s.y, w, h);
            return { x: acc.x + p.x, y: acc.y + p.y };
          }, { x: 0, y: 0 });
          avg.x /= pts.length; avg.y /= pts.length;
          const radius = 90 + intensity * 100;
          return (
            <div key={k} className="halo"
                 style={{
                   left: avg.x - radius, top: avg.y - radius,
                   width: radius * 2, height: radius * 2,
                   background: `radial-gradient(circle, ${f.accent}${recording ? '30' : '20'} 0%, transparent 70%)`,
                   opacity: 0.4 + intensity * 0.5,
                 }} />
          );
        })}
      </div>

      {/* section seats */}
      {sections.map(s => {
        const st = sectionState[s.id] || { intensity: 60, mute: false, solo: false };
        const pos = st.pos || { x: s.x, y: s.y };
        const p = gridToXY(pos.x, pos.y, w, h);
        const fam = FAMILIES[s.family];
        const isHover = hover === s.id;
        return (
          <SectionSeat key={s.id}
                       s={s} fam={fam} pos={p} state={st}
                       hover={isHover}
                       stageRef={stageRef}
                       onHover={(on) => setHover(on ? s.id : null)}
                       onRemove={() => onRemove(s.id)}
                       onMove={(gx, gy) => setSectionState({ ...sectionState, [s.id]: { ...st, pos: { x: gx, y: gy } } })}
                       onChange={(patch) => setSectionState({ ...sectionState, [s.id]: { ...st, ...patch } })}
                       recording={recording} />
        );
      })}

      {/* podium */}
      <div className="podium" style={{ left: w / 2, top: h * 0.97 }}>
        <div className="podium-disk" />
        <div className="podium-label"><span>YOU · 你</span></div>
      </div>

      {/* baton */}
      <div className="baton" style={{ left: baton.x, top: baton.y }} onPointerDown={onBatonDown}>
        <div className="baton-tip" />
        <div className="baton-trail" />
        <div className="baton-label">BATON · 指揮棒</div>
      </div>

      {/* tap ripple — short pulse where the user long-pressed / dbl-clicked */}
      {ripple && (
        <div className="tap-ripple" style={{ left: ripple.x, top: ripple.y }} />
      )}

      {/* quick-add popover */}
      {quickAdd && (
        <QuickAddMenu
          x={quickAdd.x} y={quickAdd.y}
          stageW={w} stageH={h}
          activeIds={new Set(sections.map(s => s.id))}
          onPick={(id) => {
            onAdd(id, { x: quickAdd.gridX, y: quickAdd.gridY });
            setQuickAdd(null);
          }}
          onClose={() => setQuickAdd(null)}
        />
      )}
    </div>
  );
}

// Floating instrument picker that appears at the long-press / double-click point
// on the empty stage. Two-step: pick a family chip, then pick an instrument from
// that family. Closes on outside-click via the backdrop, on Escape, or on pick.
function QuickAddMenu({ x, y, stageW, stageH, activeIds, onPick, onClose }) {
  const [fam, setFam] = useState(null);
  // Clamp the panel inside the stage. We anchor by the click point and offset
  // by an estimated panel size so the menu hugs the cursor without overflowing.
  const PW = 260, PH = fam ? 230 : 130;
  const left = clamp(x - PW / 2, 12, Math.max(12, stageW - PW - 12));
  const top  = clamp(y + 14,     12, Math.max(12, stageH - PH - 12));
  // If the menu would have been pushed up off-screen, flip above the cursor.
  const flipUp = (y + PH + 28) > stageH;
  const finalTop = flipUp ? clamp(y - PH - 14, 12, stageH - PH - 12) : top;

  useEffect(() => {
    const onKey = (e) => { if (e.key === 'Escape') onClose(); };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, [onClose]);

  const families = Object.entries(FAMILIES).filter(([k]) =>
    Object.values(CATALOG).some(c => c.family === k));
  const items = fam ? Object.entries(CATALOG).filter(([, c]) => c.family === fam) : [];

  return (
    <>
      {/* invisible backdrop so any tap outside dismisses */}
      <div className="quick-add-bg" onPointerDown={(e) => { e.stopPropagation(); onClose(); }} />
      <div className={'quick-add' + (flipUp ? ' flip-up' : '')}
           style={{ left, top: finalTop, width: PW }}
           onPointerDown={(e) => e.stopPropagation()}>
        {/* anchor tail */}
        <span className={'quick-add-tail' + (flipUp ? ' down' : '')}
              style={{ left: clamp(x - left, 14, PW - 14) }} />
        <div className="quick-add-hd">
          {fam ? (
            <>
              <button className="quick-add-back" onClick={() => setFam(null)}>‹</button>
              <span className="quick-add-title">
                {FAMILIES[fam].zh}<i>{FAMILIES[fam].en}</i>
              </span>
            </>
          ) : (
            <>
              <span className="quick-add-title">新增樂器<i>Add instrument here</i></span>
            </>
          )}
          <button className="quick-add-x" onClick={onClose} aria-label="Close">✕</button>
        </div>

        {!fam ? (
          <div className="quick-add-fams">
            {families.map(([k, f]) => (
              <button key={k} className="quick-add-fam" onClick={() => setFam(k)}
                      style={{ '--fam-accent': f.accent }}>
                <span className="quick-add-fam-dot" />
                <span className="quick-add-fam-zh">{f.zh}</span>
                <span className="quick-add-fam-en">{f.en}</span>
                <span className="quick-add-fam-n">
                  {Object.values(CATALOG).filter(c => c.family === k).length}
                </span>
              </button>
            ))}
          </div>
        ) : (
          <div className="quick-add-list">
            {items.map(([id, c]) => {
              const taken = activeIds.has(id);
              return (
                <button key={id}
                        className={'quick-add-item' + (taken ? ' taken' : '')}
                        disabled={taken}
                        onClick={() => onPick(id)}>
                  <span className="quick-add-icon"
                        style={{ borderColor: FAMILIES[c.family].accent, color: FAMILIES[c.family].accent }}>
                    {c.icon}
                  </span>
                  <span className="quick-add-item-text">
                    <span className="quick-add-item-zh">{c.zh}</span>
                    <span className="quick-add-item-en">{c.en}</span>
                  </span>
                  {taken && <span className="quick-add-taken">已加入</span>}
                </button>
              );
            })}
          </div>
        )}
      </div>
    </>
  );
}

function SectionSeat({ s, fam, pos, state, hover, onHover, onChange, onMove, onRemove, stageRef, recording }) {
  const seatRef = useRef(null);
  const [dragging, setDragging] = useState(false);

  const onIntDown = (e) => {
    e.stopPropagation();
    const r = seatRef.current.querySelector('.seat-bar').getBoundingClientRect();
    const move = (ev) => {
      const v = clamp(((ev.clientX - r.left) / r.width) * 100, 0, 100);
      onChange({ intensity: Math.round(v), mute: false });
    };
    move(e);
    const up = () => { window.removeEventListener('pointermove', move); window.removeEventListener('pointerup', up); };
    window.addEventListener('pointermove', move); window.addEventListener('pointerup', up);
  };

  // Wheel/vertical-drag on the seat card adjusts intensity.
  //   • wheel: −deltaY → up (louder), each notch ≈ ±4
  //   • vertical drag started before the long-press fires → intensity scrub
  //     (every 2px of vertical movement ≈ 1 unit; up = louder)
  //   • horizontal drag before long-press → cancels (lets the page scroll, etc.)
  //   • after long-press (280ms held still) → relocate, as before
  const onWheel = (e) => {
    if (e.target.closest('.seat-btn, .seat-x')) return;
    e.preventDefault();
    const step = e.shiftKey ? 1 : 4;
    const delta = -Math.sign(e.deltaY) * step;
    onChange({ intensity: clamp(Math.round(state.intensity + delta), 0, 100), mute: false });
  };

  const onCardDown = (e) => {
    if (e.target.closest('.seat-bar, .seat-btn, .seat-x')) return;
    const startX = e.clientX, startY = e.clientY;
    let armed = false;       // long-press fired → relocate mode
    let scrubbing = false;   // vertical drag → intensity scrub
    const intStart = state.intensity;
    const padX = 0.07, padY = 0.06;
    const stageRect = stageRef.current.getBoundingClientRect();
    const toGrid = (clientX, clientY) => {
      const rx = (clientX - stageRect.left) / stageRect.width;
      const ry = (clientY - stageRect.top) / stageRect.height;
      return {
        x: clamp((rx - padX) / (1 - padX * 2), 0.02, 0.98),
        y: clamp((ry - padY) / (1 - padY * 2), 0.02, 0.98),
      };
    };
    const arm = setTimeout(() => {
      if (scrubbing) return; // user is already scrubbing intensity — don't promote to relocate
      armed = true;
      setDragging(true);
      const g = toGrid(startX, startY);
      onMove(g.x, g.y);
    }, 280);
    const move = (ev) => {
      const dx = ev.clientX - startX, dy = ev.clientY - startY;
      if (!armed && !scrubbing) {
        // Decide which gesture this drag is — wait for a clear signal (>6px)
        // then commit to one mode for the rest of the gesture.
        if (Math.hypot(dx, dy) > 6) {
          // Predominantly vertical → intensity scrub.
          // Horizontal-only → bail so the user can do something else.
          if (Math.abs(dy) > Math.abs(dx)) {
            scrubbing = true;
            clearTimeout(arm);
          } else {
            clearTimeout(arm);
            cleanup();
            return;
          }
        }
      }
      if (scrubbing) {
        // 2px per unit feels like a fader (full sweep ≈ 200px of travel).
        const v = clamp(Math.round(intStart - dy / 2), 0, 100);
        onChange({ intensity: v, mute: false });
      } else if (armed) {
        const g = toGrid(ev.clientX, ev.clientY);
        onMove(g.x, g.y);
      }
    };
    const cleanup = () => {
      clearTimeout(arm);
      setDragging(false);
      window.removeEventListener('pointermove', move);
      window.removeEventListener('pointerup', up);
    };
    const up = () => cleanup();
    window.addEventListener('pointermove', move);
    window.addEventListener('pointerup', up);
  };

  const intensity = state.mute ? 0 : state.intensity;
  const pulse = recording && !state.mute ? (intensity / 100) : 0;

  return (
    <div ref={seatRef}
         className={'seat' + (hover ? ' hover' : '') + (state.mute ? ' mute' : '') + (state.solo ? ' solo' : '') + (dragging ? ' dragging' : '')}
         style={{ left: pos.x, top: pos.y, '--accent': fam.accent }}
         onPointerDown={onCardDown}
         onWheel={onWheel}
         onMouseEnter={() => onHover(true)}
         onMouseLeave={() => onHover(false)}>

      <div className="seat-pulse" style={{ opacity: pulse * 0.6, transform: `scale(${1 + pulse * 0.6})` }} />

      <div className="seat-chairs">
        {Array.from({ length: Math.min(s.count, 6) }).map((_, i) => (
          <span key={i} className="chair" style={{ opacity: 0.3 + (intensity / 100) * 0.7 }} />
        ))}
      </div>

      <div className="seat-card">
        <button className="seat-x" onClick={(e) => { e.stopPropagation(); onRemove(); }} title="Remove">✕</button>
        <div className="seat-hd">
          <span className="seat-zh">{s.zh}</span>
          <span className="seat-count">×{s.count}</span>
        </div>
        <div className="seat-en">{s.en}</div>
        <div className="seat-bar" onPointerDown={onIntDown}>
          <div className="seat-bar-fill" style={{ width: intensity + '%', background: fam.accent }} />
          <div className="seat-bar-handle" style={{ left: intensity + '%' }} />
          <span className="seat-bar-val">{intensity}</span>
        </div>
        <div className="seat-actions">
          <button className={'seat-btn' + (state.mute ? ' on' : '')}
                  onClick={(e) => { e.stopPropagation(); onChange({ mute: !state.mute }); }}>M</button>
          <button className={'seat-btn' + (state.solo ? ' on' : '')}
                  onClick={(e) => { e.stopPropagation(); onChange({ solo: !state.solo }); }}>S</button>
          <span className="seat-family" style={{ background: fam.accent }} />
        </div>
      </div>
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// MIXER VIEW (alt mode)

function Mixer({ sections, sectionState, setSectionState, setSections, onAdd }) {
  const [pickerOpen, setPickerOpen] = useState(false);
  // id of the strip currently being dragged (long-press → drag); null when idle.
  const [dragging, setDragging] = useState(null);
  const activeIds = new Set(sections.map(s => s.id));

  const byFamily = {};
  sections.forEach(s => { (byFamily[s.family] = byFamily[s.family] || []).push(s); });

  // Move the dragged strip immediately before (or after, if at end) the strip
  // it's hovering over. Cross-family drags are ignored so the family columns
  // stay meaningful — the strip just snaps back into its own group.
  const reorder = (fromId, toId) => {
    setSections(prev => {
      const fi = prev.findIndex(s => s.id === fromId);
      const ti = prev.findIndex(s => s.id === toId);
      if (fi < 0 || ti < 0 || fi === ti) return prev;
      if (prev[fi].family !== prev[ti].family) return prev;
      const arr = [...prev];
      const [item] = arr.splice(fi, 1);
      arr.splice(ti, 0, item);
      return arr;
    });
  };

  return (
    <div className="mixer">
      {Object.entries(byFamily).map(([k, fsecs]) => {
        const f = FAMILIES[k]; if (!f) return null;
        return (
          <div key={k} className="mixer-group" data-fam={k}>
            <div className="mixer-group-hd" style={{ borderColor: f.accent }}>
              <span className="mixer-group-zh">{f.zh}</span>
              <span className="mixer-group-en">{f.en}</span>
            </div>
            <div className="mixer-strips">
              {fsecs.map(s => {
                const st = sectionState[s.id] || { intensity: 60, mute: false, solo: false };
                return (
                  <MixerStrip key={s.id} section={s} state={st} accent={f.accent}
                              dragging={dragging === s.id}
                              anyDragging={dragging != null}
                              sameFamilyAsDrag={dragging != null &&
                                sections.find(x => x.id === dragging)?.family === s.family}
                              onDragStart={() => setDragging(s.id)}
                              onDragOver={(toId) => { if (dragging && dragging !== toId) reorder(dragging, toId); }}
                              onDragEnd={() => setDragging(null)}
                              onChange={(patch) => setSectionState({ ...sectionState, [s.id]: { ...st, ...patch } })} />
                );
              })}
            </div>
          </div>
        );
      })}
      <button className="mixer-add" onClick={() => setPickerOpen(true)}
              title="新增樂器 / Add instrument">
        <span className="mixer-add-glyph">＋</span>
        <span className="mixer-add-zh">新增樂器</span>
        <span className="mixer-add-en">Add instrument</span>
      </button>
      <InstrumentPicker open={pickerOpen} onClose={() => setPickerOpen(false)}
                        activeIds={activeIds} onAdd={onAdd} />
    </div>
  );
}

// Channel strip with three gestures:
//   • Vertical drag on the fader (track or thumb): scrub intensity.
//   • Mouse wheel anywhere on the strip: each notch = 4 units (Shift = 1).
//   • Long-press (~280ms) on the strip name/body → drag-to-reorder.
//     Hovering another strip in the same family swaps their order live.
function MixerStrip({ section, state, accent, dragging, anyDragging, sameFamilyAsDrag,
                     onDragStart, onDragOver, onDragEnd, onChange }) {
  const trackRef = useRef(null);
  const stripRef = useRef(null);
  const v = state.mute ? 0 : state.intensity;

  const onFaderDown = (e) => {
    e.preventDefault();
    e.stopPropagation();
    const track = trackRef.current;
    if (!track) return;
    const r = track.getBoundingClientRect();
    const setFromY = (clientY) => {
      const rel = clamp((clientY - r.top) / r.height, 0, 1);
      onChange({ intensity: Math.round((1 - rel) * 100), mute: false });
    };
    setFromY(e.clientY);
    const move = (ev) => setFromY(ev.clientY);
    const up = () => {
      window.removeEventListener('pointermove', move);
      window.removeEventListener('pointerup', up);
    };
    window.addEventListener('pointermove', move);
    window.addEventListener('pointerup', up);
  };
  const onWheel = (e) => {
    if (e.target.closest('.strip-btn')) return;
    e.preventDefault();
    const step = e.shiftKey ? 1 : 4;
    const delta = -Math.sign(e.deltaY) * step;
    onChange({ intensity: clamp(Math.round(state.intensity + delta), 0, 100), mute: false });
  };

  // Long-press → reorder drag. Skip when the press lands on a gesture-owning
  // child (fader, M/S buttons). If the user moves >8px before the long-press
  // timer fires, we abandon — they were probably trying to flick the fader.
  const onStripDown = (e) => {
    if (e.target.closest('.strip-fader, .strip-btns, .strip-btn')) return;
    if (e.button !== undefined && e.button !== 0) return;
    const startX = e.clientX, startY = e.clientY;
    let dragged = false;
    const timer = setTimeout(() => {
      dragged = true;
      onDragStart();
    }, 280);
    const move = (ev) => {
      if (!dragged) {
        if (Math.hypot(ev.clientX - startX, ev.clientY - startY) > 8) {
          clearTimeout(timer);
          window.removeEventListener('pointermove', move);
          window.removeEventListener('pointerup', up);
        }
        return;
      }
      // Hit-test for the strip under the pointer; swap if it's a different one.
      const el = document.elementFromPoint(ev.clientX, ev.clientY);
      const target = el?.closest('.strip');
      if (target && target !== stripRef.current) {
        const tid = target.dataset.sid;
        if (tid) onDragOver(tid);
      }
    };
    const up = () => {
      clearTimeout(timer);
      if (dragged) onDragEnd();
      window.removeEventListener('pointermove', move);
      window.removeEventListener('pointerup', up);
    };
    window.addEventListener('pointermove', move);
    window.addEventListener('pointerup', up);
  };

  const cls = 'strip'
    + (dragging ? ' dragging' : '')
    + (anyDragging && !dragging && sameFamilyAsDrag ? ' drop-target' : '')
    + (anyDragging && !sameFamilyAsDrag && !dragging ? ' drop-disabled' : '');

  return (
    <div className={cls} ref={stripRef} data-sid={section.id}
         onPointerDown={onStripDown} onWheel={onWheel}>
      <div className="strip-name">{section.zh}<span>{section.en}</span></div>
      <div className="strip-fader" onPointerDown={onFaderDown}>
        <div className="strip-track" ref={trackRef}>
          <div className="strip-fill" style={{ height: v + '%', background: accent }} />
          <div className="strip-thumb" style={{ bottom: `calc(${v}% - 5px)`, background: accent }} />
        </div>
      </div>
      <div className="strip-val">{v}</div>
      <div className="strip-btns">
        <button className={'strip-btn' + (state.mute ? ' on' : '')}
                onClick={() => onChange({ mute: !state.mute })}>M</button>
        <button className={'strip-btn' + (state.solo ? ' on' : '')}
                onClick={() => onChange({ solo: !state.solo })}>S</button>
      </div>
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// SCORE VIEW

function Score({ time, recording }) {
  return (
    <div className="score">
      <div className="score-hd">
        <div className="score-title">即時生成譜面</div>
        <div className="score-sub">Auto-generated · 4/4 · ♩=92 · C major</div>
      </div>
      <div className="staff-stack">
        {['Voice 人聲', 'Strings 弦樂', 'Woodwinds 木管', 'Brass 銅管', 'Percussion 打擊'].map((name, i) => (
          <div key={name} className="staff-row">
            <div className="staff-name">{name}</div>
            <div className="staff">
              {[0,1,2,3,4].map(j => <span key={j} className="staff-line" />)}
              <div className="staff-clef">{i === 0 ? '𝄞' : i === 4 ? '𝄢' : '𝄞'}</div>
              {/* notes */}
              {TRACK.map((n, idx) => {
                const x = 8 + (n.t / 12) * 92;
                const past = recording && time >= n.t;
                return (
                  <span key={idx}
                        className={'note n-' + i + (past ? ' past' : '')}
                        style={{ left: x + '%', top: (15 + (idx % 5) * 8) + 'px' }}>
                    ♪
                  </span>
                );
              })}
              <span className="staff-cursor" style={{ left: (8 + clamp(time / 12, 0, 1) * 92) + '%', opacity: recording ? 1 : 0.2 }} />
            </div>
          </div>
        ))}
      </div>
      <div className="score-chords">
        {TRACK.map((n, i) => (
          <span key={i} className={'chord-chip' + (recording && time >= n.t && time < (TRACK[i+1]?.t ?? Infinity) ? ' active' : '')}>{n.chord}</span>
        ))}
      </div>
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// PROCESSING / PLAYBACK

function ProcessingPanel({ processState, processResult, playback, onPlayResult, onProcessAgain, activeAudioSource, compact = false }) {
  const status = processState.status;
  const isProcessing = status === 'processing';
  const isReady = status === 'ready' && processResult;
  const progress = isProcessing ? processState.progress : isReady ? 100 : 0;
  const stage = PROCESS_STAGES[processState.stageIndex] || PROCESS_STAGES[0];
  const playing = playback.state === 'playing';
  const title = isProcessing ? stage.zh : isReady ? '處理完成' : status === 'error' ? '處理中斷' : '等待錄音';
  const queuedSource = activeAudioSource ? `${activeAudioSource.name} · ready` : 'Record take queue';
  const subtitle = isProcessing ? stage.detail : isReady ? `${processResult.format} · ${processResult.lufs} LUFS` : queuedSource;

  return (
    <section className={'process-panel ' + status + (compact ? ' compact' : '')}
             data-testid="processing-panel"
             aria-live="polite">
      <div className="process-hd">
        <div>
          <div className="panel-eyebrow">PROCESS · 整合處理</div>
          <div className="process-title">{title}<span>{stage.en}</span></div>
        </div>
        <div className="process-badge">{Math.round(progress)}%</div>
      </div>

      <div className="process-bar">
        <div className="process-fill" style={{ width: progress + '%' }} />
      </div>

      {isProcessing && (
        <div className="process-steps">
          {PROCESS_STAGES.map((item, index) => (
            <div key={item.zh}
                 className={'process-step' + (index < processState.stageIndex ? ' done' : '') + (index === processState.stageIndex ? ' on' : '')}>
              <span className="process-dot" />
              <span>{item.zh}</span>
            </div>
          ))}
        </div>
      )}

      {!isProcessing && !isReady && (
        <div className="process-empty">
          <span className="process-empty-main">{subtitle}</span>
          <span className="process-empty-sub">
            {activeAudioSource ? '可直接編制選取音檔，或重新錄音建立新的來源。' : '停止錄音後會自動分析、套用效果並建立可播放預覽。'}
          </span>
        </div>
      )}

      {isReady && (
        <>
          <div className="result-grid">
            <ResultItem label="Vocal" value={processResult.vocal} />
            <ResultItem label="Mix" value={processResult.mix} />
            <ResultItem label="Score" value={`${processResult.notes} notes · ${processResult.key}`} />
            <ResultItem label="Parts" value={`${processResult.activeSections.length} sections`} />
            <ResultItem label="Source" value={`${processResult.sourceType} · ${processResult.sourceName}`} />
          </div>
          <div className="transport">
            <button className={'btn ' + (playing ? 'btn-stop' : 'btn-rec')}
                    onClick={onPlayResult}
                    data-testid="play-result">
              {playing ? <><span className="rec-square" /> 停止播放</> : <><span className="play-triangle" /> 播放處理結果</>}
            </button>
            <button className="btn btn-ghost" onClick={() => onProcessAgain()}>重新處理</button>
          </div>
          <div className="playback-bar" data-on={playing ? '1' : '0'}>
            <div className="playback-fill" style={{ width: playback.progress + '%' }} />
          </div>
        </>
      )}
    </section>
  );
}

function ResultItem({ label, value }) {
  return (
    <div className="result-item">
      <span>{label}</span>
      <b>{value}</b>
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// EXPORT MODAL

function ExportModal({ open, onClose, processState, processResult, playback, onPlayResult, onProcessAgain, activeAudioSource }) {
  const [stage, setStage] = useState(0); // 0 idle, 1 rendering, 2 done
  const [pct, setPct] = useState(0);

  useEffect(() => {
    if (!open) { setStage(0); setPct(0); return; }
  }, [open]);

  const start = () => {
    if (!processResult) {
      onProcessAgain();
      return;
    }
    setStage(1); setPct(0);
    const iv = setInterval(() => {
      setPct(p => {
        const nx = p + 6 + Math.random() * 8;
        if (nx >= 100) { clearInterval(iv); setStage(2); return 100; }
        return nx;
      });
    }, 110);
  };

  if (!open) return null;
  return (
    <div className="modal-bg" onClick={onClose}>
      <div className="modal" onClick={e => e.stopPropagation()}>
        <div className="modal-hd">
          <div>
            <div className="panel-eyebrow">EXPORT · 匯出</div>
            <div className="modal-title">合奏匯出 / Bounce ensemble</div>
          </div>
          <button className="modal-x" onClick={onClose}>✕</button>
        </div>

        {stage < 2 ? (
          <>
            <div className="export-grid">
              <ExportRow label="音檔來源 / Source"     value={processResult ? `${processResult.sourceType} · ${processResult.sourceName}` : activeAudioSource?.name || '尚未選取'} />
              <ExportRow label="檔案格式 / Format"   value={processResult?.format || 'WAV · 48 kHz · 24-bit'} />
              <ExportRow label="人聲處理 / Vocal"    value={processResult?.vocal || '保留乾聲 + Reverb · Hall A'} />
              <ExportRow label="樂器混音 / Mix"      value={processResult?.mix || '自動配重 · 等待處理結果'} />
              <ExportRow label="總長度 / Duration"   value={processResult ? formatTime(processResult.durationSeconds) : '00:12.40'} />
              <ExportRow label="LUFS"                value={(processResult?.lufs || '-14.0') + ' (streaming)'} />
            </div>
            <ProcessingPanel processState={processState}
                             processResult={processResult}
                             playback={playback}
                             onPlayResult={onPlayResult}
                             onProcessAgain={onProcessAgain}
                             activeAudioSource={activeAudioSource}
                             compact />
            <div className="export-progress" data-on={stage === 1 ? '1' : '0'}>
              <div className="export-bar"><div className="export-fill" style={{ width: pct + '%' }} /></div>
              <div className="export-pct">{Math.round(pct)}%</div>
            </div>
            <div className="modal-actions">
              <button className="btn btn-ghost" onClick={onClose}>取消</button>
              <button className="btn btn-rec" onClick={start} disabled={stage === 1 || processState.status === 'processing'}>
                {processState.status === 'processing' ? '處理中…' : stage === 1 ? '渲染中…' : processResult ? '開始渲染 / Render' : '開始整合處理'}
              </button>
            </div>
          </>
        ) : (
          <div className="export-done">
            <div className="export-done-mark">✓</div>
            <div className="export-done-title">完成！合奏已匯出</div>
            <div className="export-done-sub">music_elf_bounce_2026-05-06.wav · 4.8 MB</div>
            <div className="modal-actions">
              <button className="btn btn-ghost" onClick={onClose}>關閉</button>
              <button className="btn btn-rec">下載 / Download</button>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

function ExportRow({ label, value }) {
  return (
    <div className="export-row">
      <div className="export-label">{label}</div>
      <div className="export-value">{value}</div>
    </div>
  );
}

// ─────────────────────────────────────────────────────────────────────────────
// APP

const TWEAK_DEFAULTS = /*EDITMODE-BEGIN*/{
  "palette": "warm",
  "view": "conductor",
  "showLatin": true,
  "haloIntensity": 70,
  "podiumStyle": "disk"
}/*EDITMODE-END*/;

// ─────────────────────────────────────────────────────────────────────────────
// MOBILE LAYOUT — no conductor stage. Compact voice header, tabbed body
// (mixer / score / style), sticky record bar at the bottom.

function MobileVoiceStrip({ recording, time, current, pitchHistory }) {
  const pathD = useMemo(() => {
    if (!pitchHistory.length) return '';
    return pitchHistory.map((p, i) => {
      const x = (i / Math.max(1, pitchHistory.length - 1)) * 100;
      const y = 100 - clamp(p, 0, 100);
      return `${i === 0 ? 'M' : 'L'} ${x.toFixed(2)} ${y.toFixed(2)}`;
    }).join(' ');
  }, [pitchHistory]);

  const activeLyric = TRACK.slice().reverse().find(l => recording && time >= l.t);

  return (
    <div className="m-voice">
      <div className="m-voice-pitch">
        <svg viewBox="0 0 100 100" preserveAspectRatio="none">
          <path d={pathD} fill="none" stroke="var(--accent)" strokeWidth="1.2"
                strokeLinecap="round" strokeLinejoin="round" vectorEffect="non-scaling-stroke" />
          {pitchHistory.length > 0 && (
            <circle cx={100} cy={100 - clamp(pitchHistory[pitchHistory.length-1], 0, 100)}
                    r="1.8" fill="var(--accent)" vectorEffect="non-scaling-stroke" />
          )}
        </svg>
        <div className="m-voice-now">
          <span className="m-voice-note">{current.pitch}</span>
          <span className={'m-voice-led' + (recording ? ' on' : '')} />
        </div>
      </div>
      <div className="m-voice-lyric">
        <span className="m-voice-chord">{activeLyric ? activeLyric.chord : current.chord}</span>
        <span className="m-voice-text">{activeLyric ? activeLyric.lyric : (recording ? '聆聽中…' : '等待開始錄音')}</span>
      </div>
    </div>
  );
}

function MobileMixer({ sections, sectionState, setSectionState, setSections, onAdd }) {
  const [pickerOpen, setPickerOpen] = useState(false);
  const activeIds = new Set(sections.map(s => s.id));
  return (
    <div className="m-mixer">
      {sections.map(s => {
        const st = sectionState[s.id] || { intensity: 60, mute: false, solo: false };
        const fam = FAMILIES[s.family];
        const v = st.mute ? 0 : st.intensity;
        const onSet = (patch) => setSectionState({ ...sectionState, [s.id]: { ...st, ...patch } });
        return (
          <div key={s.id} className={'m-strip' + (st.mute ? ' mute' : '') + (st.solo ? ' solo' : '')}>
            <div className="m-strip-hd">
              <span className="m-strip-dot" style={{ background: fam.accent }} />
              <span className="m-strip-zh">{s.zh}</span>
              <span className="m-strip-en">{s.en}</span>
              <span className="m-strip-val">{v}</span>
              <button className="m-strip-x" aria-label="Remove"
                      onClick={() => setSections(prev => prev.filter(x => x.id !== s.id))}>✕</button>
            </div>
            <input className="m-strip-fader" type="range" min={0} max={100} value={v}
                   onChange={(e) => onSet({ intensity: Number(e.target.value), mute: false })}
                   style={{ '--accent': fam.accent }} />
            <div className="m-strip-btns">
              <button className={'m-strip-btn' + (st.mute ? ' on' : '')}
                      onClick={() => onSet({ mute: !st.mute })}>M</button>
              <button className={'m-strip-btn' + (st.solo ? ' on' : '')}
                      onClick={() => onSet({ solo: !st.solo })}>S</button>
            </div>
          </div>
        );
      })}
      <button className="m-mixer-add" onClick={() => setPickerOpen(true)}>
        <span className="m-mixer-add-glyph">＋</span>
        <span>新增樂器 · Add instrument</span>
      </button>
      <InstrumentPicker open={pickerOpen} onClose={() => setPickerOpen(false)}
                        activeIds={activeIds} onAdd={onAdd} />
    </div>
  );
}

function MobileStylePanel({ style, setStyle, master, setMaster, swell, setSwell,
                            favorites, onSave, onLoad, onDelete }) {
  return (
    <div className="m-style">
      <div className="panel-eyebrow">STYLE · 風格</div>
      <div className="m-style-grid">
        {STYLES.map(s => (
          <button key={s.id} className={'style-card' + (style === s.id ? ' on' : '')}
                  onClick={() => setStyle(s.id)}>
            <span className="style-zh">{s.zh}</span>
            <span className="style-en">{s.en}</span>
            <span className="style-sub">{s.sub}</span>
          </button>
        ))}
      </div>
      <div className="panel-eyebrow" style={{marginTop: 16}}>CONDUCTOR · 指揮</div>
      <Slider label="總音量 / Master" value={master} onChange={setMaster} unit="dB" min={-30} max={6} />
      <Slider label="氣勢 / Swell"     value={swell}  onChange={setSwell}  unit="" min={0} max={100} />
      <FavoritesSection favorites={favorites} onSave={onSave} onLoad={onLoad} onDelete={onDelete} />
    </div>
  );
}

function App() {
  const [t, setTweak] = useTweaks(TWEAK_DEFAULTS);
  const isMobile = useIsMobile();

  const [mode, setMode] = useState('conductor');
  const [recording, setRecording] = useState(false);
  const [time, setTime] = useState(0);
  const [style, setStyle] = useState('rock');
  // Active ensemble — list of section objects (cloned from preset so we can move/remove)
  const [sections, setSections] = useState(() => STYLE_PRESETS.rock.map(s => ({ ...s })));
  const [sectionState, setSectionState] = useState(() => {
    const o = {};
    STYLE_PRESETS.rock.forEach(s => { o[s.id] = { intensity: 60 + Math.floor(Math.random()*30), mute: false, solo: false }; });
    return o;
  });
  // Switching style swaps in that preset's ensemble.
  const onStyleChange = (id) => {
    setStyle(id);
    const next = STYLE_PRESETS[id].map(s => ({ ...s }));
    setSections(next);
    setSectionState(prev => {
      const o = {};
      next.forEach(s => { o[s.id] = prev[s.id] || { intensity: 60 + Math.floor(Math.random()*30), mute: false, solo: false }; });
      return o;
    });
  };
  // `pos` is optional {x, y} in 0..1 grid coords — set by the on-stage long-press
  // / double-click quick-add so the new seat lands exactly where the conductor
  // tapped. Without it we sprinkle it near the top of the stage.
  const onAddInstrument = (catalogId, pos) => {
    if (sections.find(s => s.id === catalogId)) return; // already on stage
    const meta = CATALOG[catalogId];
    if (!meta) return;
    const fallback = { x: 0.5 + (Math.random() - 0.5) * 0.4, y: 0.30 + Math.random() * 0.2 };
    const where = pos || fallback;
    const slot = { ...meta, id: catalogId, x: where.x, y: where.y };
    setSections(s => [...s, slot]);
    setSectionState(prev => ({
      ...prev,
      [catalogId]: { intensity: 60, mute: false, solo: false, pos: where, justAdded: Date.now() },
    }));
  };
  const onRemoveInstrument = (id) => {
    setSections(s => s.filter(x => x.id !== id));
  };
  const [master, setMaster] = useState(-3);
  const [swell, setSwell] = useState(62);
  const [hover, setHover] = useState(null);
  const [baton, setBaton] = useState({ x: 700, y: 480 });
  const [pitchHistory, setPitchHistory] = useState([]);
  const [exportOpen, setExportOpen] = useState(false);
  const [processState, setProcessState] = useState({ status: 'idle', progress: 0, stageIndex: 0 });
  const [processResult, setProcessResult] = useState(null);
  const [playback, setPlayback] = useState({ state: 'idle', progress: 0 });
  const [saveRecording, setSaveRecording] = useState(() => {
    try { return localStorage.getItem(SAVE_RECORDING_KEY) !== '0'; }
    catch { return true; }
  });
  const [audioTakes, setAudioTakes] = useState(() => {
    try {
      const raw = localStorage.getItem(AUDIO_TAKES_KEY);
      const arr = raw ? JSON.parse(raw) : [];
      return Array.isArray(arr) ? arr.slice(0, AUDIO_TAKE_LIMIT) : [];
    } catch { return []; }
  });
  const [activeAudioSource, setActiveAudioSource] = useState(null);
  const [captureState, setCaptureState] = useState({
    status: 'idle',
    message: '錄音可儲存在本機，或選擇既有音檔編制。',
  });
  const processTimerRef = useRef(null);
  const playbackRef = useRef({ ctx: null, raf: null, startedAt: 0, duration: 0 });
  const mediaRecorderRef = useRef(null);
  const mediaStreamRef = useRef(null);
  const recordedChunksRef = useRef([]);
  const recordingSessionRef = useRef(0);

  const [favorites, setFavorites] = useState(() => {
    try {
      const raw = localStorage.getItem('musicelf.favorites.v1');
      const arr = raw ? JSON.parse(raw) : [];
      return Array.isArray(arr) ? arr.slice(0, FAV_MAX) : [];
    } catch { return []; }
  });
  useEffect(() => {
    try { localStorage.setItem('musicelf.favorites.v1', JSON.stringify(favorites)); }
    catch { /* quota or disabled — skip */ }
  }, [favorites]);

  useEffect(() => {
    try { localStorage.setItem(SAVE_RECORDING_KEY, saveRecording ? '1' : '0'); }
    catch { /* localStorage disabled — keep session value only */ }
  }, [saveRecording]);

  useEffect(() => {
    try { localStorage.setItem(AUDIO_TAKES_KEY, JSON.stringify(audioTakes)); }
    catch { /* metadata is optional; blobs still remain in IndexedDB */ }
  }, [audioTakes]);

  const onSaveFavorite = () => {
    if (favorites.length >= FAV_MAX) return;
    const styleMeta = STYLES.find(s => s.id === style);
    const suggested = `${styleMeta ? styleMeta.zh : style} · ${sections.length}件`;
    const name = (window.prompt('編制名稱 / Name this ensemble:', suggested) || '').trim();
    if (!name) return;
    const snap = {
      id: 'fav_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6),
      name: name.slice(0, 40),
      style,
      sections: sections.map(s => ({ ...s })),
      sectionState: JSON.parse(JSON.stringify(sectionState)),
      savedAt: Date.now(),
    };
    setFavorites(prev => [snap, ...prev].slice(0, FAV_MAX));
  };
  const onLoadFavorite = (id) => {
    const f = favorites.find(x => x.id === id);
    if (!f) return;
    setStyle(f.style);
    setSections(f.sections.map(s => ({ ...s })));
    setSectionState(JSON.parse(JSON.stringify(f.sectionState || {})));
  };
  const onDeleteFavorite = (id) => {
    setFavorites(prev => prev.filter(f => f.id !== id));
  };

  // Apply palette to root
  useEffect(() => {
    document.documentElement.dataset.palette = t.palette;
  }, [t.palette]);

  // Recording timer
  useEffect(() => {
    if (!recording) return;
    let raf, last = performance.now();
    const tick = (now) => {
      const dt = (now - last) / 1000; last = now;
      setTime(p => Math.min(12, p + dt));
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [recording]);

  // Pitch history while recording
  useEffect(() => {
    if (!recording) return;
    const iv = setInterval(() => {
      setPitchHistory(h => {
        const next = [...h];
        // synth a wandering pitch with vibrato
        const tt = (next.length / 60) * Math.PI;
        const v = 50 + Math.sin(tt * 0.7) * 22 + Math.sin(tt * 5) * 6 + (Math.random() - 0.5) * 4;
        next.push(clamp(v, 0, 100));
        if (next.length > 120) next.shift();
        return next;
      });
    }, 80);
    return () => clearInterval(iv);
  }, [recording]);

  const current = useMemo(() => {
    let cur = TRACK[0];
    for (const line of TRACK) if (time >= line.t) cur = line;
    return cur;
  }, [time]);

  const stopPlayback = () => {
    clearPlaybackEngine(playbackRef);
    setPlayback({ state: 'idle', progress: 0 });
  };

  const stopInputStream = () => {
    mediaStreamRef.current?.getTracks().forEach(track => track.stop());
    mediaStreamRef.current = null;
  };

  const startBrowserRecording = async (sessionId) => {
    recordedChunksRef.current = [];
    if (!navigator.mediaDevices?.getUserMedia || !window.MediaRecorder) {
      setCaptureState({
        status: 'simulated',
        message: '瀏覽器無法直接錄音，停止後會儲存一段可測試的示範音檔。',
      });
      return;
    }

    try {
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
      if (sessionId !== recordingSessionRef.current) {
        stream.getTracks().forEach(track => track.stop());
        return;
      }
      const mimeType = preferredRecordingMime();
      const recorder = mimeType ? new MediaRecorder(stream, { mimeType }) : new MediaRecorder(stream);
      recorder.ondataavailable = (event) => {
        if (event.data?.size) recordedChunksRef.current.push(event.data);
      };
      recorder.start(250);
      mediaStreamRef.current = stream;
      mediaRecorderRef.current = recorder;
      setCaptureState({
        status: 'recording',
        message: saveRecording ? '麥克風錄音中，停止後會儲存到錄音檔庫。' : '麥克風錄音中，此次不會儲存。',
      });
    } catch (err) {
      stopInputStream();
      mediaRecorderRef.current = null;
      setCaptureState({
        status: 'simulated',
        message: '無法取得麥克風權限，停止後會使用示範音檔完成流程。',
      });
    }
  };

  const stopBrowserRecording = () => new Promise((resolve) => {
    const recorder = mediaRecorderRef.current;
    const finish = () => {
      stopInputStream();
      mediaRecorderRef.current = null;
      const type = recordedChunksRef.current[0]?.type || recorder?.mimeType || 'audio/webm';
      const blob = recordedChunksRef.current.length ? new Blob(recordedChunksRef.current, { type }) : null;
      recordedChunksRef.current = [];
      resolve(blob);
    };

    if (!recorder || recorder.state === 'inactive') {
      finish();
      return;
    }

    recorder.onstop = finish;
    try {
      recorder.stop();
    } catch {
      finish();
    }
  });

  const saveRecordedTake = async (blob, durationSeconds) => {
    const savedAt = Date.now();
    const meta = {
      id: 'audio_' + savedAt.toString(36) + Math.random().toString(36).slice(2, 6),
      name: `錄音 ${formatTakeStamp(savedAt)}`,
      origin: 'library',
      durationSeconds: clamp(durationSeconds || 12, 1, 180),
      mimeType: blob.type || 'audio/wav',
      size: blob.size,
      savedAt,
    };

    try {
      await putAudioBlob(meta.id, blob);
      setAudioTakes(prev => {
        const next = [meta, ...prev].slice(0, AUDIO_TAKE_LIMIT);
        prev.slice(AUDIO_TAKE_LIMIT - 1).forEach(oldTake => deleteAudioBlob(oldTake.id).catch(() => {}));
        return next;
      });
      setCaptureState({ status: 'saved', message: `已儲存 ${meta.name}，可稍後重新編制。` });
      return meta;
    } catch {
      setCaptureState({ status: 'error', message: '錄音儲存失敗，但仍可用目前錄音繼續編制。' });
      return null;
    }
  };

  const prepareAudioSource = (source) => {
    setActiveAudioSource(source);
    setTime(clamp(source.durationSeconds || 0, 0, 12));
    setPitchHistory(buildSourcePitchHistory(source));
  };

  const startProcessing = (sourceOverride, durationOverride) => {
    const sourceForProcessing = sourceOverride?.id ? sourceOverride : activeAudioSource;
    const analysisTime = durationOverride ?? sourceForProcessing?.durationSeconds ?? time;
    stopPlayback();
    if (processTimerRef.current) clearInterval(processTimerRef.current);
    setProcessResult(null);
    setProcessState({ status: 'processing', progress: 0, stageIndex: 0 });

    const startedAt = performance.now();
    const durationMs = 4200;
    processTimerRef.current = setInterval(() => {
      const elapsed = performance.now() - startedAt;
      const progress = clamp((elapsed / durationMs) * 100, 0, 100);
      const stageIndex = clamp(
        Math.floor((progress / 100) * PROCESS_STAGES.length),
        0,
        PROCESS_STAGES.length - 1
      );

      if (progress >= 100) {
        clearInterval(processTimerRef.current);
        processTimerRef.current = null;
        const result = buildProcessResult({
          time: analysisTime,
          style,
          sections,
          sectionState,
          master,
          swell,
          audioSource: sourceForProcessing,
        });
        setProcessResult(result);
        setProcessState({ status: 'ready', progress: 100, stageIndex: PROCESS_STAGES.length - 1 });
        return;
      }

      setProcessState({ status: 'processing', progress, stageIndex });
    }, 90);
  };

  const onImportAudio = async (event) => {
    const file = event.target.files?.[0];
    event.target.value = '';
    if (!file || recording) return;
    if (processTimerRef.current) clearInterval(processTimerRef.current);
    const durationSeconds = await readAudioDuration(file);
    const source = {
      id: 'file_' + Date.now().toString(36),
      name: file.name || 'Imported audio',
      origin: 'file',
      durationSeconds: clamp(durationSeconds, 1, 180),
      mimeType: file.type || 'audio/*',
      size: file.size,
      savedAt: Date.now(),
    };
    prepareAudioSource(source);
    setCaptureState({ status: 'loaded', message: `已載入 ${source.name}，開始編制處理。` });
    startProcessing(source, source.durationSeconds);
  };

  const onProcessSavedAudio = async (take) => {
    if (recording) return;
    try {
      const blob = await getAudioBlob(take.id);
      if (!blob) throw new Error('Missing blob');
      const source = {
        ...take,
        origin: 'library',
        size: take.size || blob.size,
        mimeType: take.mimeType || blob.type || 'audio/*',
      };
      prepareAudioSource(source);
      setCaptureState({ status: 'loaded', message: `已選取 ${source.name}，開始編制處理。` });
      startProcessing(source, source.durationSeconds);
    } catch {
      setCaptureState({ status: 'error', message: '找不到這筆錄音內容，請刪除後重新錄製。' });
    }
  };

  const onProcessActiveAudio = () => {
    if (!activeAudioSource || recording) return;
    if (activeAudioSource.origin === 'library') {
      onProcessSavedAudio(activeAudioSource);
      return;
    }
    startProcessing(activeAudioSource, activeAudioSource.durationSeconds);
  };

  const onDeleteSavedAudio = (id) => {
    setAudioTakes(prev => prev.filter(take => take.id !== id));
    if (activeAudioSource?.id === id) setActiveAudioSource(null);
    deleteAudioBlob(id).catch(() => {});
  };

  const onPlayResult = () => {
    if (playback.state === 'playing') {
      stopPlayback();
      return;
    }
    if (!processResult) return;
    startProcessedPreview(processResult, playbackRef, setPlayback);
  };

  const onToggleRec = async () => {
    if (recording) {
      const stoppedAt = clamp(time || 1, 1, 12);
      recordingSessionRef.current += 1;
      setRecording(false);
      setCaptureState({ status: 'saving', message: saveRecording ? '正在整理並儲存錄音…' : '正在整理錄音…' });
      const capturedBlob = await stopBrowserRecording();
      const usableBlob = capturedBlob?.size ? capturedBlob : createDemoTakeBlob(stoppedAt);
      const liveSource = {
        id: 'live_' + Date.now().toString(36),
        name: saveRecording ? '目前錄音' : '未儲存錄音',
        origin: 'recording',
        durationSeconds: stoppedAt,
        mimeType: usableBlob.type || 'audio/wav',
        size: usableBlob.size,
        savedAt: Date.now(),
      };
      const savedSource = saveRecording ? await saveRecordedTake(usableBlob, stoppedAt) : null;
      const source = savedSource || liveSource;
      setActiveAudioSource(source);
      if (!saveRecording) {
        setCaptureState({ status: 'ready', message: '已保留目前錄音進行編制，但沒有加入錄音檔庫。' });
      }
      startProcessing(source, stoppedAt);
    } else {
      if (processTimerRef.current) {
        clearInterval(processTimerRef.current);
        processTimerRef.current = null;
      }
      stopPlayback();
      stopInputStream();
      setProcessState({ status: 'idle', progress: 0, stageIndex: 0 });
      setProcessResult(null);
      setActiveAudioSource(null);
      setRecording(true);
      setTime(0);
      setPitchHistory([]);
      const sessionId = recordingSessionRef.current + 1;
      recordingSessionRef.current = sessionId;
      setCaptureState({
        status: 'arming',
        message: saveRecording ? '準備錄音，停止後會儲存。' : '準備錄音，此次不儲存。',
      });
      startBrowserRecording(sessionId);
    }
  };

  useEffect(() => {
    return () => {
      if (processTimerRef.current) clearInterval(processTimerRef.current);
      clearPlaybackEngine(playbackRef);
      stopInputStream();
    };
  }, []);

  // On mobile we skip the conductor stage entirely; if the saved/default mode
  // was 'conductor', flip to 'mixer' so the first paint is meaningful.
  useEffect(() => {
    if (isMobile && mode === 'conductor') setMode('mixer');
  }, [isMobile, mode]);

  if (isMobile) {
    const mobileTabs = [
      ['mixer', '混音', 'Mix'],
      ['score', '譜面', 'Score'],
      ['style', '風格', 'Style'],
    ];
    const activeMode = mode === 'conductor' ? 'mixer' : mode;
    return (
      <div className="app app-mobile">
        <BackgroundDecor />
        <Header recording={recording} time={time} onToggleRec={onToggleRec}
                mode={activeMode} setMode={setMode}
                tabs={mobileTabs} compact
                onExport={() => setExportOpen(true)} />

        <MobileVoiceStrip recording={recording} time={time}
                          current={current} pitchHistory={pitchHistory} />

        <div className="m-process-wrap">
          <AudioSourcePanel recording={recording}
                            saveRecording={saveRecording}
                            onSaveRecordingChange={setSaveRecording}
                            audioTakes={audioTakes}
                            activeAudioSource={activeAudioSource}
                            captureState={captureState}
                            onImportAudio={onImportAudio}
                            onProcessSavedAudio={onProcessSavedAudio}
                            onDeleteSavedAudio={onDeleteSavedAudio}
                            onProcessActiveAudio={onProcessActiveAudio}
                            compact />
          <ProcessingPanel processState={processState}
                           processResult={processResult}
                           playback={playback}
                           onPlayResult={onPlayResult}
                           onProcessAgain={startProcessing}
                           activeAudioSource={activeAudioSource}
                           compact />
        </div>

        <main className="m-body">
          {activeMode === 'mixer' && (
            <MobileMixer sections={sections} sectionState={sectionState}
                         setSectionState={setSectionState}
                         setSections={setSections}
                         onAdd={onAddInstrument} />
          )}
          {activeMode === 'score' && <Score time={time} recording={recording} />}
          {activeMode === 'style' && (
            <MobileStylePanel style={style} setStyle={onStyleChange}
                              master={master} setMaster={setMaster}
                              swell={swell} setSwell={setSwell}
                              favorites={favorites}
                              onSave={onSaveFavorite}
                              onLoad={onLoadFavorite}
                              onDelete={onDeleteFavorite} />
          )}
        </main>

        <div className="m-recbar">
          <div className="m-recbar-time">
            <span className="dot" data-on={recording ? '1' : '0'} />
            <span className="time-num">{formatTime(time)}</span>
          </div>
          <button className={'btn ' + (recording ? 'btn-stop' : 'btn-rec') + ' m-recbar-btn'}
                  onClick={onToggleRec}>
            {recording
              ? <><span className="rec-square" /> 停止</>
              : <><span className="rec-circle" /> 開始錄音</>}
          </button>
          <button className="btn btn-ghost m-recbar-export" onClick={() => setExportOpen(true)}>匯出</button>
        </div>

        <ExportModal open={exportOpen} onClose={() => setExportOpen(false)}
                     processState={processState}
                     processResult={processResult}
                     playback={playback}
                     onPlayResult={onPlayResult}
                     onProcessAgain={startProcessing}
                     activeAudioSource={activeAudioSource} />
        <ApplyDOMFlags showLatin={t.showLatin} haloIntensity={t.haloIntensity} />
      </div>
    );
  }

  return (
    <div className="app">
      <BackgroundDecor />
      <Header recording={recording} time={time} onToggleRec={onToggleRec}
              mode={mode} setMode={setMode}
              onExport={() => setExportOpen(true)} />

      <main className="grid">
        <VoicePanel recording={recording} time={time} current={current} pitchHistory={pitchHistory}
                    processState={processState}
                    processResult={processResult}
                    playback={playback}
                    onPlayResult={onPlayResult}
                    onProcessAgain={startProcessing}
                    saveRecording={saveRecording}
                    onSaveRecordingChange={setSaveRecording}
                    audioTakes={audioTakes}
                    activeAudioSource={activeAudioSource}
                    captureState={captureState}
                    onImportAudio={onImportAudio}
                    onProcessSavedAudio={onProcessSavedAudio}
                    onDeleteSavedAudio={onDeleteSavedAudio}
                    onProcessActiveAudio={onProcessActiveAudio} />

        <section className="center">
          <div className="center-hd">
            <div className="center-title">
              <span className="center-zh">{({conductor:'指揮台',mixer:'混音台',score:'譜面'})[mode]}</span>
              <span className="center-en">{({conductor:'Conductor stage',mixer:'Mixer console',score:'Live score'})[mode]}</span>
            </div>
            <div className="center-meta">
              <span className="meta-pill">{STYLES.find(s => s.id === style)?.zh} · {STYLES.find(s => s.id === style)?.en}</span>
              <span className="meta-pill">{sections.length} 件樂器 · instruments</span>
              <span className="meta-pill">{sections.reduce((n, s) => n + (s.count || 1), 0)} musicians</span>
              <span className="meta-pill">C major · ♩=92</span>
            </div>
          </div>

          {mode === 'conductor' && (
            <Stage sections={sections} sectionState={sectionState} setSectionState={setSectionState}
                   hover={hover} setHover={setHover}
                   baton={baton} setBaton={setBaton}
                   onAdd={onAddInstrument} onRemove={onRemoveInstrument}
                   recording={recording} />
          )}
          {mode === 'mixer' && <Mixer sections={sections} sectionState={sectionState} setSectionState={setSectionState}
                                       setSections={setSections} onAdd={onAddInstrument} />}
          {mode === 'score' && <Score time={time} recording={recording} />}

          <Footer recording={recording} time={time} />
        </section>

        <ControlsPanel style={style} setStyle={onStyleChange}
                       master={master} setMaster={setMaster}
                       swell={swell} setSwell={setSwell}
                       favorites={favorites}
                       onSaveFavorite={onSaveFavorite}
                       onLoadFavorite={onLoadFavorite}
                       onDeleteFavorite={onDeleteFavorite} />
      </main>

      <ExportModal open={exportOpen} onClose={() => setExportOpen(false)}
                   processState={processState}
                   processResult={processResult}
                   playback={playback}
                   onPlayResult={onPlayResult}
                   onProcessAgain={startProcessing}
                   activeAudioSource={activeAudioSource} />

      <TweaksPanel title="Tweaks">
        <TweakSection label="Theme">
          <TweakColor label="Palette" value={t.palette}
                      options={[
                        {value:'warm',  label:'warm'},
                        {value:'cool',  label:'cool'},
                        {value:'noir',  label:'noir'},
                      ].map(o => o.value)}
                      onChange={(v) => setTweak('palette', v)} />
        </TweakSection>
        <TweakSection label="Layout">
          <TweakRadio label="View"
                      value={mode}
                      options={[{value:'conductor',label:'Stage'},{value:'mixer',label:'Mix'},{value:'score',label:'Score'}]}
                      onChange={(v) => setMode(v)} />
          <TweakToggle label="Show Latin labels" value={t.showLatin}
                       onChange={(v) => setTweak('showLatin', v)} />
        </TweakSection>
        <TweakSection label="Stage">
          <TweakSlider label="Halo intensity" value={t.haloIntensity} min={0} max={100}
                       onChange={(v) => setTweak('haloIntensity', v)} />
        </TweakSection>
      </TweaksPanel>

      <style>{`html[data-show-latin="0"] .en, html[data-show-latin="0"] .seat-en, html[data-show-latin="0"] .style-en, html[data-show-latin="0"] .family-en, html[data-show-latin="0"] .center-en, html[data-show-latin="0"] .panel-title-en { display: none; }`}</style>
      <ApplyDOMFlags showLatin={t.showLatin} haloIntensity={t.haloIntensity} />
    </div>
  );
}

function ApplyDOMFlags({ showLatin, haloIntensity }) {
  useEffect(() => { document.documentElement.dataset.showLatin = showLatin ? '1' : '0'; }, [showLatin]);
  useEffect(() => { document.documentElement.style.setProperty('--halo-intensity', haloIntensity / 100); }, [haloIntensity]);
  return null;
}

function Footer({ recording, time }) {
  return (
    <div className="footer">
      <div className="ftr-l">
        <div className="ftr-tip">
          <span className="kbd">M</span>ute · <span className="kbd">S</span>olo · drag intensity bar · <span className="kbd">long-press</span> seat to move ·
          tap empty area to move <span className="kbd">baton</span> · <span className="kbd">long-press</span>／<span className="kbd">dbl-click</span> empty area to add instrument
        </div>
      </div>
      <div className="ftr-r">
        <div className="timeline">
          <div className="timeline-track">
            <div className="timeline-fill" style={{ width: (clamp(time / 12, 0, 1) * 100) + '%' }} />
            {TRACK.map((n, i) => (
              <span key={i} className="timeline-mark" style={{ left: ((n.t / 12) * 100) + '%' }} title={n.chord} />
            ))}
          </div>
          <div className="timeline-time">{formatTime(time)} / 00:12.00</div>
        </div>
      </div>
    </div>
  );
}

function BackgroundDecor() {
  return (
    <div className="bgdecor" aria-hidden="true">
      <div className="bgd bgd-1" />
      <div className="bgd bgd-2" />
      <div className="bgd-grain" />
    </div>
  );
}

ReactDOM.createRoot(document.getElementById('root')).render(<App />);
