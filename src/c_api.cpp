#include "music_elf/c_api.h"

#include "music_elf/audio_io.hpp"
#include "music_elf/core_pipeline.hpp"
#include "music_elf/pitch_detector.hpp"

#include <algorithm>
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

MusicElfPipelineSummary to_c_summary(
    const music_elf::AudioBuffer& audio,
    const music_elf::CorePipelineResult& result) {
    MusicElfPipelineSummary summary{};
    summary.sample_rate = audio.sample_rate;
    summary.channels = audio.channels;
    const std::size_t frames = audio.channels > 0
                                   ? audio.samples.size() / static_cast<std::size_t>(audio.channels)
                                   : 0;
    summary.duration_seconds = audio.sample_rate > 0
                                   ? static_cast<double>(frames) / static_cast<double>(audio.sample_rate)
                                   : 0.0;
    summary.pitch_frame_count = result.pitch_estimates.size();
    summary.voiced_pitch_frame_count = static_cast<std::size_t>(std::count_if(
        result.pitch_estimates.begin(),
        result.pitch_estimates.end(),
        [](const music_elf::PitchEstimate& estimate) {
            return estimate.voiced;
        }));
    summary.note_count = result.notes.size();
    summary.estimated_bpm = result.rhythm.beat_grid.bpm;
    summary.key_tonic_pitch_class = result.key.tonic_pitch_class;
    summary.key_is_minor = result.key.mode == music_elf::ScaleMode::Minor ? 1 : 0;
    summary.key_confidence = result.key.confidence;
    summary.chord_progression_count = result.chord_progressions.size();
    summary.accompaniment_note_count = result.accompaniment_notes.size();
    summary.lyric_alignment_count = result.lyric_alignments.size();
    summary.midi_byte_count = result.midi_bytes.size();
    summary.musicxml_char_count = result.musicxml.size();
    return summary;
}

int write_binary_file(const char* path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return set_error(std::string("failed to create file: ") + path);
    }
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return 0;
}

int write_text_file(const char* path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return set_error(std::string("failed to create file: ") + path);
    }
    out << text;
    return 0;
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
    return music_elf_process_wav_to_outputs(input_wav_path, output_midi_path, nullptr, nullptr);
}

int music_elf_analyze_wav(const char* input_wav_path, MusicElfPipelineSummary* out_summary) {
    try {
        clear_error();
        if (input_wav_path == nullptr || out_summary == nullptr) {
            return set_error("input path and summary output must not be null");
        }
        const auto audio = music_elf::read_wav_file(input_wav_path);
        const auto result = music_elf::run_core_pipeline(audio);
        *out_summary = to_c_summary(audio, result);
        return 0;
    } catch (const std::exception& error) {
        return set_error(error.what());
    }
}

int music_elf_process_wav_to_outputs(
    const char* input_wav_path,
    const char* output_midi_path,
    const char* output_musicxml_path,
    MusicElfPipelineSummary* out_summary) {
    try {
        clear_error();
        if (input_wav_path == nullptr) {
            return set_error("input path must not be null");
        }
        if (output_midi_path == nullptr && output_musicxml_path == nullptr && out_summary == nullptr) {
            return set_error("at least one output path or summary output must be provided");
        }

        const auto audio = music_elf::read_wav_file(input_wav_path);
        const auto result = music_elf::run_core_pipeline(audio);
        if (output_midi_path != nullptr && write_binary_file(output_midi_path, result.midi_bytes) != 0) {
            return -1;
        }
        if (output_musicxml_path != nullptr && write_text_file(output_musicxml_path, result.musicxml) != 0) {
            return -1;
        }
        if (out_summary != nullptr) {
            *out_summary = to_c_summary(audio, result);
        }
        return 0;
    } catch (const std::exception& error) {
        return set_error(error.what());
    }
}

const char* music_elf_last_error(void) {
    return g_last_error.c_str();
}

}  // extern "C"
