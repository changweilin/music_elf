# Model Integration Schemas

These schemas define the boundary between the deterministic native MVP core and
future model-backed modules. They are data contracts only; source separation,
singing ASR, neural audio-to-MIDI, and orchestration models are not implemented
in the native core yet.

## Shared Envelope

Every model response should include a stable envelope so UI and app bindings can
surface confidence, provenance, and recoverable failures consistently.

```json
{
  "schema_version": "music-elf.model.v1",
  "provider": "provider-name",
  "model": "model-or-checkpoint-id",
  "created_at_unix_ms": 0,
  "audio": {
    "sample_rate": 48000,
    "channels": 1,
    "duration_seconds": 0.0
  },
  "warnings": []
}
```

## Source Separation

Input: one WAV-compatible PCM stream, usually a full mixed song.

Output stems must preserve the input sample rate and duration. Missing stems are
represented by empty audio plus a low confidence score rather than omitted keys.

```json
{
  "schema_version": "music-elf.source-separation.v1",
  "stems": {
    "vocals": {"path": "vocals.wav", "confidence": 0.0},
    "drums": {"path": "drums.wav", "confidence": 0.0},
    "bass": {"path": "bass.wav", "confidence": 0.0},
    "other": {"path": "other.wav", "confidence": 0.0}
  }
}
```

The native bridge target is `music_elf::SourceSeparationModel` in
`include/music_elf/model_interfaces.hpp`.

## Singing ASR / Unknown Lyrics

Input: vocal stem or clean monophonic vocal audio.

Output words should be time-aligned in seconds against the same audio timeline
used by note segmentation.

```json
{
  "schema_version": "music-elf.singing-asr.v1",
  "language": "und",
  "tokens": [
    {
      "text": "word",
      "start_seconds": 0.0,
      "end_seconds": 0.0,
      "confidence": 0.0
    }
  ]
}
```

The native bridge target is `music_elf::LyricsTranscriptionModel`.

## Neural MIDI / Polyphonic Notes

Input: melody audio, vocal stem, instrument stem, or mixed audio depending on
provider capability.

Output notes intentionally mirror the native `NoteEvent` timing and pitch shape
so neural notes can replace or augment deterministic note segmentation.

```json
{
  "schema_version": "music-elf.neural-midi.v1",
  "notes": [
    {
      "start_seconds": 0.0,
      "end_seconds": 0.0,
      "midi_note": 60,
      "frequency_hz": 261.626,
      "velocity": 88,
      "confidence": 0.0,
      "source": "melody"
    }
  ],
  "polyphonic": false
}
```

The native bridge target is `music_elf::NeuralAudioToMidiModel`.

## Orchestration Drafts

Input: melody notes, chord progression candidates, key estimate, target style,
and optional stem or lyric context.

Output generated notes are grouped by named parts and General MIDI program so
the current MIDI writer can serialize them without requiring a new event type.

```json
{
  "schema_version": "music-elf.orchestration.v1",
  "style": "cinematic",
  "parts": [
    {
      "name": "Violins",
      "gm_program": 40,
      "channel": 0,
      "notes": [
        {
          "start_seconds": 0.0,
          "end_seconds": 1.0,
          "midi_note": 72,
          "velocity": 84
        }
      ]
    }
  ]
}
```

## Validation Rules

- `start_seconds` must be greater than or equal to `0`.
- `end_seconds` must be greater than `start_seconds`.
- `midi_note` must be in `[0, 127]`.
- `velocity` must be in `[1, 127]`.
- `confidence` values must be in `[0.0, 1.0]`.
- Model responses must be deterministic for the same model id, parameters, and
  input checksum when provider settings expose a deterministic mode.
