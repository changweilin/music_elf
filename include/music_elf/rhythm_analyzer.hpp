#ifndef MUSIC_ELF_RHYTHM_ANALYZER_HPP
#define MUSIC_ELF_RHYTHM_ANALYZER_HPP

#include "music_elf/note_segmenter.hpp"

#include <vector>

namespace music_elf {

struct RhythmAnalyzerConfig {
    double min_bpm = 60.0;
    double max_bpm = 200.0;
    int beats_per_bar = 4;
    int subdivisions_per_beat = 4;
};

struct BeatGrid {
    double bpm = 120.0;
    int beats_per_bar = 4;
    double first_beat_seconds = 0.0;
    double beat_duration_seconds = 0.5;
    std::vector<double> beat_times_seconds;
};

struct QuantizedNote {
    NoteEvent note;
    double start_beats = 0.0;
    double duration_beats = 0.0;
    double quantized_start_seconds = 0.0;
    double quantized_duration_seconds = 0.0;
};

struct RhythmAnalysis {
    BeatGrid beat_grid;
    std::vector<QuantizedNote> quantized_notes;
};

class RhythmAnalyzer {
public:
    explicit RhythmAnalyzer(const RhythmAnalyzerConfig& config = RhythmAnalyzerConfig{});

    const RhythmAnalyzerConfig& config() const noexcept;

    RhythmAnalysis analyze(const NoteEvent* notes, std::size_t count) const;

private:
    double estimate_beat_duration(const NoteEvent* notes, std::size_t count) const;
    BeatGrid build_beat_grid(const NoteEvent* notes, std::size_t count, double beat_duration) const;
    QuantizedNote quantize_note(const NoteEvent& note, const BeatGrid& grid) const;

    RhythmAnalyzerConfig config_;
};

}  // namespace music_elf

#endif  // MUSIC_ELF_RHYTHM_ANALYZER_HPP

