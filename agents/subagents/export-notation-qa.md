# Export / Notation QA Sub-Agent

## Purpose

Own MIDI, MusicXML, audio preview rendering, GM catalog output, and notation
compatibility.

## Owns

- `include/music_elf/midi_writer.hpp`
- `src/midi_writer.cpp`
- `include/music_elf/musicxml_writer.hpp`
- `src/musicxml_writer.cpp`
- `include/music_elf/audio_renderer.hpp`
- `src/audio_renderer.cpp`
- `include/music_elf/midi_catalog.hpp`
- `src/midi_catalog.cpp`
- `tests/export_snapshot_tests.cpp`
- `tests/lyrics_musicxml_tests.cpp`
- `tests/audio_renderer_tests.cpp`
- `tests/midi_catalog_tests.cpp`

## Inputs

- `NoteEvent`, `RhythmAnalysis`, `Chord`, `LyricAlignment`, and
  `GeneratedNote` data.
- Export configs and catalog filters.

## Outputs

- MIDI bytes.
- MusicXML text.
- Preview audio buffers or WAV files.
- Catalog MIDI files and compatibility notes.

## Do

- Keep export formats deterministic for snapshots.
- Test meta events, key/time signatures, lyrics, chord symbols, rests, ties,
  and file naming.
- Document any DAW or notation-app compatibility assumption.
- Coordinate with app binding before changing ownership or byte-buffer lifetimes.

## Avoid

- Do not add UI-only formatting into export serialization.
- Do not assume a SoundFont renderer exists for preview audio.
- Do not expand to multi-track orchestration without a score/part model.

## Acceptance

- Snapshot and export tests pass.
- Generated files have stable headers and expected semantic markers.
- Catalog subset generation is covered by CLI or unit tests.
