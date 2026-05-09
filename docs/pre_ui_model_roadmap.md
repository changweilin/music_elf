# Pre-UI / Pre-AI Model Roadmap

This roadmap tracks product-core work that can be completed before UI integration
and external AI model deployment.

## Progress Flow

```mermaid
flowchart TD
    A["Core algorithms complete"] --> B["Output quality"]
    B --> C["Playable audio preview"]
    C --> D["Arrangement rules"]
    D --> E["Fixtures and snapshot tests"]
    E --> F["CLI productization"]
    F --> G["Benchmark baselines"]
    G --> H["C ABI / app binding readiness"]
    H --> I["Model integration schemas"]
    I --> J["Ready for UI and model deployment"]

    classDef done fill:#d8f5df,stroke:#278a43,color:#123b20;
    classDef doing fill:#fff3bf,stroke:#b08900,color:#4a3b00;
    classDef todo fill:#ffe3e3,stroke:#c92a2a,color:#5c1010;

    class A,B,C,D,E,F,G,H,I,J done;
```

## Execution Plan

| Order | Work item | Status | Acceptance |
|---:|---|---|---|
| 1 | MIDI output quality: tempo, track name, time signature, key signature, program change | Done | MIDI tests verify meta events |
| 2 | Built-in lightweight audio preview renderer | Done | Generated notes can render to PCM/WAV without external synth |
| 3 | CLI command for render-demo / audio preview | Done | CLI writes a WAV preview and tests verify RIFF output |
| 4 | Arrangement rules: inversions, bass + chord patterns, range constraints | Done | Tests cover note ranges and smoother chord movement |
| 5 | Fixtures and snapshot tests | Done | Stable synthetic fixtures produce deterministic MIDI/MusicXML |
| 6 | CLI productization: inspect, benchmark, render helpers | Done | CTest covers CLI smoke and e2e workflows |
| 7 | Benchmark baselines | Done | Runtime summaries are emitted and tracked by tests/tools |
| 8 | C ABI pipeline expansion | Done | C API can run pipeline-level analysis/export |
| 9 | Model integration schemas | Done | Source separation, ASR, and neural MIDI schemas are documented |
