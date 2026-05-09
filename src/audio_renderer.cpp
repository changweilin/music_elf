#include "music_elf/audio_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

constexpr double kPi = 3.1415926535897932384626433832795;

double midi_to_frequency(int midi_note) {
    return 440.0 * std::pow(2.0, (static_cast<double>(midi_note) - 69.0) / 12.0);
}

float oscillator(double phase, PreviewWaveform waveform) {
    const double normalized = phase / (2.0 * kPi);
    const double wrapped = normalized - std::floor(normalized);
    switch (waveform) {
        case PreviewWaveform::Sine:
            return static_cast<float>(std::sin(phase));
        case PreviewWaveform::Square:
            return wrapped < 0.5 ? 1.0f : -1.0f;
        case PreviewWaveform::Saw:
            return static_cast<float>(wrapped * 2.0 - 1.0);
        case PreviewWaveform::Triangle:
            return static_cast<float>(1.0 - 4.0 * std::fabs(wrapped - 0.5));
    }
    return static_cast<float>(std::sin(phase));
}

float envelope(double time, double duration, const AudioRendererConfig& config) {
    if (duration <= 0.0) {
        return 0.0f;
    }
    const double attack = std::max(0.0001, config.attack_seconds);
    const double release = std::max(0.0001, config.release_seconds);
    const double fade_in = std::min(1.0, time / attack);
    const double fade_out = std::min(1.0, (duration - time) / release);
    return static_cast<float>(std::max(0.0, std::min(fade_in, fade_out)));
}

}  // namespace

AudioBuffer render_notes_to_audio(
    const GeneratedNote* notes,
    std::size_t count,
    const AudioRendererConfig& config) {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }
    if (config.sample_rate <= 0) {
        throw std::invalid_argument("sample_rate must be positive");
    }
    if (config.master_gain < 0.0f) {
        throw std::invalid_argument("master_gain must be non-negative");
    }

    double end_seconds = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        end_seconds = std::max(end_seconds, notes[i].end_seconds);
    }

    const std::size_t sample_count = static_cast<std::size_t>(
        std::ceil((end_seconds + config.tail_seconds) * static_cast<double>(config.sample_rate)));
    std::vector<float> samples(sample_count, 0.0f);

    for (std::size_t note_index = 0; note_index < count; ++note_index) {
        const GeneratedNote& note = notes[note_index];
        if (note.end_seconds <= note.start_seconds || note.midi_note < 0 || note.midi_note > 127) {
            continue;
        }

        const double frequency = midi_to_frequency(note.midi_note);
        const double duration = note.end_seconds - note.start_seconds;
        const std::size_t start_sample = static_cast<std::size_t>(
            std::max(0.0, std::floor(note.start_seconds * static_cast<double>(config.sample_rate))));
        const std::size_t end_sample = std::min(
            samples.size(),
            static_cast<std::size_t>(
                std::ceil(note.end_seconds * static_cast<double>(config.sample_rate))));
        const float velocity_gain = static_cast<float>(std::max(1, std::min(127, note.velocity))) / 127.0f;

        for (std::size_t sample = start_sample; sample < end_sample; ++sample) {
            const double absolute_time = static_cast<double>(sample) / static_cast<double>(config.sample_rate);
            const double local_time = absolute_time - note.start_seconds;
            const double phase = 2.0 * kPi * frequency * local_time;
            const float value =
                oscillator(phase, config.waveform) * envelope(local_time, duration, config) *
                velocity_gain * config.master_gain;
            samples[sample] += value;
        }
    }

    float peak = 0.0f;
    for (float sample : samples) {
        peak = std::max(peak, std::fabs(sample));
    }
    if (peak > 1.0f) {
        for (float& sample : samples) {
            sample /= peak;
        }
    }

    return make_mono_audio(config.sample_rate, std::move(samples));
}

const char* preview_waveform_name(PreviewWaveform waveform) noexcept {
    switch (waveform) {
        case PreviewWaveform::Sine:
            return "sine";
        case PreviewWaveform::Square:
            return "square";
        case PreviewWaveform::Saw:
            return "saw";
        case PreviewWaveform::Triangle:
            return "triangle";
    }
    return "sine";
}

}  // namespace music_elf

