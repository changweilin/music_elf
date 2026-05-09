#include "music_elf/note_segmenter.hpp"

#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

double midi_note_to_frequency(int midi_note) {
    return 440.0 * std::pow(2.0, (static_cast<double>(midi_note) - 69.0) / 12.0);
}

double cents_between(double frequency_hz, double reference_hz) {
    return 1200.0 * std::log2(frequency_hz / reference_hz);
}

void validate_output(const NoteEvent* out, std::size_t out_capacity) {
    if (out_capacity > 0 && out == nullptr) {
        throw std::invalid_argument("out must not be null when out_capacity is non-zero");
    }
}

}  // namespace

NoteSegmenter::NoteSegmenter(const NoteSegmenterConfig& config) : config_(config) {
    if (config_.min_confidence < 0.0f || config_.min_confidence > 1.0f) {
        throw std::invalid_argument("min_confidence must be in [0, 1]");
    }
    if (config_.min_note_duration_seconds < 0.0) {
        throw std::invalid_argument("min_note_duration_seconds must be non-negative");
    }
    if (config_.max_unvoiced_gap_seconds < 0.0) {
        throw std::invalid_argument("max_unvoiced_gap_seconds must be non-negative");
    }
    if (config_.pitch_change_tolerance_cents <= 0.0) {
        throw std::invalid_argument("pitch_change_tolerance_cents must be positive");
    }
}

const NoteSegmenterConfig& NoteSegmenter::config() const noexcept {
    return config_;
}

std::size_t NoteSegmenter::process(
    const PitchEstimate* estimates,
    std::size_t count,
    NoteEvent* out,
    std::size_t out_capacity) {
    if (count > 0 && estimates == nullptr) {
        throw std::invalid_argument("estimates must not be null when count is non-zero");
    }
    validate_output(out, out_capacity);

    std::size_t written = 0;

    for (std::size_t i = 0; i < count; ++i) {
        const PitchEstimate& estimate = estimates[i];
        update_frame_interval(estimate.time_seconds);

        if (is_usable_voiced_frame(estimate)) {
            if (!active_) {
                start_note(estimate);
            } else if (belongs_to_active_note(estimate)) {
                append_to_note(estimate);
            } else {
                emit_active_note(out, out_capacity, written);
                start_note(estimate);
            }
            in_unvoiced_gap_ = false;
            continue;
        }

        if (!active_) {
            continue;
        }

        if (!in_unvoiced_gap_) {
            in_unvoiced_gap_ = true;
            unvoiced_gap_start_seconds_ = estimate.time_seconds;
        }

        if (estimate.time_seconds - unvoiced_gap_start_seconds_ > config_.max_unvoiced_gap_seconds) {
            emit_active_note(out, out_capacity, written);
            active_ = false;
            in_unvoiced_gap_ = false;
        }
    }

    return written;
}

std::size_t NoteSegmenter::flush(NoteEvent* out, std::size_t out_capacity) {
    validate_output(out, out_capacity);

    std::size_t written = 0;
    if (active_) {
        emit_active_note(out, out_capacity, written);
        active_ = false;
    }
    in_unvoiced_gap_ = false;
    return written;
}

void NoteSegmenter::reset() noexcept {
    active_ = false;
    in_unvoiced_gap_ = false;
    active_note_ = ActiveNote{};
    unvoiced_gap_start_seconds_ = 0.0;
    last_frame_time_seconds_ = 0.0;
    frame_interval_seconds_ = 0.0;
    has_last_frame_time_ = false;
}

bool NoteSegmenter::is_usable_voiced_frame(const PitchEstimate& estimate) const noexcept {
    return estimate.voiced && estimate.frequency_hz > 0.0f && estimate.midi_note > 0 &&
           estimate.confidence >= config_.min_confidence;
}

bool NoteSegmenter::belongs_to_active_note(const PitchEstimate& estimate) const noexcept {
    const double reference_frequency = midi_note_to_frequency(active_note_.midi_note);
    const double cents_from_active = cents_between(estimate.frequency_hz, reference_frequency);
    return std::fabs(cents_from_active) <= config_.pitch_change_tolerance_cents;
}

void NoteSegmenter::start_note(const PitchEstimate& estimate) {
    active_note_ = ActiveNote{};
    active_note_.start_seconds = estimate.time_seconds;
    active_note_.last_voiced_seconds = estimate.time_seconds;
    active_note_.midi_note = estimate.midi_note;
    active_ = true;
    append_to_note(estimate);
}

void NoteSegmenter::append_to_note(const PitchEstimate& estimate) {
    active_note_.last_voiced_seconds = estimate.time_seconds;
    active_note_.frequency_sum += static_cast<double>(estimate.frequency_hz);
    active_note_.cents_sum += static_cast<double>(estimate.cents);
    active_note_.confidence_sum += static_cast<double>(estimate.confidence);
    active_note_.voiced_frames += 1;
}

bool NoteSegmenter::build_note_event(NoteEvent& event) const noexcept {
    if (!active_ || active_note_.voiced_frames == 0) {
        return false;
    }

    const double frame_tail = frame_interval_seconds_ > 0.0 ? frame_interval_seconds_ : 0.0;
    event.start_seconds = active_note_.start_seconds;
    event.end_seconds = active_note_.last_voiced_seconds + frame_tail;
    event.duration_seconds = event.end_seconds - event.start_seconds;
    if (event.duration_seconds < config_.min_note_duration_seconds) {
        return false;
    }

    const double frame_count = static_cast<double>(active_note_.voiced_frames);
    event.midi_note = active_note_.midi_note;
    event.average_frequency_hz = static_cast<float>(active_note_.frequency_sum / frame_count);
    event.average_cents = static_cast<float>(active_note_.cents_sum / frame_count);
    event.average_confidence = static_cast<float>(active_note_.confidence_sum / frame_count);
    return true;
}

void NoteSegmenter::emit_active_note(NoteEvent* out, std::size_t out_capacity, std::size_t& written) {
    NoteEvent event;
    const bool has_event = build_note_event(event);
    active_ = false;
    in_unvoiced_gap_ = false;
    active_note_ = ActiveNote{};

    if (!has_event) {
        return;
    }
    if (written >= out_capacity) {
        throw std::overflow_error("note segmenter output capacity was too small");
    }
    out[written] = event;
    written += 1;
}

void NoteSegmenter::update_frame_interval(double time_seconds) noexcept {
    if (has_last_frame_time_) {
        const double delta = time_seconds - last_frame_time_seconds_;
        if (delta > 0.0 && delta < 1.0) {
            frame_interval_seconds_ = delta;
        }
    }
    last_frame_time_seconds_ = time_seconds;
    has_last_frame_time_ = true;
}

}  // namespace music_elf

