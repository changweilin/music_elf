#ifndef MUSIC_ELF_CORE_PIPELINE_HPP
#define MUSIC_ELF_CORE_PIPELINE_HPP

#include "music_elf/accompaniment_generator.hpp"
#include "music_elf/audio_io.hpp"
#include "music_elf/audio_renderer.hpp"
#include "music_elf/dynamics_analyzer.hpp"
#include "music_elf/harmony_analyzer.hpp"
#include "music_elf/lyric_aligner.hpp"
#include "music_elf/midi_writer.hpp"
#include "music_elf/musicxml_writer.hpp"
#include "music_elf/note_segmenter.hpp"
#include "music_elf/pitch_detector.hpp"
#include "music_elf/rhythm_analyzer.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace music_elf {

struct CorePipelineConfig {
    PitchDetectorConfig pitch;
    NoteSegmenterConfig note_segmenter;
    RhythmAnalyzerConfig rhythm;
    DynamicsAnalyzerConfig dynamics;
    HarmonyAnalyzerConfig harmony;
    AccompanimentGeneratorConfig accompaniment;
    AudioRendererConfig renderer;
    AudioMixConfig vocal_band_mix;
    MidiWriterConfig midi;
    MusicXmlWriterConfig musicxml;
    std::size_t processing_chunk_samples = 512;
    bool render_preview_audio = false;
};

struct CorePipelineResult {
    std::vector<PitchEstimate> pitch_estimates;
    std::vector<NoteEvent> notes;
    RhythmAnalysis rhythm;
    std::vector<NoteDynamics> dynamics;
    KeyEstimate key;
    std::vector<ChordProgression> chord_progressions;
    std::vector<GeneratedNote> accompaniment_notes;
    std::vector<LyricAlignment> lyric_alignments;
    std::vector<std::uint8_t> midi_bytes;
    std::string musicxml;
    AudioBuffer instrumental_audio;
    AudioBuffer vocal_band_audio;
};

CorePipelineResult run_core_pipeline(
    const AudioBuffer& audio,
    const std::vector<LyricToken>& lyric_tokens = {},
    const CorePipelineConfig& config = CorePipelineConfig{});

}  // namespace music_elf

#endif  // MUSIC_ELF_CORE_PIPELINE_HPP
