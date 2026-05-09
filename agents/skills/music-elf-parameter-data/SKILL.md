---
name: music-elf-parameter-data
description: Design or modify Music Elf parameter, preset, catalog, provider setting, and app data storage workflows. Use when working with CorePipelineConfig presets, MIDI catalog metadata, SQLite/Drift schemas, JSON import/export, model provider parameters, analysis history, migrations, or validation of user-editable configuration.
---

# Music Elf Parameter Data

## Overview

Use this skill when turning native config structs and catalog tables into
app-managed presets, migrations, or import/export formats.

## Data Boundaries

- Native core boundary: C++ structs such as `CorePipelineConfig`,
  `PitchDetectorConfig`, `NoteSegmenterConfig`, `RhythmAnalyzerConfig`,
  `DynamicsAnalyzerConfig`, `HarmonyAnalyzerConfig`,
  `AccompanimentGeneratorConfig`, `MidiWriterConfig`,
  `MusicXmlWriterConfig`, and `MidiCatalogConfig`.
- App storage boundary: SQLite/Drift tables for presets, catalog metadata,
  provider settings, and analysis history.
- Import/export boundary: versioned JSON with explicit schema version and
  validation errors.

## Workflow

1. Identify whether the data is native config, app-only metadata, provider
   parameters, or user history.
2. Keep defaults aligned with existing C++ config values unless a migration or
   product decision says otherwise.
3. Version every persisted schema and JSON document.
4. Validate ranges before calling native code.
5. Add migration and round-trip tests for every persisted shape.

## Guardrails

- Do not make app storage the source of truth for DSP behavior.
- Do not duplicate the GM catalog in multiple hand-maintained places without a
  generated or tested sync path.
- Do not store API secrets in plain app database rows.
- Keep provider parameters separate from deterministic core presets.

## Acceptance

- SQLite migrations are tested forward from the previous version.
- Presets round-trip through storage and JSON.
- Invalid ranges fail before native execution.
- Catalog filters cover instruments, roots, chords, duration, BPM, and octave.
