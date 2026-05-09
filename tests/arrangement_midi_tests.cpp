#include "music_elf/accompaniment_generator.hpp"
#include "music_elf/midi_writer.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::AccompanimentGenerator;
using music_elf::AccompanimentGeneratorConfig;
using music_elf::AccompanimentPattern;
using music_elf::Chord;
using music_elf::ChordQuality;
using music_elf::GeneratedNote;
using music_elf::MidiWriterConfig;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Chord make_chord(double start, double end, int root, ChordQuality quality) {
    Chord chord;
    chord.start_seconds = start;
    chord.end_seconds = end;
    chord.root_pitch_class = root;
    chord.quality = quality;
    return chord;
}

bool contains_bytes(const std::vector<std::uint8_t>& bytes, const std::vector<std::uint8_t>& pattern) {
    return std::search(bytes.begin(), bytes.end(), pattern.begin(), pattern.end()) != bytes.end();
}

void test_block_chord_generation() {
    const Chord chord = make_chord(0.0, 2.0, 0, ChordQuality::Major);
    AccompanimentGeneratorConfig config;
    config.pattern = AccompanimentPattern::BlockChord;
    config.root_octave = 3;
    config.velocity = 70;
    AccompanimentGenerator generator(config);

    const auto notes = generator.generate(&chord, 1);
    require(notes.size() == 3, "C major block chord should have three notes");
    require(notes[0].midi_note == 48, "block root should be C3");
    require(notes[1].midi_note == 52, "block third should be E3");
    require(notes[2].midi_note == 55, "block fifth should be G3");
    require(notes[0].start_seconds == 0.0 && notes[0].end_seconds == 2.0, "block timing");
}

void test_arpeggio_generation() {
    const Chord chord = make_chord(0.0, 2.0, 0, ChordQuality::Major);
    AccompanimentGeneratorConfig config;
    config.pattern = AccompanimentPattern::Arpeggio;
    config.bpm = 120.0;
    config.arpeggio_step_beats = 0.5;
    AccompanimentGenerator generator(config);

    const auto notes = generator.generate(&chord, 1);
    require(notes.size() == 8, "2 seconds at 120 bpm eighth arpeggio should make eight notes");
    require(notes[0].midi_note == 48, "arpeggio note 1");
    require(notes[1].midi_note == 52, "arpeggio note 2");
    require(notes[2].midi_note == 55, "arpeggio note 3");
    require(notes.back().end_seconds <= 2.0, "arpeggio should stay inside chord");
}

void test_midi_writer_outputs_valid_smf() {
    const std::vector<GeneratedNote> notes = {
        {0.0, 0.5, 60, 90, 0},
        {0.5, 1.0, 64, 80, 0},
    };
    MidiWriterConfig config;
    config.bpm = 120.0;
    const auto bytes = write_midi(notes.data(), notes.size(), config);

    require(bytes.size() > 30, "MIDI should not be empty");
    require(bytes[0] == 'M' && bytes[1] == 'T' && bytes[2] == 'h' && bytes[3] == 'd', "MThd header");
    require(contains_bytes(bytes, {'M', 'T', 'r', 'k'}), "MTrk header");
    require(contains_bytes(bytes, {0xff, 0x51, 0x03}), "tempo meta event");
    require(contains_bytes(bytes, {0x90, 60, 90}), "first note on");
    require(contains_bytes(bytes, {0x80, 60, 0}), "first note off");
    require(contains_bytes(bytes, {0xff, 0x2f, 0x00}), "end of track");
}

}  // namespace

int main() {
    try {
        test_block_chord_generation();
        test_arpeggio_generation();
        test_midi_writer_outputs_valid_smf();
    } catch (const std::exception& error) {
        std::cerr << "arrangement_midi_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "arrangement_midi_tests passed\n";
    return EXIT_SUCCESS;
}

