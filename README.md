# Music Elf

Native C++ core for realtime monophonic singing analysis, note extraction,
harmonization, accompaniment generation, and editable score export.

## Build and Test on Windows

This repository is configured for C++17 with CMake and CTest. On the current
Windows development machine, Visual Studio Community 2022 provides both MSVC and
CMake.

```powershell
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"" -S . -B build -G ""Visual Studio 17 2022"" -A x64"
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"" --build build --config Release"
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && cd build && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"" -C Release --output-on-failure"
```

## CLI Demo

The `music_elf_cli` executable runs the native pipeline from a WAV file to
editable outputs:

```powershell
build\Release\music_elf_cli.exe input.wav --out-midi output.mid --out-musicxml output.musicxml --lyrics "I can sing" --pattern arpeggio
```

Supported accompaniment patterns are `block`, `arpeggio`, `broken`, and `pad`.

## Core Pipeline

The implemented non-UI core follows the docs pipeline:

```text
mono float32 PCM
  -> realtime pitch detection
  -> note segmentation
  -> rhythm quantization + note dynamics
  -> key detection + chord progression candidates
  -> simple accompaniment patterns
  -> MIDI / MusicXML export
```

See `docs/algorithm_flow_status.md` for a status flowchart that marks completed,
partial, and not-started algorithm stages.

## Core APIs

Pitch detection: `include/music_elf/pitch_detector.hpp`

- `PitchDetectorConfig` controls sample rate, analysis frame/hop, supported
  frequency range, YIN threshold, and silence RMS gate.
- `PitchDetector::process(...)` accepts streaming mono `float32` PCM and writes
  `PitchEstimate` frames into caller-owned output storage.
- `PitchEstimate` reports voiced/unvoiced state, frequency, nearest MIDI note,
  cents deviation, confidence, and frame timestamp.

Note segmentation: `include/music_elf/note_segmenter.hpp`

- `NoteSegmenterConfig` controls minimum confidence, minimum note duration,
  tolerated unvoiced gaps, and pitch-change tolerance.
- `NoteSegmenter::process(...)` accepts streaming `PitchEstimate` frames and
  writes quantized `NoteEvent` values into caller-owned output storage.
- `NoteSegmenter::flush(...)` emits the final active note at the end of a stream.

Rhythm and dynamics:

- `include/music_elf/rhythm_analyzer.hpp` estimates BPM/beat grid from note
  onsets and quantizes note starts/durations.
- `include/music_elf/dynamics_analyzer.hpp` computes note-level RMS, peak,
  MIDI velocity, and dynamic mark from the original PCM.

Harmony and arrangement:

- `include/music_elf/harmony_analyzer.hpp` detects key and generates multiple
  style-based chord progression candidates.
- `include/music_elf/accompaniment_generator.hpp` generates block chord,
  arpeggio, broken chord, and pad accompaniment notes.

Lyrics and export:

- `include/music_elf/audio_io.hpp` reads PCM/float WAV files, writes PCM16 WAV,
  and downmixes to mono.
- `include/music_elf/core_pipeline.hpp` runs the end-to-end non-UI pipeline.
- `include/music_elf/lyric_aligner.hpp` aligns known lyric tokens to extracted
  notes deterministically. It is not ASR.
- `include/music_elf/midi_writer.hpp` exports Standard MIDI files in memory.
- `include/music_elf/musicxml_writer.hpp` exports simple MusicXML strings.
- `include/music_elf/c_api.h` exposes a small C ABI for app embedding.
- `include/music_elf/model_interfaces.hpp` defines explicit interfaces for
  model-backed features that are not part of the deterministic MVP core.

## Current Limits

The current implementation targets clean monophonic voice or single-melody
input. It does not perform source separation, unknown-lyrics ASR, neural
audio-to-MIDI, polyphonic pitch detection, or full orchestration. Those pieces
should be added later through explicit model-backed modules.
