#include "music_elf/midi_catalog.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool contains_bytes(const std::vector<std::uint8_t>& bytes, const std::vector<std::uint8_t>& pattern) {
    return std::search(bytes.begin(), bytes.end(), pattern.begin(), pattern.end()) != bytes.end();
}

const music_elf::CatalogRoot& find_root(const std::string& name) {
    const auto& roots = music_elf::catalog_roots();
    const auto found = std::find_if(
        roots.begin(),
        roots.end(),
        [&name](const music_elf::CatalogRoot& root) {
            return root.name == name;
        });
    require(found != roots.end(), "root not found");
    return *found;
}

const music_elf::CatalogChordType& find_chord(const std::string& id) {
    const auto& chords = music_elf::catalog_chord_types();
    const auto found = std::find_if(
        chords.begin(),
        chords.end(),
        [&id](const music_elf::CatalogChordType& chord) {
            return chord.id == id;
        });
    require(found != chords.end(), "chord not found");
    return *found;
}

void test_catalog_tables() {
    require(music_elf::gm_instruments().size() == 128, "GM instrument count");
    require(music_elf::catalog_roots().size() == 12, "root count");
    require(music_elf::catalog_chord_types().size() >= 12, "chord type count");
    require(music_elf::gm_instruments()[0].name == "Acoustic Grand Piano", "first GM name");
    require(music_elf::gm_instruments()[40].name == "Violin", "violin GM name");
}

void test_catalog_midi_contains_program_and_chord_notes() {
    const auto& instrument = music_elf::gm_instruments()[40];
    const auto& root = find_root("C");
    const auto& chord = find_chord("major");
    music_elf::MidiCatalogConfig config;
    config.root_octave = 4;
    const auto bytes = music_elf::make_catalog_chord_midi(instrument, root, chord, config);

    require(contains_bytes(bytes, {'M', 'T', 'h', 'd'}), "MIDI header");
    require(contains_bytes(bytes, {0xc0, 40}), "violin program change");
    require(contains_bytes(bytes, {0x90, 60, 88}), "C note on");
    require(contains_bytes(bytes, {0x90, 64, 88}), "E note on");
    require(contains_bytes(bytes, {0x90, 67, 88}), "G note on");
}

void test_catalog_file_name() {
    const std::string name = music_elf::midi_catalog_file_name(
        music_elf::gm_instruments()[0],
        find_root("Db"),
        find_chord("min7"));
    require(name.find("000_acoustic_grand_piano_db_min7.mid") != std::string::npos, "catalog file name");
}

}  // namespace

int main() {
    try {
        test_catalog_tables();
        test_catalog_midi_contains_program_and_chord_notes();
        test_catalog_file_name();
    } catch (const std::exception& error) {
        std::cerr << "midi_catalog_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "midi_catalog_tests passed\n";
    return EXIT_SUCCESS;
}

