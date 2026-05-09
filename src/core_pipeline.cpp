#include "music_elf/core_pipeline.hpp"

#include <algorithm>
#include <stdexcept>

namespace music_elf {
namespace {

std::vector<PitchEstimate> detect_pitch_frames(
    const std::vector<float>& mono,
    const PitchDetectorConfig& config,
    std::size_t chunk_size) {
    if (chunk_size == 0) {
        throw std::invalid_argument("processing_chunk_samples must be positive");
    }

    PitchDetector detector(config);
    std::vector<PitchEstimate> estimates;
    std::vector<PitchEstimate> scratch(4096);
    for (std::size_t offset = 0; offset < mono.size(); offset += chunk_size) {
        const std::size_t count = std::min(chunk_size, mono.size() - offset);
        const std::size_t written = detector.process(
            mono.data() + offset,
            count,
            scratch.data(),
            scratch.size());
        estimates.insert(estimates.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(written));
    }
    return estimates;
}

std::vector<NoteEvent> segment_notes(
    const std::vector<PitchEstimate>& estimates,
    const NoteSegmenterConfig& config) {
    NoteSegmenter segmenter(config);
    std::vector<NoteEvent> notes;
    std::vector<NoteEvent> scratch(4096);

    const std::size_t written = segmenter.process(estimates.data(), estimates.size(), scratch.data(), scratch.size());
    notes.insert(notes.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(written));
    const std::size_t flushed = segmenter.flush(scratch.data(), scratch.size());
    notes.insert(notes.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(flushed));
    return notes;
}

std::vector<GeneratedNote> make_midi_notes(
    const std::vector<NoteEvent>& melody,
    const std::vector<NoteDynamics>& dynamics,
    const std::vector<GeneratedNote>& accompaniment,
    const MidiWriterConfig& config) {
    std::vector<GeneratedNote> notes;
    notes.reserve(melody.size() + accompaniment.size());
    for (std::size_t i = 0; i < melody.size(); ++i) {
        GeneratedNote note;
        note.start_seconds = melody[i].start_seconds;
        note.end_seconds = melody[i].end_seconds;
        note.midi_note = melody[i].midi_note;
        note.velocity = i < dynamics.size() ? dynamics[i].velocity : config.melody_velocity;
        note.channel = config.melody_channel;
        notes.push_back(note);
    }
    notes.insert(notes.end(), accompaniment.begin(), accompaniment.end());
    return notes;
}

}  // namespace

CorePipelineResult run_core_pipeline(
    const AudioBuffer& audio,
    const std::vector<LyricToken>& lyric_tokens,
    const CorePipelineConfig& config) {
    if (audio.sample_rate <= 0) {
        throw std::invalid_argument("audio sample_rate must be positive");
    }

    CorePipelineConfig local_config = config;
    local_config.pitch.sample_rate = audio.sample_rate;
    local_config.dynamics.sample_rate = audio.sample_rate;

    CorePipelineResult result;
    const std::vector<float> mono = downmix_to_mono(audio);
    result.pitch_estimates = detect_pitch_frames(
        mono,
        local_config.pitch,
        local_config.processing_chunk_samples);
    result.notes = segment_notes(result.pitch_estimates, local_config.note_segmenter);

    RhythmAnalyzer rhythm_analyzer(local_config.rhythm);
    result.rhythm = rhythm_analyzer.analyze(result.notes.data(), result.notes.size());

    DynamicsAnalyzer dynamics_analyzer(local_config.dynamics);
    result.dynamics = dynamics_analyzer.analyze(mono.data(), mono.size(), result.notes.data(), result.notes.size());

    HarmonyAnalyzer harmony_analyzer(local_config.harmony);
    result.key = harmony_analyzer.detect_key(result.notes.data(), result.notes.size());
    result.chord_progressions = harmony_analyzer.generate_chord_progressions(result.notes.data(), result.notes.size());

    if (!result.chord_progressions.empty()) {
        local_config.accompaniment.bpm = result.rhythm.beat_grid.bpm;
        AccompanimentGenerator accompaniment_generator(local_config.accompaniment);
        const auto& chords = result.chord_progressions.front().chords;
        result.accompaniment_notes = accompaniment_generator.generate(chords.data(), chords.size());
    }

    if (!lyric_tokens.empty()) {
        LyricAligner lyric_aligner;
        result.lyric_alignments =
            lyric_aligner.align(lyric_tokens.data(), lyric_tokens.size(), result.notes.data(), result.notes.size());
    }

    local_config.midi.bpm = result.rhythm.beat_grid.bpm;
    const std::vector<GeneratedNote> midi_notes =
        make_midi_notes(result.notes, result.dynamics, result.accompaniment_notes, local_config.midi);
    result.midi_bytes = write_midi(midi_notes.data(), midi_notes.size(), local_config.midi);

    local_config.musicxml.bpm = result.rhythm.beat_grid.bpm;
    result.musicxml = write_musicxml(
        result.notes.data(),
        result.notes.size(),
        result.lyric_alignments.data(),
        result.lyric_alignments.size(),
        local_config.musicxml);

    return result;
}

}  // namespace music_elf

