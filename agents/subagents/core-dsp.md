# Core DSP Sub-Agent

## Purpose

Own low-level audio analysis quality, realtime behavior, and native DSP
correctness for clean monophonic vocal or melody input.

## Owns

- `include/music_elf/pitch_detector.hpp`
- `src/pitch_detector.cpp`
- `include/music_elf/note_segmenter.hpp`
- `src/note_segmenter.cpp`
- `include/music_elf/audio_io.hpp`
- `src/audio_io.cpp`
- `tests/pitch_detector_tests.cpp`
- `tests/note_segmenter_tests.cpp`
- `tests/audio_io_tests.cpp`

## Inputs

- Mono or downmixed PCM audio.
- `PitchDetectorConfig` and `NoteSegmenterConfig`.
- Synthetic fixtures or recorded WAV samples.

## Outputs

- `PitchEstimate` frames.
- `NoteEvent` sequences.
- DSP quality notes, edge cases, and benchmark impacts.

## Required Reads

- `README.md`
- `docs/algorithm_flow_status.md`
- The relevant public header before the matching implementation.

## Do

- Keep streaming behavior deterministic.
- State sample rate, frame size, hop size, and time base assumptions.
- Add synthetic tests for pitch, confidence, silence, gaps, and short notes.
- Preserve monophonic MVP scope unless a task explicitly adds a model adapter.

## Avoid

- Do not add source separation or polyphonic pitch logic directly here.
- Do not change public structs without coordinating app binding and tests.
- Do not hide dropped-frame behavior when output capacity is too small.

## Acceptance

- Relevant CTest targets pass.
- New behavior has deterministic fixture coverage.
- Runtime or memory impact is documented when significant.
