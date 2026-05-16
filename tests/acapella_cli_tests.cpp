// Runs the music_elf CLI against a real a cappella WAV (Twinkle, +4 semitone,
// 8 kHz) to cover the four user-visible subcommands: default export, inspect,
// benchmark, render-preview. Outputs are persisted to build/acapella_outputs/cli/
// for manual auditing.

#include "music_elf/audio_io.hpp"

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef MUSIC_ELF_ACAPELLA_DIR
#define MUSIC_ELF_ACAPELLA_DIR "data/acapella/fixtures"
#endif
#ifndef MUSIC_ELF_ACAPELLA_OUT
#define MUSIC_ELF_ACAPELLA_OUT "acapella_outputs"
#endif

namespace fs = std::filesystem;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool file_contains(const fs::path& path, const std::string& needle) {
    std::ifstream in(path, std::ios::binary);
    const std::string data((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
    return data.find(needle) != std::string::npos;
}

// Wraps a CLI invocation in `cmd /c "..."` so Windows handles the outer quoting,
// then quotes every argument that contains a space or special character.
int run_cli(const std::string& cli_path, const std::string& argv_tail) {
    const std::string command =
        std::string("cmd /c \"\"") + cli_path + "\" " + argv_tail + "\"";
    return std::system(command.c_str());
}

std::string q(const fs::path& p) {
    return std::string("\"") + p.string() + "\"";
}

fs::path find_cli_fixture() {
    const fs::path root{MUSIC_ELF_ACAPELLA_DIR};
    const fs::path preferred =
        root / "Twinkle twinkle little star Acapella-I8XV8UR-8Vg.wav";
    if (fs::exists(preferred)) {
        return preferred;
    }

    std::vector<fs::path> wavs;
    if (fs::exists(root)) {
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == ".wav") {
                wavs.push_back(entry.path());
            }
        }
    }
    std::sort(wavs.begin(), wavs.end());
    require(!wavs.empty(), "missing acapella WAVs under: " + root.string());
    return wavs.front();
}

void test_default_export(const std::string& cli_path,
                         const fs::path& wav,
                         const fs::path& out_dir) {
    const fs::path midi = out_dir / "twinkle.mid";
    const fs::path xml = out_dir / "twinkle.musicxml";
    fs::remove(midi);
    fs::remove(xml);

    const std::string args = q(wav) + " --out-midi " + q(midi) +
                             " --out-musicxml " + q(xml) +
                             " --lyrics \"Twinkle twinkle little star\""
                             " --pattern arpeggio";
    const int rc = run_cli(cli_path, args);
    require(rc == 0, "default export should exit 0 (got " + std::to_string(rc) + ")");
    require(file_contains(midi, "MThd"), "MIDI header missing");
    require(file_contains(xml, "<score-partwise"), "MusicXML root missing");
    require(file_contains(xml, "<harmony>"), "MusicXML chord symbols missing");
}

void test_inspect(const std::string& cli_path,
                  const fs::path& wav,
                  const fs::path& out_dir) {
    const fs::path summary = out_dir / "twinkle_inspect.txt";
    fs::remove(summary);

    const std::string args = std::string("inspect ") + q(wav) +
                             " --out-summary " + q(summary) +
                             " --lyrics \"Twinkle twinkle little star\""
                             " --pattern block";
    const int rc = run_cli(cli_path, args);
    require(rc == 0, "inspect should exit 0 (got " + std::to_string(rc) + ")");
    require(file_contains(summary, "command: inspect"),
            "inspect summary missing command label");
    require(file_contains(summary, "pitch_frames:"),
            "inspect summary missing pitch_frames");
    require(file_contains(summary, "pitch_stability_cents:"),
            "inspect summary missing pitch_stability_cents");
    require(file_contains(summary, "stable_pitch_frame_ratio:"),
            "inspect summary missing stable_pitch_frame_ratio");
    require(file_contains(summary, "musicxml_chars:"),
            "inspect summary missing musicxml_chars");
    require(file_contains(summary, "vocal_band_samples:"),
            "inspect summary missing vocal_band_samples");
}

void test_benchmark(const std::string& cli_path,
                    const fs::path& wav,
                    const fs::path& out_dir) {
    const fs::path summary = out_dir / "twinkle_benchmark.txt";
    fs::remove(summary);

    const std::string args = std::string("benchmark ") + q(wav) +
                             " --iterations 2 --out-summary " + q(summary);
    const int rc = run_cli(cli_path, args);
    require(rc == 0, "benchmark should exit 0 (got " + std::to_string(rc) + ")");
    require(file_contains(summary, "command: benchmark"),
            "benchmark summary missing command label");
    require(file_contains(summary, "benchmark_iterations: 2"),
            "benchmark iteration count wrong");
    require(file_contains(summary, "benchmark_average_ms:"),
            "benchmark average missing");
    require(file_contains(summary, "pitch_stability_cents:"),
            "benchmark summary missing pitch_stability_cents");
}

void test_render_preview(const std::string& cli_path,
                         const fs::path& wav,
                         const fs::path& out_dir) {
    const fs::path preview = out_dir / "twinkle_preview.wav";
    fs::remove(preview);

    const std::string args = std::string("render-preview ") + q(wav) +
                             " --out-wav " + q(preview) +
                             " --waveform triangle --pattern block";
    const int rc = run_cli(cli_path, args);
    require(rc == 0,
            "render-preview should exit 0 (got " + std::to_string(rc) + ")");
    require(file_contains(preview, "RIFF"),
            "render-preview WAV missing RIFF header");
}

}  // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "expected path to music_elf_cli");
        const std::string cli_path = argv[1];
        const fs::path wav = find_cli_fixture();
        require(fs::exists(wav), "missing acapella WAV: " + wav.string());

        const fs::path out_dir = fs::path{MUSIC_ELF_ACAPELLA_OUT} / "cli";
        fs::create_directories(out_dir);

        test_default_export(cli_path, wav, out_dir);
        test_inspect(cli_path, wav, out_dir);
        test_benchmark(cli_path, wav, out_dir);
        test_render_preview(cli_path, wav, out_dir);
    } catch (const std::exception& e) {
        std::cerr << "acapella_cli_tests failed: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "acapella_cli_tests passed; outputs under "
              << MUSIC_ELF_ACAPELLA_OUT << "/cli/\n";
    return EXIT_SUCCESS;
}
