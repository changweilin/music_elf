#include "music_elf/audio_io.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
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
    require(file_contains(musicxml_path, "<time><beats>4</beats><beat-type>4</beat-type></time>"),
            "CLI MusicXML should include time signature");
    require(file_contains(musicxml_path, "<clef><sign>G</sign><line>2</line></clef>"),
            "CLI MusicXML should include clef");

    std::remove(wav_path.c_str());
    std::remove(midi_path.c_str());
    std::remove(musicxml_path.c_str());
}

void test_cli_generates_catalog_subset(const std::string& cli_path) {
    const std::string out_dir = "cli_catalog_out";
    std::filesystem::remove_all(out_dir);

    const std::string command = "cmd /c \"\"" + cli_path +
                                "\" generate-catalog --out-dir " + out_dir +
                                " --instruments 0,40 --roots C,D --chords major,minor\"";
    const int exit_code = std::system(command.c_str());
    require(exit_code == 0, "catalog CLI command should exit successfully");

    int midi_count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(out_dir)) {
        if (entry.path().extension() == ".mid") {
            midi_count += 1;
        }
    }
    std::filesystem::remove_all(out_dir);
    require(midi_count == 8, "catalog subset should write 8 MIDI files");
}

void test_cli_render_demo_wav(const std::string& cli_path) {
    const std::string wav_path = "cli_preview.wav";
    std::remove(wav_path.c_str());

    const std::string command = "cmd /c \"\"" + cli_path +
                                "\" render-demo --out-wav " + wav_path +
                                " --program 0 --root C --chord major\"";
    const int exit_code = std::system(command.c_str());
    require(exit_code == 0, "render-demo CLI command should exit successfully");
    require(file_contains(wav_path, "RIFF"), "render-demo should write WAV");
    std::remove(wav_path.c_str());
}

void test_cli_inspect_writes_summary(const std::string& cli_path) {
    const std::string wav_path = "cli_inspect_input.wav";
    const std::string summary_path = "cli_inspect_summary.txt";
    std::remove(summary_path.c_str());

    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.0, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    const std::string command = "cmd /c \"\"" + cli_path +
                                "\" inspect " + wav_path +
                                " --out-summary " + summary_path +
                                " --lyrics la --pattern block\"";
    const int exit_code = std::system(command.c_str());
    require(exit_code == 0, "inspect CLI command should exit successfully");
    require(file_contains(summary_path, "command: inspect"), "inspect summary command");
    require(file_contains(summary_path, "pitch_frames:"), "inspect summary pitch frames");
    require(file_contains(summary_path, "musicxml_chars:"), "inspect summary MusicXML size");

    std::remove(wav_path.c_str());
    std::remove(summary_path.c_str());
}

void test_cli_benchmark_writes_runtime_summary(const std::string& cli_path) {
    const std::string wav_path = "cli_benchmark_input.wav";
    const std::string summary_path = "cli_benchmark_summary.txt";
    std::remove(summary_path.c_str());

    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.0, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    const std::string command = "cmd /c \"\"" + cli_path +
                                "\" benchmark " + wav_path +
                                " --iterations 2 --out-summary " + summary_path + "\"";
    const int exit_code = std::system(command.c_str());
    require(exit_code == 0, "benchmark CLI command should exit successfully");
    require(file_contains(summary_path, "command: benchmark"), "benchmark summary command");
    require(file_contains(summary_path, "benchmark_iterations: 2"), "benchmark iteration count");
    require(file_contains(summary_path, "benchmark_average_ms:"), "benchmark average");

    std::remove(wav_path.c_str());
    std::remove(summary_path.c_str());
}

void test_cli_render_preview_wav(const std::string& cli_path) {
    const std::string wav_path = "cli_render_preview_input.wav";
    const std::string preview_path = "cli_render_preview.wav";
    std::remove(preview_path.c_str());

    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.0, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    const std::string command = "cmd /c \"\"" + cli_path +
                                "\" render-preview " + wav_path +
                                " --out-wav " + preview_path +
                                " --waveform triangle --pattern arpeggio\"";
    const int exit_code = std::system(command.c_str());
    require(exit_code == 0, "render-preview CLI command should exit successfully");
    require(file_contains(preview_path, "RIFF"), "render-preview should write WAV");

    std::remove(wav_path.c_str());
    std::remove(preview_path.c_str());
}

}  // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "expected path to music_elf_cli");
        test_cli_wav_to_outputs(argv[1]);
        test_cli_generates_catalog_subset(argv[1]);
        test_cli_render_demo_wav(argv[1]);
        test_cli_inspect_writes_summary(argv[1]);
        test_cli_benchmark_writes_runtime_summary(argv[1]);
        test_cli_render_preview_wav(argv[1]);
    } catch (const std::exception& error) {
        std::cerr << "cli_end_to_end_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "cli_end_to_end_tests passed\n";
    return EXIT_SUCCESS;
}
