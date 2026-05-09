# Algorithm Flow Status

Legend:

- Completed: implemented in the native C++ core and covered by tests.
- Partial: implemented as a deterministic MVP, but not production-complete for all musical cases.
- Not started: requires additional model integration, datasets, or a later product decision.

```mermaid
flowchart TD
    A["WAV input / Audio input"] --> A1["WAV read + mono downmix"]
    A1 --> B{"Input type"}

    B -->|Clean monophonic vocal / melody| C["Mono float32 PCM stream"]
    B -->|Full mixed song| U1["Source separation: vocal / drums / bass / other"]
    B -->|Unknown lyrics| U2["Singing ASR / ALT"]

    C --> D["Realtime pitch detection<br/>YIN-style F0 tracking"]
    D --> E["Note segmentation<br/>smoothing, gap handling, MIDI note quantization"]
    E --> F["Rhythm analysis<br/>BPM estimate, beat grid, note quantization"]
    C --> G["Dynamics analysis<br/>RMS, peak, velocity, dynamic mark"]

    E --> H["Known lyrics alignment<br/>token-to-note timing"]
    U2 --> H2["Unknown lyric transcription and confidence"]

    E --> I["Key detection<br/>pitch-class profile scoring"]
    I --> J["Chord progression candidates<br/>Pop, Ballad, Jazz, Cinematic, Classical"]
    U1 --> U3["Chord recognition from full mixed audio<br/>chroma / CQT / bass inference"]

    J --> K["Simple accompaniment generation<br/>block, arpeggio, bass+chord, range limits, voice leading"]
    K --> L["MIDI export<br/>Standard MIDI file bytes"]
    K --> X4["Built-in audio preview render<br/>simple oscillator WAV"]
    E --> L
    E --> M["MusicXML export<br/>quantized single-part editable score"]
    H --> M
    L --> X1["CLI demo<br/>WAV to MIDI / MusicXML"]
    M --> X1
    X1 --> X2["C ABI wrapper<br/>app embedding"]
    L --> X3["GM MIDI catalog generator<br/>instrument / root / chord combinations"]

    J --> P1["Orchestration rules<br/>instrument ranges, voice leading, section writing"]
    P1 --> U4["Full orchestra draft generation<br/>multi-track arrangement"]
    U4 --> U5["Commercial-quality automatic orchestration"]

    classDef done fill:#d8f5df,stroke:#278a43,color:#123b20;
    classDef partial fill:#fff3bf,stroke:#b08900,color:#4a3b00;
    classDef todo fill:#ffe3e3,stroke:#c92a2a,color:#5c1010;

    class A1,C,D,E,F,G,H,I,J,K,L,M,X1,X2,X3,X4 done;
    class U1,U2,H2,U3,P1,U4,U5 todo;
```

## Completed Native Core

- Realtime monophonic pitch detection: `include/music_elf/pitch_detector.hpp`
- Note segmentation: `include/music_elf/note_segmenter.hpp`
- Rhythm quantization: `include/music_elf/rhythm_analyzer.hpp`
- Note dynamics: `include/music_elf/dynamics_analyzer.hpp`
- Known lyrics alignment: `include/music_elf/lyric_aligner.hpp`
- Key detection and chord candidates: `include/music_elf/harmony_analyzer.hpp`
- Simple accompaniment generation: `include/music_elf/accompaniment_generator.hpp`
- WAV I/O and mono downmix: `include/music_elf/audio_io.hpp`
- Built-in note audio preview renderer: `include/music_elf/audio_renderer.hpp`
- End-to-end pipeline runner: `include/music_elf/core_pipeline.hpp`
- MIDI export: `include/music_elf/midi_writer.hpp`
- General MIDI catalog generator: `include/music_elf/midi_catalog.hpp`
- Quantized single-part MusicXML export with measures, rests, ties, lyrics, key, time, and clef: `include/music_elf/musicxml_writer.hpp`
- CLI demo, inspect, benchmark, catalog, and render-preview helpers: `tools/music_elf_cli.cpp`
- C ABI wrapper with pitch, pipeline summary, MIDI export, and MusicXML export helpers: `include/music_elf/c_api.h`
- Model-backed feature interfaces: `include/music_elf/model_interfaces.hpp`
- Model integration data contracts: `docs/model_integration_schemas.md`

## Not Completed Yet

- Source separation for full songs.
- Unknown lyric transcription / singing ASR.
- Full-song chord recognition from mixed audio.
- Neural audio-to-MIDI models such as Basic Pitch / CREPE-style model inference.
- Full orchestration rules and multi-track orchestra generation.
- Commercial-quality automatic orchestration.
