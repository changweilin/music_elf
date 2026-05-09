#ifndef MUSIC_ELF_MUSICXML_WRITER_HPP
#define MUSIC_ELF_MUSICXML_WRITER_HPP

#include "music_elf/harmony_analyzer.hpp"
#include "music_elf/lyric_aligner.hpp"
#include "music_elf/rhythm_analyzer.hpp"

#include <string>

namespace music_elf {

struct MusicXmlWriterConfig {
    int divisions_per_quarter = 480;
    double bpm = 120.0;
    std::string part_name = "Voice";
    int time_signature_numerator = 4;
    int time_signature_denominator = 4;
    int key_signature_fifths = 0;
    bool key_signature_is_minor = false;
    int minimum_note_value_denominator = 16;
    bool include_rests = true;
};

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const Chord* chords,
    std::size_t chord_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const Chord* chords,
    std::size_t chord_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const Chord* chords,
    std::size_t chord_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const Chord* chords,
    std::size_t chord_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config = MusicXmlWriterConfig{});

}  // namespace music_elf

#endif  // MUSIC_ELF_MUSICXML_WRITER_HPP
