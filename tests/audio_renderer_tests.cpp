#include "music_elf/audio_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

float peak_abs(const std::vector<float>& samples) {
    float peak = 0.0f;
    for (float sample : samples) {
        peak = std::max(peak, std::fabs(sample));
    }
    return peak;
}

void test_render_notes_to_audio() {
    const std::vector<music_elf::GeneratedNote> notes = {
        {0.0, 0.5, 60, 100, 0},
        {0.5, 1.0, 64, 100, 0},
    };
    music_elf::AudioRendererConfig config;
    config.sample_rate = 48000;
    const auto audio = music_elf::render_notes_to_audio(notes.data(), notes.size(), config);

    require(audio.sample_rate == 48000, "render sample rate");
    require(audio.channels == 1, "render channel count");
    require(audio.samples.size() >= 48000, "render duration");
    require(peak_abs(audio.samples) > 0.05f, "render should produce audible signal");
    require(peak_abs(audio.samples) <= 1.0f, "render should normalize peak");
}

void test_write_rendered_wav() {
    const std::string path = "audio_renderer_preview.wav";
    const std::vector<music_elf::GeneratedNote> notes = {
        {0.0, 0.25, 69, 90, 0},
    };
    const auto audio = music_elf::render_notes_to_audio(notes.data(), notes.size());
    music_elf::write_wav_file(path, audio);
    const auto loaded = music_elf::read_wav_file(path);
    std::remove(path.c_str());

    require(loaded.sample_rate == audio.sample_rate, "preview wav sample rate");
    require(!loaded.samples.empty(), "preview wav samples");
}

void test_render_empty_notes_returns_empty_audio() {
    music_elf::AudioRendererConfig config;
    config.sample_rate = 48000;
    const auto audio = music_elf::render_notes_to_audio(nullptr, 0, config);

    require(audio.sample_rate == 48000, "empty render sample rate");
    require(audio.channels == 1, "empty render channel count");
    require(audio.samples.empty(), "empty render should not add a tail");
}

void test_mix_audio_buffers_preserves_source_channels() {
    music_elf::AudioBuffer source;
    source.sample_rate = 48000;
    source.channels = 2;
    source.samples = {
        0.25f, -0.25f,
        0.25f, -0.25f,
    };

    const auto overlay = music_elf::make_mono_audio(
        48000,
        std::vector<float>{0.50f, 0.50f});
    music_elf::AudioMixConfig config;
    config.normalize_peak = false;

    const auto mixed = music_elf::mix_audio_buffers(source, overlay, config);

    require(mixed.sample_rate == source.sample_rate, "mix sample rate");
    require(mixed.channels == source.channels, "mix should preserve source channels");
    require(mixed.samples.size() == source.samples.size(), "mix sample count");
    require(std::fabs(mixed.samples[0] - 0.75f) < 0.0001f, "mix left channel");
    require(std::fabs(mixed.samples[1] - 0.25f) < 0.0001f, "mix right channel");
}

}  // namespace

int main() {
    try {
        test_render_notes_to_audio();
        test_write_rendered_wav();
        test_render_empty_notes_returns_empty_audio();
        test_mix_audio_buffers_preserves_source_channels();
    } catch (const std::exception& error) {
        std::cerr << "audio_renderer_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "audio_renderer_tests passed\n";
    return EXIT_SUCCESS;
}
