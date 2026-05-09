#include "music_elf/core_pipeline.hpp"

#include <algorithm>
#include <cmath>
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

double midi_to_frequency(int midi_note) {
    return 440.0 * std::pow(2.0, (static_cast<double>(midi_note) - 69.0) / 12.0);
}

music_elf::AudioBuffer make_test_melody() {
    const int sample_rate = 48000;
    const std::vector<int> melody = {60, 64, 67, 72, 67, 64, 60, 0};
    std::vector<float> samples;
    double phase = 0.0;
    for (int midi : melody) {
        const std::size_t count = static_cast<std::size_t>(0.35 * sample_rate);
        const double frequency = midi > 0 ? midi_to_frequency(midi) : 0.0;
        for (std::size_t i = 0; i < count; ++i) {
            if (midi > 0) {
                samples.push_back(static_cast<float>(0.5 * std::sin(phase)));
                phase += 2.0 * kPi * frequency / static_cast<double>(sample_rate);
            } else {
                samples.push_back(0.0f);
            }
        }
    }
    return music_elf::make_mono_audio(sample_rate, std::move(samples));
}

void test_pipeline_from_wav_to_exports() {
    const std::string wav_path = "pipeline_input.wav";
    const auto audio = make_test_melody();
    music_elf::write_wav_file(wav_path, audio);
    const auto loaded = music_elf::read_wav_file(wav_path);
    std::remove(wav_path.c_str());

    const std::vector<music_elf::LyricToken> lyrics = {{"I"}, {"can"}, {"sing"}};
    music_elf::CorePipelineConfig config;
    config.accompaniment.pattern = music_elf::AccompanimentPattern::Arpeggio;
    const auto result = music_elf::run_core_pipeline(loaded, lyrics, config);

    require(result.pitch_estimates.size() > 100, "pipeline should produce pitch frames");
    require(result.notes.size() >= 5, "pipeline should produce notes");
    require(!result.dynamics.empty(), "pipeline should produce dynamics");
    require(!result.chord_progressions.empty(), "pipeline should produce chords");
    require(!result.accompaniment_notes.empty(), "pipeline should produce accompaniment");
    require(!result.lyric_alignments.empty(), "pipeline should align lyrics");
    require(result.midi_bytes.size() > 30, "pipeline should produce MIDI bytes");
    require(result.musicxml.find("<score-partwise") != std::string::npos, "pipeline should produce MusicXML");
    require(result.midi_bytes[0] == 'M' && result.midi_bytes[1] == 'T', "MIDI header");
}

}  // namespace

int main() {
    try {
        test_pipeline_from_wav_to_exports();
    } catch (const std::exception& error) {
        std::cerr << "pipeline_integration_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "pipeline_integration_tests passed\n";
    return EXIT_SUCCESS;
}

