#include "music_elf/pitch_detector.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace music_elf {
namespace {

float clamp01(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

int choose_downsample_factor(int sample_rate, float max_frequency_hz) {
    int factor = 1;
    while (factor < 8 &&
           sample_rate / (factor * 2) >= 12000 &&
           sample_rate / (factor * 2) >= static_cast<int>(max_frequency_hz * 8.0f)) {
        factor *= 2;
    }
    return factor;
}

int frequency_to_midi_note(float frequency_hz) {
    const double midi = 69.0 + 12.0 * std::log2(static_cast<double>(frequency_hz) / 440.0);
    return static_cast<int>(std::lround(midi));
}

float frequency_to_cents(float frequency_hz, int midi_note) {
    const double midi = 69.0 + 12.0 * std::log2(static_cast<double>(frequency_hz) / 440.0);
    return static_cast<float>((midi - static_cast<double>(midi_note)) * 100.0);
}

}  // namespace

PitchDetector::PitchDetector(const PitchDetectorConfig& config) : config_(config) {
    if (config_.sample_rate <= 0) {
        throw std::invalid_argument("sample_rate must be positive");
    }
    if (config_.min_frequency_hz <= 0.0f || config_.max_frequency_hz <= 0.0f ||
        config_.min_frequency_hz >= config_.max_frequency_hz) {
        throw std::invalid_argument("frequency range is invalid");
    }
    if (config_.frame_size < 64 || config_.hop_size == 0 || config_.hop_size > config_.frame_size) {
        throw std::invalid_argument("frame_size and hop_size are invalid");
    }
    if (config_.yin_threshold <= 0.0f || config_.yin_threshold >= 1.0f) {
        throw std::invalid_argument("yin_threshold must be in (0, 1)");
    }
    if (config_.silence_rms < 0.0f) {
        throw std::invalid_argument("silence_rms must be non-negative");
    }

    downsample_factor_ = choose_downsample_factor(config_.sample_rate, config_.max_frequency_hz);
    analysis_sample_rate_ = config_.sample_rate / downsample_factor_;
    analysis_frame_size_ = config_.frame_size / static_cast<std::size_t>(downsample_factor_);
    if (analysis_frame_size_ < 32) {
        throw std::invalid_argument("frame_size is too small for the selected sample rate");
    }

    const auto raw_min_tau = static_cast<std::size_t>(
        std::floor(static_cast<float>(analysis_sample_rate_) / config_.max_frequency_hz));
    const auto raw_max_tau = static_cast<std::size_t>(
        std::ceil(static_cast<float>(analysis_sample_rate_) / config_.min_frequency_hz));

    min_tau_ = std::max<std::size_t>(2, raw_min_tau);
    max_tau_ = std::min<std::size_t>(raw_max_tau, (analysis_frame_size_ / 2) - 1);

    if (min_tau_ >= max_tau_) {
        throw std::invalid_argument("frequency range is incompatible with frame_size");
    }

    ring_.assign(config_.frame_size, 0.0f);
    raw_frame_.assign(config_.frame_size, 0.0f);
    analysis_frame_.assign(analysis_frame_size_, 0.0f);
    difference_.assign(max_tau_ + 2, 0.0f);
    cmnd_.assign(max_tau_ + 2, 1.0f);
}

const PitchDetectorConfig& PitchDetector::config() const noexcept {
    return config_;
}

std::size_t PitchDetector::process(
    const float* samples,
    std::size_t count,
    PitchEstimate* out,
    std::size_t out_capacity) {
    if (count > 0 && samples == nullptr) {
        throw std::invalid_argument("samples must not be null when count is non-zero");
    }
    if (out_capacity > 0 && out == nullptr) {
        throw std::invalid_argument("out must not be null when out_capacity is non-zero");
    }

    std::size_t written = 0;

    while (written < out_capacity && !pending_estimates_.empty()) {
        out[written] = pending_estimates_.front();
        pending_estimates_.pop_front();
        written += 1;
    }
    if (!pending_estimates_.empty()) {
        return written;
    }

    for (std::size_t i = 0; i < count; ++i) {
        ring_[ring_write_index_] = samples[i];
        ring_write_index_ = (ring_write_index_ + 1) % config_.frame_size;
        samples_seen_ += 1;
        samples_in_ring_ = std::min<std::size_t>(samples_in_ring_ + 1, config_.frame_size);

        while (samples_in_ring_ == config_.frame_size &&
               samples_seen_ >= next_frame_start_ + config_.frame_size) {
            const auto frame_start = next_frame_start_;
            next_frame_start_ += config_.hop_size;

            const PitchEstimate estimate = analyze_current_frame(frame_start);
            if (written < out_capacity) {
                out[written] = estimate;
                written += 1;
            } else {
                pending_estimates_.push_back(estimate);
            }
        }
    }

    return written;
}

PitchEstimate PitchDetector::analyze_current_frame(std::uint64_t frame_start_sample) {
    PitchEstimate estimate;
    estimate.time_seconds =
        (static_cast<double>(frame_start_sample) + static_cast<double>(config_.frame_size) * 0.5) /
        static_cast<double>(config_.sample_rate);

    double sum = 0.0;
    for (std::size_t i = 0; i < config_.frame_size; ++i) {
        const std::size_t ring_index = (ring_write_index_ + i) % config_.frame_size;
        raw_frame_[i] = ring_[ring_index];
        sum += static_cast<double>(raw_frame_[i]);
    }

    const float dc = static_cast<float>(sum / static_cast<double>(config_.frame_size));
    double square_sum = 0.0;
    for (std::size_t i = 0; i < config_.frame_size; ++i) {
        raw_frame_[i] -= dc;
        square_sum += static_cast<double>(raw_frame_[i]) * static_cast<double>(raw_frame_[i]);
    }

    const float rms = static_cast<float>(std::sqrt(square_sum / static_cast<double>(config_.frame_size)));
    if (rms <= config_.silence_rms) {
        return estimate;
    }

    if (downsample_factor_ == 1) {
        std::copy(raw_frame_.begin(), raw_frame_.begin() + static_cast<std::ptrdiff_t>(analysis_frame_size_),
                  analysis_frame_.begin());
    } else {
        const auto factor = static_cast<std::size_t>(downsample_factor_);
        for (std::size_t i = 0; i < analysis_frame_size_; ++i) {
            float block_sum = 0.0f;
            const std::size_t base = i * factor;
            for (std::size_t j = 0; j < factor; ++j) {
                block_sum += raw_frame_[base + j];
            }
            analysis_frame_[i] = block_sum / static_cast<float>(factor);
        }
    }

    std::fill(difference_.begin(), difference_.end(), 0.0f);
    for (std::size_t tau = 1; tau <= max_tau_; ++tau) {
        double diff = 0.0;
        const std::size_t limit = analysis_frame_size_ - tau;
        for (std::size_t i = 0; i < limit; ++i) {
            const float delta = analysis_frame_[i] - analysis_frame_[i + tau];
            diff += static_cast<double>(delta) * static_cast<double>(delta);
        }
        difference_[tau] = static_cast<float>(diff);
    }

    cmnd_[0] = 1.0f;
    float running_sum = 0.0f;
    for (std::size_t tau = 1; tau <= max_tau_; ++tau) {
        running_sum += difference_[tau];
        cmnd_[tau] = running_sum > std::numeric_limits<float>::epsilon()
                         ? difference_[tau] * static_cast<float>(tau) / running_sum
                         : 1.0f;
    }

    std::size_t tau_estimate = 0;
    for (std::size_t tau = min_tau_; tau <= max_tau_; ++tau) {
        if (cmnd_[tau] < config_.yin_threshold) {
            tau_estimate = tau;
            while (tau_estimate + 1 <= max_tau_ && cmnd_[tau_estimate + 1] < cmnd_[tau_estimate]) {
                tau_estimate += 1;
            }
            break;
        }
    }

    if (tau_estimate == 0) {
        float best = 1.0f;
        for (std::size_t tau = min_tau_; tau <= max_tau_; ++tau) {
            if (cmnd_[tau] < best) {
                best = cmnd_[tau];
                tau_estimate = tau;
            }
        }
        if (tau_estimate == 0 || best > config_.yin_threshold * 2.5f) {
            estimate.confidence = clamp01(1.0f - best);
            return estimate;
        }
    }

    float refined_tau = static_cast<float>(tau_estimate);
    if (tau_estimate > 1 && tau_estimate < max_tau_) {
        const float left = cmnd_[tau_estimate - 1];
        const float center = cmnd_[tau_estimate];
        const float right = cmnd_[tau_estimate + 1];
        const float denominator = 2.0f * (left - 2.0f * center + right);
        if (std::fabs(denominator) > std::numeric_limits<float>::epsilon()) {
            const float offset = (left - right) / denominator;
            if (std::fabs(offset) <= 1.0f) {
                refined_tau += offset;
            }
        }
    }

    if (refined_tau <= 0.0f) {
        return estimate;
    }

    const float frequency = static_cast<float>(analysis_sample_rate_) / refined_tau;
    if (frequency < config_.min_frequency_hz || frequency > config_.max_frequency_hz) {
        return estimate;
    }

    estimate.frequency_hz = frequency;
    estimate.midi_note = frequency_to_midi_note(frequency);
    estimate.cents = frequency_to_cents(frequency, estimate.midi_note);
    estimate.confidence = clamp01(1.0f - cmnd_[tau_estimate]);
    estimate.voiced = estimate.confidence >= (1.0f - config_.yin_threshold * 2.5f);

    if (!estimate.voiced) {
        estimate.frequency_hz = 0.0f;
        estimate.midi_note = 0;
        estimate.cents = 0.0f;
    }

    return estimate;
}

}  // namespace music_elf
