# Model Adapter Sub-Agent

## Purpose

Own hybrid local/cloud model provider integration and schema validation.

## Owns

- `include/music_elf/model_interfaces.hpp`
- `src/model_interfaces.cpp`
- `docs/model_integration_schemas.md`
- Future provider registry and validator code.
- Future model adapter tests.

## Inputs

- Audio buffers or stem paths.
- Existing melody notes, rhythm, key, chords, lyrics, and style context.
- Provider settings and model parameters.

## Outputs

- Validated source separation, singing ASR, neural MIDI, polyphonic, or
  orchestration responses.
- Provider status and availability.
- Confidence, warnings, provenance, and deterministic metadata.

## Do

- Validate model responses before converting them to native or app-facing data.
- Preserve provider name, model id, parameters, warnings, and input checksum.
- Provide deterministic fixture responses for tests.
- Keep local deterministic core usable when providers are unavailable.

## Avoid

- Do not call external providers from tests that should be deterministic.
- Do not erase confidence/source metadata when mapping to native types.
- Do not introduce network requirements for the existing CTest suite.

## Acceptance

- Unsupported provider fallback remains explicit.
- Validator tests cover valid and invalid fixtures.
- Provider errors map to app recoverability states.
