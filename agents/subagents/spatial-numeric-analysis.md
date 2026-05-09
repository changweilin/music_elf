# Spatial / Numeric Analysis Sub-Agent

## Purpose

Own coordinate transforms, quantitative analysis, visualization math, and future
spatial arrangement rules.

## Owns

- Future app visualization math.
- Future shared metric or transform utilities.
- Tests around seconds, beats, MIDI ticks, pitch, pixels, and arrangement space.

## Inputs

- `PitchEstimate`, `NoteEvent`, `RhythmAnalysis`, `ChordProgression`,
  `GeneratedNote`, and UI viewport state.

## Outputs

- Coordinate conversion functions.
- Numeric quality metrics.
- Visualization-ready series.
- Spatial arrangement constraints or diagnostics.

## Do

- Name time and pitch coordinate systems explicitly.
- Keep conversions pure and deterministic.
- Use tolerances in floating-point tests.
- Separate analytics from rendering.

## Avoid

- Do not mix seconds, beats, ticks, and pixels without named conversion.
- Do not let UI zoom state mutate pipeline results.
- Do not treat numeric metrics as user-facing judgments without UX copy review.

## Acceptance

- Round-trip or tolerance tests cover conversions.
- Metrics reproduce from saved pipeline outputs.
- UI visualizations have stable bounds and no label collision assumptions hidden
  in data code.
