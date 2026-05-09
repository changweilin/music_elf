#include "music_elf/dynamics_analyzer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

float amplitude_to_db(float amplitude, float floor_db) {
    if (amplitude <= 0.0f) {
        return floor_db;
    }
    return std::max(floor_db, 20.0f * std::log10(amplitude));
}

int velocity_from_db(float db, float min_db, float max_db) {
    if (max_db - min_db < 1.0f) {
        return 80;
    }
    const float normalized = std::max(0.0f, std::min(1.0f, (db - min_db) / (max_db - min_db)));
    return static_cast<int>(std::lround(1.0f + normalized * 126.0f));
}

DynamicMark mark_from_velocity(int velocity) {
    if (velocity < 35) {
        return DynamicMark::Piano;
    }
    if (velocity < 55) {
        return DynamicMark::MezzoPiano;
    }
    if (velocity < 85) {
        return DynamicMark::MezzoForte;
    }
    if (velocity < 110) {
        return DynamicMark::Forte;
    }
    return DynamicMark::Fortissimo;
}

}  // namespace

DynamicsAnalyzer::DynamicsAnalyzer(const DynamicsAnalyzerConfig& config) : config_(config) {
    if (config_.sample_rate <= 0) {
        throw std::invalid_argument("sample_rate must be positive");
    }
    if (config_.silence_floor_db >= 0.0f) {
        throw std::invalid_argument("silence_floor_db must be negative");
    }
}

const DynamicsAnalyzerConfig& DynamicsAnalyzer::config() const noexcept {
    return config_;
}

std::vector<NoteDynamics> DynamicsAnalyzer::analyze(
    const float* samples,
    std::size_t sample_count,
    const NoteEvent* notes,
    std::size_t note_count) const {
    if (sample_count > 0 && samples == nullptr) {
        throw std::invalid_argument("samples must not be null when sample_count is non-zero");
    }
    if (note_count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when note_count is non-zero");
    }

    std::vector<NoteDynamics> dynamics;
    dynamics.reserve(note_count);

    for (std::size_t i = 0; i < note_count; ++i) {
        const NoteEvent& note = notes[i];
        const auto start = static_cast<std::size_t>(
            std::max(0.0, std::floor(note.start_seconds * static_cast<double>(config_.sample_rate))));
        const auto end = static_cast<std::size_t>(
            std::max(0.0, std::ceil(note.end_seconds * static_cast<double>(config_.sample_rate))));
        const std::size_t clamped_start = std::min(start, sample_count);
        const std::size_t clamped_end = std::min(std::max(end, clamped_start), sample_count);

        double square_sum = 0.0;
        float peak = 0.0f;
        for (std::size_t sample = clamped_start; sample < clamped_end; ++sample) {
            const float absolute = std::fabs(samples[sample]);
            peak = std::max(peak, absolute);
            square_sum += static_cast<double>(samples[sample]) * static_cast<double>(samples[sample]);
        }

        const std::size_t length = clamped_end - clamped_start;
        const float rms = length > 0
                              ? static_cast<float>(std::sqrt(square_sum / static_cast<double>(length)))
                              : 0.0f;
        NoteDynamics item;
        item.note = note;
        item.rms_db = amplitude_to_db(rms, config_.silence_floor_db);
        item.peak_db = amplitude_to_db(peak, config_.silence_floor_db);
        dynamics.push_back(item);
    }

    if (dynamics.empty()) {
        return dynamics;
    }

    const auto minmax = std::minmax_element(
        dynamics.begin(),
        dynamics.end(),
        [](const NoteDynamics& left, const NoteDynamics& right) {
            return left.rms_db < right.rms_db;
        });
    const float min_db = minmax.first->rms_db;
    const float max_db = minmax.second->rms_db;

    for (auto& item : dynamics) {
        item.velocity = velocity_from_db(item.rms_db, min_db, max_db);
        item.dynamic_mark = mark_from_velocity(item.velocity);
    }

    return dynamics;
}

const char* dynamic_mark_name(DynamicMark mark) noexcept {
    switch (mark) {
        case DynamicMark::Piano:
            return "p";
        case DynamicMark::MezzoPiano:
            return "mp";
        case DynamicMark::MezzoForte:
            return "mf";
        case DynamicMark::Forte:
            return "f";
        case DynamicMark::Fortissimo:
            return "ff";
    }
    return "mf";
}

}  // namespace music_elf

