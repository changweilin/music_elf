#include "music_elf/audio_io.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kPi = 3.1415926535897932384626433832795;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::vector<float> make_sine(int sample_rate, double seconds, double frequency_hz) {
    std::vector<float> samples(static_cast<std::size_t>(seconds * sample_rate));
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const double time = static_cast<double>(i) / static_cast<double>(sample_rate);
        samples[i] = static_cast<float>(0.5 * std::sin(2.0 * kPi * frequency_hz * time));
    }
    return samples;
}

void test_wav_roundtrip_mono() {
    const std::string path = "audio_io_roundtrip.wav";
    const auto source = music_elf::make_mono_audio(48000, make_sine(48000, 0.25, 440.0));
    music_elf::write_wav_file(path, source);
    const auto loaded = music_elf::read_wav_file(path);
    std::remove(path.c_str());

    require(loaded.sample_rate == source.sample_rate, "sample rate should roundtrip");
    require(loaded.channels == 1, "channel count should roundtrip");
    require(loaded.samples.size() == source.samples.size(), "sample count should roundtrip");
    require(std::fabs(loaded.samples[100] - source.samples[100]) < 0.001f, "PCM16 sample should be close");
}

void test_downmix_stereo() {
    music_elf::AudioBuffer audio;
    audio.sample_rate = 48000;
    audio.channels = 2;
    audio.samples = {1.0f, 0.0f, 0.25f, -0.25f};
    const auto mono = music_elf::downmix_to_mono(audio);
    require(mono.size() == 2, "downmix frame count");
    require(std::fabs(mono[0] - 0.5f) < 0.0001f, "downmix first frame");
    require(std::fabs(mono[1] - 0.0f) < 0.0001f, "downmix second frame");
}

}  // namespace

int main() {
    try {
        test_wav_roundtrip_mono();
        test_downmix_stereo();
    } catch (const std::exception& error) {
        std::cerr << "audio_io_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "audio_io_tests passed\n";
    return EXIT_SUCCESS;
}

