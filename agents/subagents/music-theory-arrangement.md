# Music Theory / Arrangement Sub-Agent

## Purpose

Own key detection, chord generation, accompaniment patterns, voice leading, and
future orchestration rules.

## Owns

- `include/music_elf/harmony_analyzer.hpp`
- `src/harmony_analyzer.cpp`
- `include/music_elf/accompaniment_generator.hpp`
- `src/accompaniment_generator.cpp`
- `tests/harmony_tests.cpp`
- `tests/arrangement_midi_tests.cpp`

## Inputs

- `NoteEvent` melody.
- `RhythmAnalysis`.
- Style intent such as pop, ballad, jazz, cinematic, or classical.
- Arrangement parameters such as range, inversion, pattern, and channel.

## Outputs

- `KeyEstimate`.
- `ChordProgression` candidates.
- `GeneratedNote` accompaniment notes.
- Arrangement rationale and limitations.

## Do

- Keep harmony scoring explainable and deterministic.
- Respect pitch-class, chord quality, range, and voice-leading constraints.
- Add tests for chord coverage, style templates, inversions, range limits, and
  smoother movement.
- Coordinate with export work before adding multi-part or multi-track behavior.

## Avoid

- Do not overload `ChordQuality` with unsupported chord symbols.
- Do not represent orchestration parts as anonymous single-track notes.
- Do not assume full-song chord recognition from mixed audio exists.

## Acceptance

- Harmony and arrangement tests pass.
- Chord symbols remain valid for MusicXML export.
- Generated notes stay within configured ranges.
