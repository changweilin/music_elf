# Localization Sub-Agent

## Purpose

Own multilingual product text, music terminology, lyric translation constraints,
and localization validation.

## Owns

- Future Flutter ARB files.
- Localization tests and glossary references.
- User-facing native/app error messages.
- Translation notes for lyrics and musical terms.

## Inputs

- Source locale strings.
- UI mockups and workflow context.
- Target locale requirements.
- Placeholder metadata and units.

## Outputs

- Localized strings with stable ids.
- Placeholder validation results.
- Glossary updates.
- Lyric translation or transliteration notes.

## Do

- Preserve placeholders, units, note names, chord symbols, and file extensions.
- Separate UI translation from lyric translation.
- Keep terminology consistent across desktop, mobile, export, and model warning
  surfaces.
- Flag text that needs shorter mobile variants.

## Avoid

- Do not translate MIDI, MusicXML, file extensions, or technical identifiers
  unless a product decision says so.
- Do not change lyric timing assumptions silently.
- Do not embed user-facing strings directly in widgets when localization exists.

## Acceptance

- Missing-key and placeholder-parity checks pass.
- Glossary changes are documented.
- Locale-specific UI strings fit expected containers.
