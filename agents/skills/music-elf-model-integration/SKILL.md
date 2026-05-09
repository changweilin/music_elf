---
name: music-elf-model-integration
description: Integrate model-backed Music Elf features with the deterministic native core. Use when designing or implementing source separation, singing ASR, neural audio-to-MIDI, polyphonic pitch detection, orchestration providers, model response schemas, provider registries, validation rules, confidence/provenance handling, or hybrid local/cloud execution.
---

# Music Elf Model Integration

## Overview

Use this skill to attach AI/ML providers without weakening the deterministic
core boundary or losing confidence, provenance, and validation metadata.

## Required Context

- Read `docs/model_integration_schemas.md`.
- Read `include/music_elf/model_interfaces.hpp`.
- Read `tests/model_interfaces_tests.cpp`.
- Read any app binding contract before adding provider-facing APIs.

## Provider Boundaries

- Source separation runs before the monophonic core and returns stems.
- Singing ASR returns time-aligned lyric tokens against the same audio timeline.
- Neural audio-to-MIDI returns note events that can replace or augment native
  segmentation.
- Orchestration reads melody, chords, key, style, and optional context, then
  returns named parts.
- Polyphonic pitch detection must not be represented as plain `NoteEvent`
  without preserving source and confidence metadata.

## Workflow

1. Start with a schema and validator before calling a real provider.
2. Include provider name, model id, created time, audio metadata, warnings,
   parameters, and input checksum in model results when available.
3. Map provider errors to recoverable app errors.
4. Keep deterministic fallback behavior explicit when a provider is unavailable.
5. Add contract tests with fixed sample responses before live integration.

## Acceptance

- Schema validator tests cover valid, invalid, and missing-field responses.
- Unsupported providers fail gracefully and keep local core usable.
- Provider outputs preserve confidence and provenance through app-facing APIs.
