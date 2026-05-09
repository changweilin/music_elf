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

}  // namespace

int main() {
    try {
        test_render_notes_to_audio();
        test_write_rendered_wav();
    } catch (const std::exception& error) {
        std::cerr << "audio_renderer_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "audio_renderer_tests passed\n";
    return EXIT_SUCCESS;
}

