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

bool file_contains(const std::string& path, const std::string& needle) {
    std::ifstream in(path, std::ios::binary);
    const std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return data.find(needle) != std::string::npos;
}

void test_cli_wav_to_outputs(const std::string& cli_path) {
    const std::string wav_path = "cli_input.wav";
    const std::string midi_path = "cli_output.mid";
    const std::string musicxml_path = "cli_output.musicxml";

    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.2, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    const std::string command = "cmd /c \"\"" + cli_path + "\" " + wav_path +
                                " --out-midi " + midi_path +
                                " --out-musicxml " + musicxml_path +
                                " --lyrics I_sing --pattern arpeggio\"";
    const int exit_code = std::system(command.c_str());

    require(exit_code == 0, "CLI command should exit successfully");
    require(file_contains(midi_path, "MThd"), "CLI should write MIDI header");
    require(file_contains(musicxml_path, "<score-partwise"), "CLI should write MusicXML");

    std::remove(wav_path.c_str());
    std::remove(midi_path.c_str());
    std::remove(musicxml_path.c_str());
}

}  // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "expected path to music_elf_cli");
        test_cli_wav_to_outputs(argv[1]);
    } catch (const std::exception& error) {
        std::cerr << "cli_end_to_end_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "cli_end_to_end_tests passed\n";
    return EXIT_SUCCESS;
}
