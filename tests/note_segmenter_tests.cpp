#include "music_elf/note_segmenter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::NoteEvent;
using music_elf::NoteSegmenter;
using music_elf::NoteSegmenterConfig;
using music_elf::PitchEstimate;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

double midi_note_to_frequency(int midi_note, double cents = 0.0) {
    return 440.0 * std::pow(2.0, (static_cast<double>(midi_note) - 69.0 + cents / 100.0) / 12.0);
}

PitchEstimate make_voiced(double time_seconds, int midi_note, double cents = 0.0, float confidence = 0.95f) {
    PitchEstimate estimate;
    estimate.time_seconds = time_seconds;
    estimate.frequency_hz = static_cast<float>(midi_note_to_frequency(midi_note, cents));
    estimate.midi_note = midi_note;
    estimate.cents = static_cast<float>(cents);
    estimate.confidence = confidence;
    estimate.voiced = true;
    return estimate;
}

PitchEstimate make_unvoiced(double time_seconds) {
    PitchEstimate estimate;
    estimate.time_seconds = time_seconds;
    estimate.confidence = 0.0f;
    estimate.voiced = false;
    return estimate;
}

void add_voiced_range(
    std::vector<PitchEstimate>& estimates,
    double start_seconds,
    double end_seconds,
    double step_seconds,
    int midi_note,
    double cents = 0.0) {
    for (double time = start_seconds; time < end_seconds - 0.0000001; time += step_seconds) {
        estimates.push_back(make_voiced(time, midi_note, cents));
    }
}

void add_unvoiced_range(
    std::vector<PitchEstimate>& estimates,
    double start_seconds,
    double end_seconds,
    double step_seconds) {
    for (double time = start_seconds; time < end_seconds - 0.0000001; time += step_seconds) {
        estimates.push_back(make_unvoiced(time));
    }
}

std::vector<NoteEvent> run_segmenter(
    const std::vector<PitchEstimate>& estimates,
    const NoteSegmenterConfig& config,
    std::size_t chunk_size) {
    NoteSegmenter segmenter(config);
    std::vector<NoteEvent> notes;
    std::vector<NoteEvent> scratch(64);

    for (std::size_t offset = 0; offset < estimates.size(); offset += chunk_size) {
        const std::size_t count = std::min(chunk_size, estimates.size() - offset);
        const std::size_t written = segmenter.process(
            estimates.data() + offset,
            count,
            scratch.data(),
            scratch.size());
        notes.insert(notes.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(written));
    }

    const std::size_t flushed = segmenter.flush(scratch.data(), scratch.size());
    notes.insert(notes.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(flushed));
    return notes;
}

void require_near(double actual, double expected, double tolerance, const std::string& label) {
    const double error = std::fabs(actual - expected);
    require(error <= tolerance, label + " expected " + std::to_string(expected) + " got " + std::to_string(actual));
}

void test_stable_note() {
    NoteSegmenterConfig config;
    std::vector<PitchEstimate> estimates;
    add_voiced_range(estimates, 0.0, 0.50, 0.01, 69);

    const auto notes = run_segmenter(estimates, config, estimates.size());
    require(notes.size() == 1, "stable note should produce one note");
    require(notes[0].midi_note == 69, "stable note MIDI should be A4");
    require_near(notes[0].start_seconds, 0.0, 0.0001, "stable note start");
    require_near(notes[0].end_seconds, 0.50, 0.0001, "stable note end");
    require_near(notes[0].average_frequency_hz, 440.0, 0.1, "stable note frequency");
}

void test_rests_split_notes() {
    NoteSegmenterConfig config;
    config.max_unvoiced_gap_seconds = 0.03;

    std::vector<PitchEstimate> estimates;
    add_voiced_range(estimates, 0.00, 0.30, 0.01, 69);
    add_unvoiced_range(estimates, 0.30, 0.42, 0.01);
    add_voiced_range(estimates, 0.42, 0.70, 0.01, 72);

    const auto notes = run_segmenter(estimates, config, 5);
    require(notes.size() == 2, "rest should split two notes");
    require(notes[0].midi_note == 69, "first note should be A4");
    require(notes[1].midi_note == 72, "second note should be C5");
    require_near(notes[0].end_seconds, 0.30, 0.0001, "first note end");
    require_near(notes[1].start_seconds, 0.42, 0.0001, "second note start");
}

void test_vibrato_stays_single_note() {
    NoteSegmenterConfig config;
    config.pitch_change_tolerance_cents = 50.0;

    std::vector<PitchEstimate> estimates;
    for (int i = 0; i < 100; ++i) {
        const double cents = 30.0 * std::sin(static_cast<double>(i) * 0.35);
        estimates.push_back(make_voiced(static_cast<double>(i) * 0.01, 69, cents));
    }

    const auto notes = run_segmenter(estimates, config, 7);
    require(notes.size() == 1, "vibrato should stay one note");
    require(notes[0].midi_note == 69, "vibrato note should stay A4");
    require(std::fabs(notes[0].average_cents) < 5.0f, "vibrato average cents should stay centered");
}

void test_pitch_change_without_rest() {
    NoteSegmenterConfig config;
    std::vector<PitchEstimate> estimates;
    add_voiced_range(estimates, 0.00, 0.20, 0.01, 69);
    add_voiced_range(estimates, 0.20, 0.45, 0.01, 71);

    const auto notes = run_segmenter(estimates, config, 100);
    require(notes.size() == 2, "pitch change should split notes");
    require(notes[0].midi_note == 69, "first legato note should be A4");
    require(notes[1].midi_note == 71, "second legato note should be B4");
    require_near(notes[0].end_seconds, 0.20, 0.0001, "legato split point");
    require_near(notes[1].start_seconds, 0.20, 0.0001, "legato second start");
}

void test_short_blips_are_filtered() {
    NoteSegmenterConfig config;
    config.min_note_duration_seconds = 0.05;
    config.max_unvoiced_gap_seconds = 0.02;

    std::vector<PitchEstimate> estimates;
    add_voiced_range(estimates, 0.00, 0.02, 0.01, 69);
    add_unvoiced_range(estimates, 0.02, 0.10, 0.01);
    add_voiced_range(estimates, 0.10, 0.25, 0.01, 72);

    const auto notes = run_segmenter(estimates, config, 3);
    require(notes.size() == 1, "short blip should be filtered");
    require(notes[0].midi_note == 72, "remaining note should be C5");
}

void test_streaming_consistency() {
    NoteSegmenterConfig config;
    config.max_unvoiced_gap_seconds = 0.03;

    std::vector<PitchEstimate> estimates;
    add_voiced_range(estimates, 0.00, 0.20, 0.01, 69);
    add_unvoiced_range(estimates, 0.20, 0.25, 0.01);
    add_voiced_range(estimates, 0.25, 0.40, 0.01, 69);
    add_voiced_range(estimates, 0.40, 0.60, 0.01, 72);

    const auto full = run_segmenter(estimates, config, estimates.size());
    for (std::size_t chunk : {std::size_t{1}, std::size_t{3}, std::size_t{11}}) {
        const auto streamed = run_segmenter(estimates, config, chunk);
        require(streamed.size() == full.size(), "chunking changed note count");
        for (std::size_t i = 0; i < full.size(); ++i) {
            require(streamed[i].midi_note == full[i].midi_note, "chunking changed note MIDI");
            require_near(streamed[i].start_seconds, full[i].start_seconds, 0.0001, "chunk start");
            require_near(streamed[i].end_seconds, full[i].end_seconds, 0.0001, "chunk end");
        }
    }
}

}  // namespace

int main() {
    try {
        test_stable_note();
        test_rests_split_notes();
        test_vibrato_stays_single_note();
        test_pitch_change_without_rest();
        test_short_blips_are_filtered();
        test_streaming_consistency();
    } catch (const std::exception& error) {
        std::cerr << "note_segmenter_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "note_segmenter_tests passed\n";
    return EXIT_SUCCESS;
}

