#ifndef MUSIC_ELF_C_API_H
#define MUSIC_ELF_C_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MusicElfPitchDetector MusicElfPitchDetector;

typedef struct MusicElfPitchDetectorConfig {
    int sample_rate;
    float min_frequency_hz;
    float max_frequency_hz;
    size_t frame_size;
    size_t hop_size;
    float yin_threshold;
    float silence_rms;
} MusicElfPitchDetectorConfig;

typedef struct MusicElfPitchEstimate {
    double time_seconds;
    float frequency_hz;
    int midi_note;
    float cents;
    float confidence;
    int voiced;
} MusicElfPitchEstimate;

typedef struct MusicElfPipelineSummary {
    int sample_rate;
    int channels;
    double duration_seconds;
    size_t pitch_frame_count;
    size_t voiced_pitch_frame_count;
    size_t note_count;
    double estimated_bpm;
    int key_tonic_pitch_class;
    int key_is_minor;
    float key_confidence;
    size_t chord_progression_count;
    size_t accompaniment_note_count;
    size_t lyric_alignment_count;
    size_t midi_byte_count;
    size_t musicxml_char_count;
} MusicElfPipelineSummary;

MusicElfPitchDetectorConfig music_elf_default_pitch_detector_config(void);

int music_elf_pitch_detector_create(
    const MusicElfPitchDetectorConfig* config,
    MusicElfPitchDetector** detector);

void music_elf_pitch_detector_destroy(MusicElfPitchDetector* detector);

int music_elf_pitch_detector_process(
    MusicElfPitchDetector* detector,
    const float* samples,
    size_t sample_count,
    MusicElfPitchEstimate* out,
    size_t out_capacity,
    size_t* written);

int music_elf_process_wav_to_midi(const char* input_wav_path, const char* output_midi_path);

int music_elf_analyze_wav(const char* input_wav_path, MusicElfPipelineSummary* out_summary);

int music_elf_process_wav_to_outputs(
    const char* input_wav_path,
    const char* output_midi_path,
    const char* output_musicxml_path,
    MusicElfPipelineSummary* out_summary);

const char* music_elf_last_error(void);

#ifdef __cplusplus
}
#endif

#endif  // MUSIC_ELF_C_API_H
