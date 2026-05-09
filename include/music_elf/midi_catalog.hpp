#ifndef MUSIC_ELF_MIDI_CATALOG_HPP
#define MUSIC_ELF_MIDI_CATALOG_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace music_elf {

struct GmInstrument {
    int program = 0;
    std::string name;
};

struct CatalogRoot {
    int pitch_class = 0;
    std::string name;
};

struct CatalogChordType {
    std::string id;
    std::string display_name;
    std::vector<int> intervals;
};

struct MidiCatalogConfig {
    int root_octave = 4;
    double duration_seconds = 2.0;
    double bpm = 120.0;
    int velocity = 88;
    int channel = 0;
};

const std::vector<GmInstrument>& gm_instruments();
const std::vector<CatalogRoot>& catalog_roots();
const std::vector<CatalogChordType>& catalog_chord_types();

std::vector<std::uint8_t> make_catalog_chord_midi(
    const GmInstrument& instrument,
    const CatalogRoot& root,
    const CatalogChordType& chord,
    const MidiCatalogConfig& config = MidiCatalogConfig{});

std::string midi_catalog_file_name(
    const GmInstrument& instrument,
    const CatalogRoot& root,
    const CatalogChordType& chord);

}  // namespace music_elf

#endif  // MUSIC_ELF_MIDI_CATALOG_HPP

