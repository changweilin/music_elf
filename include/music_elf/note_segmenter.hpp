#ifndef MUSIC_ELF_NOTE_SEGMENTER_HPP
#define MUSIC_ELF_NOTE_SEGMENTER_HPP

#include "music_elf/pitch_detector.hpp"

#include <cstddef>

namespace music_elf {

struct NoteSegmenterConfig {
    float min_confidence = 0.70f;
    double min_note_duration_seconds = 0.05;
    double max_unvoiced_gap_seconds = 0.04;
    double pitch_change_tolerance_cents = 50.0;
};

struct NoteEvent {
    double start_seconds = 0.0;
    double end_seconds = 0.0;
    double duration_seconds = 0.0;
    int midi_note = 0;
    float average_frequency_hz = 0.0f;
    float average_cents = 0.0f;
    float average_confidence = 0.0f;
};

class NoteSegmenter {
public:
    explicit NoteSegmenter(const NoteSegmenterConfig& config = NoteSegmenterConfig{});

    const NoteSegmenterConfig& config() const noexcept;

    std::size_t process(
        const PitchEstimate* estimates,
        std::size_t count,
        NoteEvent* out,
        std::size_t out_capacity);

    std::size_t flush(NoteEvent* out, std::size_t out_capacity);

    void reset() noexcept;

private:
    struct ActiveNote {
        double start_seconds = 0.0;
        double last_voiced_seconds = 0.0;
        int midi_note = 0;
        double frequency_sum = 0.0;
        double cents_sum = 0.0;
        double confidence_sum = 0.0;
        std::size_t voiced_frames = 0;
    };

    bool is_usable_voiced_frame(const PitchEstimate& estimate) const noexcept;
    bool belongs_to_active_note(const PitchEstimate& estimate) const noexcept;
    void start_note(const PitchEstimate& estimate);
    void append_to_note(const PitchEstimate& estimate);
    bool build_note_event(NoteEvent& event) const noexcept;
    void emit_active_note(NoteEvent* out, std::size_t out_capacity, std::size_t& written);
    void update_frame_interval(double time_seconds) noexcept;

    NoteSegmenterConfig config_;
    bool active_ = false;
    bool in_unvoiced_gap_ = false;
    ActiveNote active_note_;
    double unvoiced_gap_start_seconds_ = 0.0;
    double last_frame_time_seconds_ = 0.0;
    double frame_interval_seconds_ = 0.0;
    bool has_last_frame_time_ = false;
};

}  // namespace music_elf

#endif  // MUSIC_ELF_NOTE_SEGMENTER_HPP

