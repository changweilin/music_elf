#include "music_elf/c_api.h"

#include "music_elf/audio_io.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
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

    detector = nullptr;
    require(music_elf_pitch_detector_create(&config, &detector) == 0, "C detector recreate");
    MusicElfPitchEstimate one{};
    std::size_t total = 0;
    written = 0;
    require(music_elf_pitch_detector_process(
                detector,
                samples.data(),
                samples.size(),
                &one,
                1,
                &written) == 0,
            "C detector small output process");
    total += written;
    do {
        written = 0;
        require(music_elf_pitch_detector_process(
                    detector,
                    nullptr,
                    0,
                    &one,
                    1,
                    &written) == 0,
                "C detector drain pending");
        total += written;
    } while (written > 0);
    music_elf_pitch_detector_destroy(detector);
    require(total > 100, "C detector should preserve pending frames");
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

void test_c_pipeline_summary_and_exports() {
    const std::string wav_path = "c_api_pipeline_input.wav";
    const std::string midi_path = "c_api_pipeline_output.mid";
    const std::string musicxml_path = "c_api_pipeline_output.musicxml";
    const std::string vocal_band_path = "c_api_pipeline_vocal_band.wav";
    std::remove(vocal_band_path.c_str());
    const auto audio = music_elf::make_mono_audio(48000, make_sine(48000, 1.0, 440.0));
    music_elf::write_wav_file(wav_path, audio);

    MusicElfPipelineSummary analysis{};
    require(music_elf_analyze_wav(wav_path.c_str(), &analysis) == 0, "C analyze WAV");
    require(analysis.sample_rate == 48000, "C summary sample rate");
    require(analysis.duration_seconds > 0.9 && analysis.duration_seconds < 1.1, "C summary duration");
    require(analysis.pitch_frame_count > 100, "C summary pitch frames");
    require(analysis.note_count >= 1, "C summary notes");

    MusicElfPipelineSummaryV2 analysis_v2{};
    require(music_elf_analyze_wav_v2(wav_path.c_str(), &analysis_v2) == 0, "C analyze WAV v2");
    require(analysis_v2.base.sample_rate == 48000, "C v2 summary sample rate");
    require(analysis_v2.vocal_band_sample_count >= audio.samples.size(), "C v2 summary vocal-band samples");

    MusicElfPipelineSummary export_summary{};
    require(music_elf_process_wav_to_outputs(
                wav_path.c_str(),
                midi_path.c_str(),
                musicxml_path.c_str(),
                &export_summary) == 0,
            "C pipeline exports");
    require(export_summary.midi_byte_count > 30, "C export MIDI byte count");
    require(export_summary.musicxml_char_count > 100, "C export MusicXML char count");
    require(file_contains(midi_path, "MThd"), "C export MIDI header");
    require(file_contains(musicxml_path, "<score-partwise"), "C export MusicXML root");

    MusicElfPipelineSummaryV2 vocal_band_summary{};
    require(music_elf_process_wav_to_vocal_band_v2(
                wav_path.c_str(),
                vocal_band_path.c_str(),
                &vocal_band_summary) == 0,
            "C vocal-band WAV export");
    require(vocal_band_summary.vocal_band_sample_count >= audio.samples.size(),
            "C vocal-band summary sample count");
    require(file_contains(vocal_band_path, "RIFF"), "C vocal-band WAV header");

    std::remove(wav_path.c_str());
    std::remove(midi_path.c_str());
    std::remove(musicxml_path.c_str());
    std::remove(vocal_band_path.c_str());
}

}  // namespace

int main() {
    try {
        test_c_pitch_detector();
        test_c_wav_to_midi();
        test_c_pipeline_summary_and_exports();
    } catch (const std::exception& error) {
        std::cerr << "c_api_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "c_api_tests passed\n";
    return EXIT_SUCCESS;
}
