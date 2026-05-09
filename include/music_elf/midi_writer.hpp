#ifndef MUSIC_ELF_MIDI_WRITER_HPP
#define MUSIC_ELF_MIDI_WRITER_HPP

#include "music_elf/accompaniment_generator.hpp"
#include "music_elf/note_segmenter.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace music_elf {

struct MidiWriterConfig {
    int ticks_per_quarter = 480;
    double bpm = 120.0;
    int melody_channel = 0;
    int melody_velocity = 88;
    int program = -1;
    std::string track_name = "Music Elf";
    int time_signature_numerator = 4;
    int time_signature_denominator = 4;
    int key_signature_fifths = 0;
    bool key_signature_is_minor = false;
};

std::vector<std::uint8_t> write_midi(
    const GeneratedNote* notes,
    std::size_t count,
    const MidiWriterConfig& config = MidiWriterConfig{});

std::vector<std::uint8_t> write_midi(
    const NoteEvent* notes,
    std::size_t count,
    const MidiWriterConfig& config = MidiWriterConfig{});

}  // namespace music_elf

#endif  // MUSIC_ELF_MIDI_WRITER_HPP
