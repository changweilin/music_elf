#include "music_elf/c_api.h"

#include "music_elf/audio_io.hpp"
#include "music_elf/core_pipeline.hpp"
#include "music_elf/pitch_detector.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

struct MusicElfPitchDetector {
    explicit MusicElfPitchDetector(const music_elf::PitchDetectorConfig& config)
        : detector(config) {}

    music_elf::PitchDetector detector;
};

namespace {

thread_local std::string g_last_error;

void clear_error() {
    g_last_error.clear();
}

int set_error(const std::string& message) {
    g_last_error = message;
    return -1;
}

music_elf::PitchDetectorConfig to_cpp_config(const MusicElfPitchDetectorConfig& config) {
    music_elf::PitchDetectorConfig cpp;
    cpp.sample_rate = config.sample_rate;
    cpp.min_frequency_hz = config.min_frequency_hz;
    cpp.max_frequency_hz = config.max_frequency_hz;
    cpp.frame_size = config.frame_size;
    cpp.hop_size = config.hop_size;
    cpp.yin_threshold = config.yin_threshold;
    cpp.silence_rms = config.silence_rms;
    return cpp;
}

MusicElfPitchEstimate to_c_estimate(const music_elf::PitchEstimate& estimate) {
    MusicElfPitchEstimate c{};
    c.time_seconds = estimate.time_seconds;
    c.frequency_hz = estimate.frequency_hz;
    c.midi_note = estimate.midi_note;
    c.cents = estimate.cents;
    c.confidence = estimate.confidence;
    c.voiced = estimate.voiced ? 1 : 0;
    return c;
}

}  // namespace

extern "C" {

MusicElfPitchDetectorConfig music_elf_default_pitch_detector_config(void) {
    const music_elf::PitchDetectorConfig cpp;
    MusicElfPitchDetectorConfig c{};
    c.sample_rate = cpp.sample_rate;
    c.min_frequency_hz = cpp.min_frequency_hz;
    c.max_frequency_hz = cpp.max_frequency_hz;
    c.frame_size = cpp.frame_size;
    c.hop_size = cpp.hop_size;
    c.yin_threshold = cpp.yin_threshold;
    c.silence_rms = cpp.silence_rms;
    return c;
}

int music_elf_pitch_detector_create(
    const MusicElfPitchDetectorConfig* config,
    MusicElfPitchDetector** detector) {
    try {
        clear_error();
        if (config == nullptr || detector == nullptr) {
            return set_error("config and detector output must not be null");
        }
        *detector = new MusicElfPitchDetector(to_cpp_config(*config));
        return 0;
    } catch (const std::exception& error) {
        return set_error(error.what());
    }
}

void music_elf_pitch_detector_destroy(MusicElfPitchDetector* detector) {
    delete detector;
}

int music_elf_pitch_detector_process(
    MusicElfPitchDetector* detector,
    const float* samples,
    size_t sample_count,
    MusicElfPitchEstimate* out,
    size_t out_capacity,
    size_t* written) {
    try {
        clear_error();
        if (detector == nullptr || written == nullptr) {
            return set_error("detector and written output must not be null");
        }
        if (out_capacity > 0 && out == nullptr) {
            return set_error("out must not be null when out_capacity is non-zero");
        }
        std::vector<music_elf::PitchEstimate> scratch(out_capacity);
        const std::size_t count = detector->detector.process(
            samples,
            sample_count,
            scratch.data(),
            scratch.size());
        for (std::size_t i = 0; i < count; ++i) {
            out[i] = to_c_estimate(scratch[i]);
        }
        *written = count;
        return 0;
    } catch (const std::exception& error) {
        return set_error(error.what());
    }
}

int music_elf_process_wav_to_midi(const char* input_wav_path, const char* output_midi_path) {
    try {
        clear_error();
        if (input_wav_path == nullptr || output_midi_path == nullptr) {
            return set_error("input and output paths must not be null");
        }
        const auto audio = music_elf::read_wav_file(input_wav_path);
        const auto result = music_elf::run_core_pipeline(audio);
        std::ofstream out(output_midi_path, std::ios::binary);
        if (!out) {
            return set_error(std::string("failed to create MIDI file: ") + output_midi_path);
        }
        out.write(reinterpret_cast<const char*>(result.midi_bytes.data()),
                  static_cast<std::streamsize>(result.midi_bytes.size()));
        return 0;
    } catch (const std::exception& error) {
        return set_error(error.what());
    }
}

const char* music_elf_last_error(void) {
    return g_last_error.c_str();
}

}  // extern "C"
