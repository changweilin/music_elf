#include "music_elf/lyric_aligner.hpp"
#include "music_elf/midi_writer.hpp"
#include "music_elf/musicxml_writer.hpp"

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr std::uint64_t kExpectedMidiSnapshot = 0x7952b42832efaabeull;
constexpr std::uint64_t kExpectedMusicXmlSnapshot = 0x85f23da2e3cb0923ull;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string hex64(std::uint64_t value) {
    std::ostringstream out;
    out << "0x" << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

std::uint64_t fnv1a(const std::vector<std::uint8_t>& bytes) {
    std::uint64_t hash = 14695981039346656037ull;
    for (std::uint8_t byte : bytes) {
        hash ^= byte;
        hash *= 1099511628211ull;
    }
    return hash;
}

std::uint64_t fnv1a(const std::string& text) {
    std::uint64_t hash = 14695981039346656037ull;
    for (unsigned char byte : text) {
        hash ^= byte;
        hash *= 1099511628211ull;
    }
    return hash;
}

music_elf::NoteEvent make_note(double start, double end, int midi_note) {
    music_elf::NoteEvent note;
    note.start_seconds = start;
    note.end_seconds = end;
    note.duration_seconds = end - start;
    note.midi_note = midi_note;
    note.average_confidence = 0.98f;
    return note;
}

std::vector<music_elf::NoteEvent> make_snapshot_melody() {
    return {
        make_note(0.0, 0.5, 60),
        make_note(0.5, 1.0, 64),
        make_note(1.0, 1.5, 67),
        make_note(1.5, 2.0, 72),
    };
}

void test_midi_snapshot() {
    const std::vector<music_elf::GeneratedNote> notes = {
        {0.0, 0.5, 60, 90, 0},
        {0.5, 1.0, 64, 82, 0},
        {1.0, 1.5, 67, 78, 0},
        {1.5, 2.0, 72, 88, 0},
    };
    music_elf::MidiWriterConfig config;
    config.bpm = 120.0;
    config.program = 0;
    config.track_name = "Snapshot Lead";
    config.time_signature_numerator = 4;
    config.time_signature_denominator = 4;
    config.key_signature_fifths = 0;

    const auto bytes = music_elf::write_midi(notes.data(), notes.size(), config);
    const std::uint64_t actual = fnv1a(bytes);
    require(actual == kExpectedMidiSnapshot,
            "MIDI snapshot changed: expected " + hex64(kExpectedMidiSnapshot) + ", actual " + hex64(actual));
}

void test_musicxml_snapshot() {
    const auto notes = make_snapshot_melody();
    const std::vector<music_elf::LyricAlignment> lyrics = {
        {"La", 0, 0.0, 0.5},
        {"lu", 1, 0.5, 1.0},
        {"li", 2, 1.0, 1.5},
        {"&", 3, 1.5, 2.0},
    };
    music_elf::MusicXmlWriterConfig config;
    config.bpm = 120.0;
    config.part_name = "Snapshot Voice";

    const std::string xml = music_elf::write_musicxml(notes.data(), notes.size(), lyrics.data(), lyrics.size(), config);
    const std::uint64_t actual = fnv1a(xml);
    require(actual == kExpectedMusicXmlSnapshot,
            "MusicXML snapshot changed: expected " + hex64(kExpectedMusicXmlSnapshot) + ", actual " + hex64(actual));
}

}  // namespace

int main() {
    try {
        test_midi_snapshot();
        test_musicxml_snapshot();
    } catch (const std::exception& error) {
        std::cerr << "export_snapshot_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "export_snapshot_tests passed\n";
    return EXIT_SUCCESS;
}
