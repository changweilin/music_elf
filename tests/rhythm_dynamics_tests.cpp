#include "music_elf/dynamics_analyzer.hpp"
#include "music_elf/rhythm_analyzer.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::DynamicsAnalyzer;
using music_elf::DynamicsAnalyzerConfig;
using music_elf::NoteEvent;
using music_elf::RhythmAnalyzer;
using music_elf::RhythmAnalyzerConfig;

constexpr double kPi = 3.1415926535897932384626433832795;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_near(double actual, double expected, double tolerance, const std::string& label) {
    const double error = std::fabs(actual - expected);
    require(error <= tolerance, label + " expected " + std::to_string(expected) + " got " + std::to_string(actual));
}

NoteEvent make_note(double start, double end, int midi_note) {
    NoteEvent note;
    note.start_seconds = start;
    note.end_seconds = end;
    note.duration_seconds = end - start;
    note.midi_note = midi_note;
    note.average_frequency_hz = 440.0f;
    note.average_confidence = 0.95f;
    return note;
}

void test_rhythm_estimates_tempo_and_quantizes() {
    const std::vector<NoteEvent> notes = {
        make_note(0.00, 0.48, 60),
        make_note(0.51, 0.98, 62),
        make_note(1.02, 1.48, 64),
        make_note(1.49, 2.02, 65),
    };

    RhythmAnalyzerConfig config;
    config.subdivisions_per_beat = 4;
    RhythmAnalyzer analyzer(config);
    const auto analysis = analyzer.analyze(notes.data(), notes.size());

    require_near(analysis.beat_grid.bpm, 120.0, 5.0, "estimated BPM");
    require(analysis.beat_grid.beats_per_bar == 4, "beats per bar");
    require(analysis.quantized_notes.size() == notes.size(), "quantized note count");
    require_near(analysis.quantized_notes[1].start_beats, 1.0, 0.0001, "second note beat");
    require_near(analysis.quantized_notes[2].start_beats, 2.0, 0.0001, "third note beat");
}

void test_rhythm_normalizes_fast_subdivisions() {
    std::vector<NoteEvent> notes;
    for (int i = 0; i < 8; ++i) {
        notes.push_back(make_note(static_cast<double>(i) * 0.25, static_cast<double>(i) * 0.25 + 0.20, 60 + i));
    }

    RhythmAnalyzer analyzer;
    const auto analysis = analyzer.analyze(notes.data(), notes.size());
    require_near(analysis.beat_grid.bpm, 120.0, 0.01, "fast subdivision BPM");
    require_near(analysis.quantized_notes[3].start_beats, 1.5, 0.0001, "eighth-note grid");
}

std::vector<float> make_segmented_sine(int sample_rate) {
    std::vector<float> samples(static_cast<std::size_t>(sample_rate));
    const double frequency = 440.0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(sample_rate);
        const double amplitude = t < 0.5 ? 0.20 : 0.80;
        samples[i] = static_cast<float>(amplitude * std::sin(2.0 * kPi * frequency * t));
    }
    return samples;
}

void test_dynamics_extracts_note_levels() {
    const int sample_rate = 48000;
    const auto samples = make_segmented_sine(sample_rate);
    const std::vector<NoteEvent> notes = {
        make_note(0.00, 0.50, 69),
        make_note(0.50, 1.00, 69),
    };

    DynamicsAnalyzerConfig config;
    config.sample_rate = sample_rate;
    DynamicsAnalyzer analyzer(config);
    const auto dynamics = analyzer.analyze(samples.data(), samples.size(), notes.data(), notes.size());

    require(dynamics.size() == 2, "dynamics count");
    require(dynamics[1].rms_db > dynamics[0].rms_db + 10.0f, "loud note should have higher RMS");
    require(dynamics[1].peak_db > dynamics[0].peak_db + 10.0f, "loud note should have higher peak");
    require(dynamics[1].velocity > dynamics[0].velocity, "loud note should have higher velocity");
}

}  // namespace

int main() {
    try {
        test_rhythm_estimates_tempo_and_quantizes();
        test_rhythm_normalizes_fast_subdivisions();
        test_dynamics_extracts_note_levels();
    } catch (const std::exception& error) {
        std::cerr << "rhythm_dynamics_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "rhythm_dynamics_tests passed\n";
    return EXIT_SUCCESS;
}

