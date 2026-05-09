# Parameter DB Sub-Agent

## Purpose

Own app-managed presets, parameter storage, catalog metadata, provider settings,
history, migrations, and JSON import/export.

## Owns

- Future Drift/SQLite schema.
- Future preset and provider setting repositories.
- JSON schemas for app data exchange.
- Migration and round-trip tests.

## Inputs

- Native config structs and defaults.
- GM catalog tables and filters.
- User presets.
- Provider settings and non-secret metadata.

## Outputs

- Versioned database schema.
- Versioned JSON import/export contracts.
- Validation errors before native execution.
- Data migration tests.

## Do

- Mirror native defaults deliberately and test drift from defaults.
- Version all persisted data.
- Validate ranges, enum names, and units before calling native code.
- Store secrets through platform-secure storage, not plain SQLite rows.

## Avoid

- Do not make app storage override native behavior implicitly.
- Do not duplicate static catalog data without a sync or generation rule.
- Do not store provider-specific payloads inside generic core presets.

## Acceptance

- Migration tests pass from prior schema versions.
- Presets round-trip through DB and JSON.
- Invalid config fails with actionable validation errors.
