---
name: music-elf-ui-app
description: Build or review the Music Elf Flutter desktop/mobile app and native app binding layer. Use when working on GUI screens, cross-platform app architecture, FFI contracts, pipeline job state, progress events, cancellation, result accessors, preview playback, imports/exports, or desktop/mobile UX around the native C++ core.
---

# Music Elf UI App

## Overview

Use Flutter for future desktop and mobile app work. Keep the GUI behind a
stable native facade instead of calling the CLI or parsing human-readable
summaries.

## App Defaults

- Place app code under `apps/music_elf_app/`.
- Target Windows, macOS, iOS, and Android from one Flutter codebase.
- Use FFI to call a native app binding facade around the C++ core.
- Keep long-running analysis as a cancellable job with progress events.
- Use SQLite/Drift for app-owned presets, catalog metadata, provider settings,
  and analysis history.

## Required Native Contract

- `PipelineJob` creation from WAV path or audio buffer.
- Machine-readable events such as `AudioLoaded`, `PitchFramesReady`,
  `NotesReady`, `RhythmReady`, `DynamicsReady`, `HarmonyReady`,
  `AccompanimentReady`, `LyricsAligned`, `ExportsReady`, `PreviewReady`,
  `Canceled`, and `Error`.
- Result handle accessors for summary, notes, chords, lyric alignment, MIDI
  bytes, MusicXML text, and preview WAV/audio buffer.
- Explicit error objects with code, message, recoverability, and stage.

## Workflow

1. Define the product flow first: import or record, analyze, inspect, edit,
   preview, export.
2. Model app state with a reducer or equivalent deterministic state machine.
3. Keep UI rendering separate from native event translation.
4. Test desktop and mobile layouts for the same workflow.
5. Verify that no feature depends on CLI stdout.

## Acceptance

- Flutter unit tests cover state transitions and event handling.
- Widget tests cover desktop and mobile breakpoints.
- Integration tests cover importing a synthetic WAV and reaching MIDI/MusicXML
  output through app-facing APIs.
