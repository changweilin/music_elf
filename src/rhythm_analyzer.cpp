#include "music_elf/rhythm_analyzer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

double median(std::vector<double> values) {
    if (values.empty()) {
        return 0.5;
    }
    const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
    std::nth_element(values.begin(), middle, values.end());
    return *middle;
}

double round_to_grid(double value, int subdivisions) {
    const double scale = static_cast<double>(subdivisions);
    return std::round(value * scale) / scale;
}

}  // namespace

RhythmAnalyzer::RhythmAnalyzer(const RhythmAnalyzerConfig& config) : config_(config) {
    if (config_.min_bpm <= 0.0 || config_.max_bpm <= 0.0 || config_.min_bpm >= config_.max_bpm) {
        throw std::invalid_argument("BPM range is invalid");
    }
    if (config_.beats_per_bar <= 0) {
        throw std::invalid_argument("beats_per_bar must be positive");
    }
    if (config_.subdivisions_per_beat <= 0) {
        throw std::invalid_argument("subdivisions_per_beat must be positive");
    }
}

const RhythmAnalyzerConfig& RhythmAnalyzer::config() const noexcept {
    return config_;
}

RhythmAnalysis RhythmAnalyzer::analyze(const NoteEvent* notes, std::size_t count) const {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }

    RhythmAnalysis analysis;
    const double beat_duration = estimate_beat_duration(notes, count);
    analysis.beat_grid = build_beat_grid(notes, count, beat_duration);
    analysis.quantized_notes.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        analysis.quantized_notes.push_back(quantize_note(notes[i], analysis.beat_grid));
    }
    return analysis;
}

double RhythmAnalyzer::estimate_beat_duration(const NoteEvent* notes, std::size_t count) const {
    const double min_beat = 60.0 / config_.max_bpm;
    const double max_beat = 60.0 / config_.min_bpm;

    std::vector<double> candidates;
    for (std::size_t i = 1; i < count; ++i) {
        double interval = notes[i].start_seconds - notes[i - 1].start_seconds;
        if (interval <= 0.0) {
            continue;
        }
        while (interval < min_beat) {
            interval *= 2.0;
        }
        while (interval > max_beat) {
            interval *= 0.5;
        }
        if (interval >= min_beat && interval <= max_beat) {
            candidates.push_back(interval);
        }
    }

    if (candidates.empty()) {
        return 60.0 / 120.0;
    }
    return median(candidates);
}

BeatGrid RhythmAnalyzer::build_beat_grid(const NoteEvent* notes, std::size_t count, double beat_duration) const {
    BeatGrid grid;
    grid.beats_per_bar = config_.beats_per_bar;
    grid.beat_duration_seconds = beat_duration;
    grid.bpm = 60.0 / beat_duration;
    grid.first_beat_seconds = count > 0 ? notes[0].start_seconds : 0.0;

    const double last_time = count > 0 ? notes[count - 1].end_seconds : grid.first_beat_seconds;
    const auto beat_count = static_cast<std::size_t>(
        std::max(1.0, std::ceil((last_time - grid.first_beat_seconds) / beat_duration) + 1.0));
    grid.beat_times_seconds.reserve(beat_count);
    for (std::size_t i = 0; i < beat_count; ++i) {
        grid.beat_times_seconds.push_back(grid.first_beat_seconds + static_cast<double>(i) * beat_duration);
    }
    return grid;
}

QuantizedNote RhythmAnalyzer::quantize_note(const NoteEvent& note, const BeatGrid& grid) const {
    QuantizedNote result;
    result.note = note;

    const double start_beats = (note.start_seconds - grid.first_beat_seconds) / grid.beat_duration_seconds;
    const double duration_beats = note.duration_seconds / grid.beat_duration_seconds;
    result.start_beats = round_to_grid(start_beats, config_.subdivisions_per_beat);
    result.duration_beats = std::max(
        1.0 / static_cast<double>(config_.subdivisions_per_beat),
        round_to_grid(duration_beats, config_.subdivisions_per_beat));
    result.quantized_start_seconds =
        grid.first_beat_seconds + result.start_beats * grid.beat_duration_seconds;
    result.quantized_duration_seconds = result.duration_beats * grid.beat_duration_seconds;
    return result;
}

}  // namespace music_elf

