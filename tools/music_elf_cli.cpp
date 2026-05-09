#include "music_elf/core_pipeline.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::AccompanimentPattern;
using music_elf::CorePipelineConfig;
using music_elf::LyricToken;

void print_usage() {
    std::cout
        << "Usage: music_elf_cli <input.wav> [--out-midi file.mid] [--out-musicxml file.musicxml]\n"
        << "                     [--lyrics \"word word\"] [--pattern block|arpeggio|broken|pad]\n";
}

std::vector<LyricToken> split_lyrics(const std::string& text) {
    std::vector<LyricToken> tokens;
    std::istringstream stream(text);
    std::string token;
    while (stream >> token) {
        tokens.push_back({token});
    }
    return tokens;
}

AccompanimentPattern parse_pattern(const std::string& value) {
    if (value == "block") {
        return AccompanimentPattern::BlockChord;
    }
    if (value == "arpeggio") {
        return AccompanimentPattern::Arpeggio;
    }
    if (value == "broken") {
        return AccompanimentPattern::BrokenChord;
    }
    if (value == "pad") {
        return AccompanimentPattern::Pad;
    }
    throw std::invalid_argument("unknown pattern: " + value);
}

void write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to write file: " + path);
    }
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_text_file(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to write file: " + path);
    }
    out << text;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            print_usage();
            return argc < 2 ? 1 : 0;
        }

        std::string input_path = argv[1];
        std::filesystem::path base(input_path);
        std::string out_midi = base.replace_extension(".mid").string();
        base = std::filesystem::path(input_path);
        std::string out_musicxml = base.replace_extension(".musicxml").string();
        std::vector<LyricToken> lyrics;
        CorePipelineConfig config;

        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help" || arg == "-h") {
                print_usage();
                return 0;
            }
            if (arg == "--out-midi" && i + 1 < argc) {
                out_midi = argv[++i];
            } else if (arg == "--out-musicxml" && i + 1 < argc) {
                out_musicxml = argv[++i];
            } else if (arg == "--lyrics" && i + 1 < argc) {
                lyrics = split_lyrics(argv[++i]);
            } else if (arg == "--pattern" && i + 1 < argc) {
                config.accompaniment.pattern = parse_pattern(argv[++i]);
            } else {
                throw std::invalid_argument("unknown or incomplete argument: " + arg);
            }
        }

        const auto audio = music_elf::read_wav_file(input_path);
        const auto result = music_elf::run_core_pipeline(audio, lyrics, config);

        write_binary_file(out_midi, result.midi_bytes);
        write_text_file(out_musicxml, result.musicxml);

        std::cout << "Music Elf analysis complete\n";
        std::cout << "Input: " << input_path << "\n";
        std::cout << "Pitch frames: " << result.pitch_estimates.size() << "\n";
        std::cout << "Notes: " << result.notes.size() << "\n";
        std::cout << "Estimated BPM: " << result.rhythm.beat_grid.bpm << "\n";
        std::cout << "Chord candidates: " << result.chord_progressions.size() << "\n";
        std::cout << "Accompaniment notes: " << result.accompaniment_notes.size() << "\n";
        std::cout << "Wrote MIDI: " << out_midi << "\n";
        std::cout << "Wrote MusicXML: " << out_musicxml << "\n";
    } catch (const std::exception& error) {
        std::cerr << "music_elf_cli failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
