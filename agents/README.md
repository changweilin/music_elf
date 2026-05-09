# Music Elf Agent Templates

This directory defines repo-versioned skills and sub-agent role templates for
Music Elf. These files are working agreements for future Codex or human-assisted
implementation, not runtime code.

## Architecture Defaults

- Keep the C++ deterministic core as the source of truth for audio analysis,
  note extraction, rhythm, dynamics, harmony, arrangement, and export.
- Build future GUI work in `apps/music_elf_app/` with Flutter for desktop and
  mobile targets.
- Connect Flutter to native code through an app binding facade and FFI. Do not
  parse CLI output for product behavior.
- Use a hybrid model strategy: run the deterministic core locally, then attach
  source separation, singing ASR, neural MIDI, and orchestration providers
  through explicit adapters.
- Keep core config structs as stable native boundaries. App storage may use
  SQLite/Drift and JSON import/export for presets, provider settings, catalog
  metadata, and analysis history.

## Skills

- `skills/music-elf-core-pipeline`: core C++ audio/music pipeline work.
- `skills/music-elf-ui-app`: Flutter desktop/mobile app work.
- `skills/music-elf-localization`: localization, lyric translation, and ARB
  consistency work.
- `skills/music-elf-parameter-data`: presets, catalog metadata, provider
  settings, and app data storage work.
- `skills/music-elf-model-integration`: model-backed provider adapters and
  schema validation work.
- `skills/music-elf-spatial-numerics`: beat/time/pitch coordinate math,
  visual analytics, and future spatial rules.
- `skills/music-elf-market-science`: market research, segmentation, feature
  prioritization, pricing, and experiment planning.

## Sub-Agents

- `subagents/core-dsp.md`
- `subagents/music-theory-arrangement.md`
- `subagents/export-notation-qa.md`
- `subagents/flutter-ui.md`
- `subagents/ui-event-state.md`
- `subagents/localization.md`
- `subagents/parameter-db.md`
- `subagents/model-adapter.md`
- `subagents/spatial-numeric-analysis.md`
- `subagents/market-science.md`

## Shared Rules

- Read existing headers, docs, and tests before proposing changes.
- Preserve clean monophonic vocal/melody as the MVP constraint unless the task
  explicitly introduces model-backed polyphonic or source-separation behavior.
- Treat seconds, beats, quantized seconds, and MIDI ticks as distinct coordinate
  systems. Name conversions explicitly.
- Keep app-facing contracts stable and machine-readable.
- Add or update tests whenever changing behavior, interfaces, schemas, or data
  migrations.
- Market and competitive analysis must use current external sources when
  conclusions depend on present-day market facts.
