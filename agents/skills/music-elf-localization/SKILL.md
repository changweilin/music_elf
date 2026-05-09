---
name: music-elf-localization
description: Localize Music Elf product text, music terminology, lyric workflows, and Flutter ARB resources. Use when adding or reviewing languages, translating UI strings, preserving placeholders, handling lyric translation/transliteration, checking terminology consistency, or validating localization coverage across desktop and mobile apps.
---

# Music Elf Localization

## Overview

Use this skill to keep UI strings, musical terms, model warnings, export labels,
and lyric-related text consistent across locales.

## Workflow

1. Identify the string surface: Flutter UI, native error, CLI/dev text, export
   metadata, lyric token, or market copy.
2. Preserve placeholders, units, note names, chord symbols, and file extensions
   exactly unless the target locale convention requires a documented change.
3. Separate UI translation from lyric translation. Lyrics may need
   transliteration, syllable preservation, or forced-alignment constraints.
4. Keep glossary choices stable for musical terms such as pitch, note, beat,
   rhythm, key, chord, dynamics, accompaniment, MIDI, and MusicXML.
5. Validate missing strings and placeholder mismatch before shipping.

## Flutter ARB Rules

- Use stable message ids that describe intent, not current English wording.
- Include metadata for placeholders and units.
- Keep desktop and mobile wording aligned unless layout constraints require a
  shorter variant.
- Add tests for missing keys and placeholder parity when localization tooling is
  present.

## Acceptance

- No missing locale keys.
- Placeholder names and counts match the source locale.
- Musical terms follow the current glossary.
- Lyric translation decisions state whether timing or semantic fidelity was
  prioritized.
