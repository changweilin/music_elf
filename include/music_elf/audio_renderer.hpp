#ifndef MUSIC_ELF_AUDIO_RENDERER_HPP
#define MUSIC_ELF_AUDIO_RENDERER_HPP

#include "music_elf/accompaniment_generator.hpp"
#include "music_elf/audio_io.hpp"

#include <cstddef>

namespace music_elf {

enum class PreviewWaveform {
    Sine,
    Square,
    Saw,
    Triangle,
};

struct AudioRendererConfig {
    int sample_rate = 48000;
    PreviewWaveform waveform = PreviewWaveform::Sine;
    float master_gain = 0.25f;
    double tail_seconds = 0.10;
    double attack_seconds = 0.01;
    double release_seconds = 0.04;
};

struct AudioMixConfig {
    float source_gain = 1.0f;
    float overlay_gain = 1.0f;
    bool normalize_peak = true;
};

AudioBuffer render_notes_to_audio(
    const GeneratedNote* notes,
    std::size_t count,
    const AudioRendererConfig& config = AudioRendererConfig{});

AudioBuffer mix_audio_buffers(
    const AudioBuffer& source,
    const AudioBuffer& overlay,
    const AudioMixConfig& config = AudioMixConfig{});

const char* preview_waveform_name(PreviewWaveform waveform) noexcept;

}  // namespace music_elf

#endif  // MUSIC_ELF_AUDIO_RENDERER_HPP
