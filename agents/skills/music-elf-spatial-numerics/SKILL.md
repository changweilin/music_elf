---
name: music-elf-spatial-numerics
description: Analyze Music Elf timing, pitch, beat-grid, score-space, visualization, numeric stability, and future spatial arrangement behavior. Use when working on coordinate transforms, note timelines, piano-roll or score views, beat/seconds/MIDI tick conversions, visual analytics, arrangement range rules, spatial audio planning, or quantitative quality metrics.
---

# Music Elf Spatial Numerics

## Overview

Use this skill for numeric analysis that connects audio time, musical time,
pitch space, UI visualization, and future spatial arrangement rules.

## Coordinate Systems

- Audio seconds: native pipeline time base for samples, notes, lyrics, and
  model schemas.
- Beat space: rhythm analysis and quantized note starts/durations.
- MIDI ticks: export serialization space.
- Pitch space: MIDI note, frequency Hz, cents, pitch class, and score spelling.
- UI space: pixels, lanes, measures, scroll positions, and zoom transforms.
- Arrangement space: range limits, register, instrument part, channel, and
  future pan/depth placement.

## Workflow

1. Name every coordinate system at function boundaries.
2. Keep conversions pure and covered by round-trip or tolerance tests.
3. Use stable tolerances for floating-point comparisons.
4. Separate analysis metrics from UI drawing code.
5. Document any lossy conversion, such as quantizing seconds to beats or ticks.

## Quality Metrics

- Pitch: voiced frame ratio, cents deviation, confidence distribution.
- Rhythm: quantization error, tempo stability, note duration distribution.
- Harmony: chord coverage, progression score, key confidence.
- Arrangement: range violations, overlap density, voice-leading movement.
- UI: viewport coverage, label collisions, zoom precision, scroll stability.

## Acceptance

- Numeric tests specify tolerance.
- Visual transforms are deterministic for fixed inputs.
- Metrics are reproducible from saved pipeline results.
