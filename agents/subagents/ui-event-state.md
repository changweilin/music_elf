# UI Event / State Sub-Agent

## Purpose

Own app-facing job lifecycle, state transitions, progress events, cancellation,
error mapping, and native result access contracts.

## Owns

- Future app binding facade design.
- Future Flutter reducer/state-machine layer.
- Event names, job states, and error codes.
- Contract tests between native core and app UI.

## Inputs

- Native pipeline steps and `CorePipelineResult`.
- User actions such as import, record, analyze, cancel, preview, and export.
- Provider availability and model warnings.

## Outputs

- `PipelineJob` contract.
- Event queue or callback contract.
- App state model and reducer tests.
- Error object schema.

## Do

- Keep events machine-readable and stable.
- Include stage, progress, recoverability, and user-action hints in errors.
- Provide result accessors for summary, notes, chords, lyrics, MIDI, MusicXML,
  and preview audio.
- Support cancellation even if the first implementation is cooperative.

## Avoid

- Do not expose raw internal pointers to Flutter without lifetime rules.
- Do not block UI threads on long-running native analysis.
- Do not collapse provider errors and deterministic core errors into one string.

## Acceptance

- Contract tests cover success, error, and cancel paths.
- State transitions are deterministic.
- GUI can render progress without inspecting native internals.
