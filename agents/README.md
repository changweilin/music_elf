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

Each skill ships in two flavors so the same role can be invoked from either
Claude Code or a ChatGPT Custom GPT / OpenAI Assistant:

- `skills/<name>/SKILL.md` — Claude Code skill (frontmatter + workflow).
- `skills/<name>/agents/openai.yaml` — ChatGPT Custom GPT spec (display
  metadata, recommended model, capabilities, full instructions, guardrails,
  acceptance criteria).

Skills:

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

Each sub-agent also has paired definitions:

- `subagents/<name>.md` — Claude Code sub-agent role brief.
- `subagents/openai/<name>.yaml` — OpenAI Assistant spec (tools, owns,
  inputs, outputs, instructions, acceptance).

Sub-agents:

- `core-dsp` — `subagents/core-dsp.md`, `subagents/openai/core-dsp.yaml`
- `music-theory-arrangement` — `subagents/music-theory-arrangement.md`,
  `subagents/openai/music-theory-arrangement.yaml`
- `export-notation-qa` — `subagents/export-notation-qa.md`,
  `subagents/openai/export-notation-qa.yaml`
- `flutter-ui` — `subagents/flutter-ui.md`,
  `subagents/openai/flutter-ui.yaml`
- `ui-event-state` — `subagents/ui-event-state.md`,
  `subagents/openai/ui-event-state.yaml`
- `localization` — `subagents/localization.md`,
  `subagents/openai/localization.yaml`
- `parameter-db` — `subagents/parameter-db.md`,
  `subagents/openai/parameter-db.yaml`
- `model-adapter` — `subagents/model-adapter.md`,
  `subagents/openai/model-adapter.yaml`
- `spatial-numeric-analysis` — `subagents/spatial-numeric-analysis.md`,
  `subagents/openai/spatial-numeric-analysis.yaml`
- `market-science` — `subagents/market-science.md`,
  `subagents/openai/market-science.yaml`

## Parity between Claude and ChatGPT versions

The two flavors are kept aligned on purpose:

- Same name, same owned files, same inputs/outputs, same guardrails, same
  acceptance criteria.
- When changing a role, update both files in the same commit.
- ChatGPT YAML files include richer surface metadata (display name,
  conversation starters, capability flags) needed by Custom GPT / Assistant
  configuration; Claude markdown stays workflow-first.

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
