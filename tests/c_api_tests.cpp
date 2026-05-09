#include "music_elf/c_api.h"

#include "music_elf/audio_io.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
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

void test_c_pitch_detector() {
    MusicElfPitchDetectorConfig config = music_elf_default_pitch_detector_config();
    MusicElfPitchDetector* detector = nullptr;
    require(music_elf_pitch_detector_create(&config, &detector) == 0, "C detector create");
    require(detector != nullptr, "C detector pointer");

    const auto samples = make_sine(config.sample_rate, 1.0, 440.0);
    std::vector<MusicElfPitchEstimate> estimates(1024);
    size_t written = 0;
    require(music_elf_pitch_detector_process(
                detector,
                samples.data(),
                samples.size(),
                estimates.data(),
                estimates.size(),
                &written) == 0,
            "C detector process");
    music_elf_pitch_detector_destroy(detector);

    require(written > 100, "C detector should produce frames");
    bool found_voiced = false;
    for (std::size_t i = 0; i < written; ++i) {
        found_voiced = found_voiced || estimates[i].voiced != 0;
    }
    require(found_voiced, "C detector should find voiced frames");
}

void test_c_wav_to_midi() {
    const std::string wav_path = "c_api_input.wav";
    const std::string midi_path = "c_api_output.mid";
    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.0, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    require(music_elf_process_wav_to_midi(wav_path.c_str(), midi_path.c_str()) == 0, "C WAV to MIDI");
    std::ifstream in(midi_path, std::ios::binary);
    std::vector<char> header(4);
    in.read(header.data(), static_cast<std::streamsize>(header.size()));
    std::remove(wav_path.c_str());
    std::remove(midi_path.c_str());
    require(header[0] == 'M' && header[1] == 'T' && header[2] == 'h' && header[3] == 'd', "C MIDI header");
}

}  // namespace

int main() {
    try {
        test_c_pitch_detector();
        test_c_wav_to_midi();
    } catch (const std::exception& error) {
        std::cerr << "c_api_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "c_api_tests passed\n";
    return EXIT_SUCCESS;
}

