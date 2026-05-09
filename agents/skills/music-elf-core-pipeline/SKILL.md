---
name: music-elf-core-pipeline
description: Work on the Music Elf deterministic C++ audio/music pipeline. Use when changing or reviewing pitch detection, note segmentation, rhythm quantization, dynamics, harmony, accompaniment, MIDI/MusicXML export, audio preview rendering, core pipeline orchestration, CTest fixtures, or CLI/C API behavior that depends on the native core.
---

# Music Elf Core Pipeline

## Overview

Use this skill to make focused changes to the native C++ core while preserving
the current MVP boundary: clean monophonic vocal or single-melody input, local
deterministic processing, and stable MIDI/MusicXML output.

## Required Context

- Read `README.md` and `docs/algorithm_flow_status.md` for scope.
- Read `include/music_elf/core_pipeline.hpp` before changing pipeline shape.
- Read the specific public header and matching `src/` file for the module being
  changed.
- Check matching tests under `tests/` before editing behavior.

## Workflow

1. Identify the pipeline stage and data type being changed:
   `AudioBuffer`, `PitchEstimate`, `NoteEvent`, `RhythmAnalysis`,
   `NoteDynamics`, `ChordProgression`, `GeneratedNote`, MIDI bytes, or
   MusicXML text.
2. Keep time bases explicit. Do not mix seconds, beats, quantized seconds, or
   MIDI ticks without a named conversion.
3. Preserve the public API unless the task asks for an interface change.
4. Add behavior tests near the module and update integration tests when the
   pipeline result changes.
5. Run CTest after implementation.

## Guardrails

- Do not add source separation, unknown-lyrics ASR, polyphonic pitch detection,
  or commercial orchestration directly to the deterministic core.
- Do not make GUI code depend on parsing CLI summaries.
- Do not hide model confidence, provenance, or source metadata inside existing
  types that cannot represent it.
- Keep MIDI channels internally zero-based and document any app-facing display
  conversion.

## Acceptance

- Relevant CTest targets pass.
- Public headers and docs agree.
- Synthetic fixtures remain deterministic.
- Export changes include MIDI or MusicXML byte/text assertions when practical.
