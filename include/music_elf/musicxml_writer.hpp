#ifndef MUSIC_ELF_MUSICXML_WRITER_HPP
#define MUSIC_ELF_MUSICXML_WRITER_HPP

#include "music_elf/lyric_aligner.hpp"

#include <string>

namespace music_elf {

struct MusicXmlWriterConfig {
    int divisions_per_quarter = 480;
    double bpm = 120.0;
    std::string part_name = "Voice";
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

}  // namespace music_elf

#endif  // MUSIC_ELF_MUSICXML_WRITER_HPP

