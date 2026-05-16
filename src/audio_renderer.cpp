#include "music_elf/audio_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

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

std::size_t validated_frame_count(const AudioBuffer& audio, const char* name) {
    if (audio.sample_rate <= 0) {
        throw std::invalid_argument(std::string(name) + " sample_rate must be positive");
    }
    if (audio.channels <= 0) {
        throw std::invalid_argument(std::string(name) + " channels must be positive");
    }
    if (audio.samples.size() % static_cast<std::size_t>(audio.channels) != 0) {
        throw std::invalid_argument(std::string(name) + " sample count must be divisible by channel count");
    }
    return audio.samples.size() / static_cast<std::size_t>(audio.channels);
}

float sample_for_output_channel(
    const AudioBuffer& audio,
    std::size_t frame,
    int channel,
    int output_channels) {
    const auto channels = static_cast<std::size_t>(audio.channels);
    const auto base = frame * channels;
    if (audio.channels == output_channels) {
        return audio.samples[base + static_cast<std::size_t>(channel)];
    }
    if (audio.channels == 1) {
        return audio.samples[base];
    }
    if (output_channels == 1) {
        double sum = 0.0;
        for (int input_channel = 0; input_channel < audio.channels; ++input_channel) {
            sum += audio.samples[base + static_cast<std::size_t>(input_channel)];
        }
        return static_cast<float>(sum / static_cast<double>(audio.channels));
    }
    if (channel < audio.channels) {
        return audio.samples[base + static_cast<std::size_t>(channel)];
    }
    return audio.samples[base + channels - 1];
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
    if (count == 0) {
        return make_mono_audio(config.sample_rate, {});
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

AudioBuffer mix_audio_buffers(
    const AudioBuffer& source,
    const AudioBuffer& overlay,
    const AudioMixConfig& config) {
    const std::size_t source_frames = validated_frame_count(source, "source");
    const std::size_t overlay_frames = validated_frame_count(overlay, "overlay");
    if (source.sample_rate != overlay.sample_rate) {
        throw std::invalid_argument("source and overlay sample rates must match");
    }
    if (config.source_gain < 0.0f) {
        throw std::invalid_argument("source_gain must be non-negative");
    }
    if (config.overlay_gain < 0.0f) {
        throw std::invalid_argument("overlay_gain must be non-negative");
    }

    AudioBuffer mixed;
    mixed.sample_rate = source.sample_rate;
    mixed.channels = source.channels;

    const std::size_t frames = std::max(source_frames, overlay_frames);
    mixed.samples.assign(
        frames * static_cast<std::size_t>(mixed.channels),
        0.0f);

    for (std::size_t frame = 0; frame < frames; ++frame) {
        for (int channel = 0; channel < mixed.channels; ++channel) {
            double value = 0.0;
            if (frame < source_frames) {
                value += static_cast<double>(
                    sample_for_output_channel(source, frame, channel, mixed.channels)) *
                    static_cast<double>(config.source_gain);
            }
            if (frame < overlay_frames) {
                value += static_cast<double>(
                    sample_for_output_channel(overlay, frame, channel, mixed.channels)) *
                    static_cast<double>(config.overlay_gain);
            }
            mixed.samples[frame * static_cast<std::size_t>(mixed.channels) + static_cast<std::size_t>(channel)] =
                static_cast<float>(value);
        }
    }

    float peak = 0.0f;
    for (float sample : mixed.samples) {
        peak = std::max(peak, std::fabs(sample));
    }
    if (config.normalize_peak && peak > 1.0f) {
        for (float& sample : mixed.samples) {
            sample /= peak;
        }
    } else {
        for (float& sample : mixed.samples) {
            sample = std::clamp(sample, -1.0f, 1.0f);
        }
    }

    return mixed;
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
