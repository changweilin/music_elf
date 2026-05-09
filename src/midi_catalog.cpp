#include "music_elf/midi_catalog.hpp"

#include "music_elf/midi_writer.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace music_elf {
namespace {

int clamp_int(int value, int low, int high) {
    return std::max(low, std::min(high, value));
}

int pitch_class_to_midi(int pitch_class, int octave) {
    const int pc = ((pitch_class % 12) + 12) % 12;
    return (octave + 1) * 12 + pc;
}

std::string slugify(const std::string& value) {
    std::string slug;
    slug.reserve(value.size());
    for (char ch : value) {
        const auto unsigned_ch = static_cast<unsigned char>(ch);
        if (std::isalnum(unsigned_ch)) {
            slug.push_back(static_cast<char>(std::tolower(unsigned_ch)));
        } else if (!slug.empty() && slug.back() != '_') {
            slug.push_back('_');
        }
    }
    while (!slug.empty() && slug.back() == '_') {
        slug.pop_back();
    }
    return slug.empty() ? "item" : slug;
}

}  // namespace

const std::vector<GmInstrument>& gm_instruments() {
    static const std::vector<GmInstrument> instruments = {
        {0, "Acoustic Grand Piano"}, {1, "Bright Acoustic Piano"}, {2, "Electric Grand Piano"},
        {3, "Honky-tonk Piano"}, {4, "Electric Piano 1"}, {5, "Electric Piano 2"},
        {6, "Harpsichord"}, {7, "Clavinet"}, {8, "Celesta"}, {9, "Glockenspiel"},
        {10, "Music Box"}, {11, "Vibraphone"}, {12, "Marimba"}, {13, "Xylophone"},
        {14, "Tubular Bells"}, {15, "Dulcimer"}, {16, "Drawbar Organ"}, {17, "Percussive Organ"},
        {18, "Rock Organ"}, {19, "Church Organ"}, {20, "Reed Organ"}, {21, "Accordion"},
        {22, "Harmonica"}, {23, "Tango Accordion"}, {24, "Acoustic Guitar Nylon"},
        {25, "Acoustic Guitar Steel"}, {26, "Electric Guitar Jazz"}, {27, "Electric Guitar Clean"},
        {28, "Electric Guitar Muted"}, {29, "Overdriven Guitar"}, {30, "Distortion Guitar"},
        {31, "Guitar Harmonics"}, {32, "Acoustic Bass"}, {33, "Electric Bass Finger"},
        {34, "Electric Bass Pick"}, {35, "Fretless Bass"}, {36, "Slap Bass 1"}, {37, "Slap Bass 2"},
        {38, "Synth Bass 1"}, {39, "Synth Bass 2"}, {40, "Violin"}, {41, "Viola"},
        {42, "Cello"}, {43, "Contrabass"}, {44, "Tremolo Strings"}, {45, "Pizzicato Strings"},
        {46, "Orchestral Harp"}, {47, "Timpani"}, {48, "String Ensemble 1"},
        {49, "String Ensemble 2"}, {50, "Synth Strings 1"}, {51, "Synth Strings 2"},
        {52, "Choir Aahs"}, {53, "Voice Oohs"}, {54, "Synth Choir"}, {55, "Orchestra Hit"},
        {56, "Trumpet"}, {57, "Trombone"}, {58, "Tuba"}, {59, "Muted Trumpet"},
        {60, "French Horn"}, {61, "Brass Section"}, {62, "Synth Brass 1"}, {63, "Synth Brass 2"},
        {64, "Soprano Sax"}, {65, "Alto Sax"}, {66, "Tenor Sax"}, {67, "Baritone Sax"},
        {68, "Oboe"}, {69, "English Horn"}, {70, "Bassoon"}, {71, "Clarinet"},
        {72, "Piccolo"}, {73, "Flute"}, {74, "Recorder"}, {75, "Pan Flute"},
        {76, "Blown Bottle"}, {77, "Shakuhachi"}, {78, "Whistle"}, {79, "Ocarina"},
        {80, "Lead 1 Square"}, {81, "Lead 2 Sawtooth"}, {82, "Lead 3 Calliope"},
        {83, "Lead 4 Chiff"}, {84, "Lead 5 Charang"}, {85, "Lead 6 Voice"},
        {86, "Lead 7 Fifths"}, {87, "Lead 8 Bass Lead"}, {88, "Pad 1 New Age"},
        {89, "Pad 2 Warm"}, {90, "Pad 3 Polysynth"}, {91, "Pad 4 Choir"},
        {92, "Pad 5 Bowed"}, {93, "Pad 6 Metallic"}, {94, "Pad 7 Halo"}, {95, "Pad 8 Sweep"},
        {96, "FX 1 Rain"}, {97, "FX 2 Soundtrack"}, {98, "FX 3 Crystal"},
        {99, "FX 4 Atmosphere"}, {100, "FX 5 Brightness"}, {101, "FX 6 Goblins"},
        {102, "FX 7 Echoes"}, {103, "FX 8 Sci-Fi"}, {104, "Sitar"}, {105, "Banjo"},
        {106, "Shamisen"}, {107, "Koto"}, {108, "Kalimba"}, {109, "Bag Pipe"},
        {110, "Fiddle"}, {111, "Shanai"}, {112, "Tinkle Bell"}, {113, "Agogo"},
        {114, "Steel Drums"}, {115, "Woodblock"}, {116, "Taiko Drum"}, {117, "Melodic Tom"},
        {118, "Synth Drum"}, {119, "Reverse Cymbal"}, {120, "Guitar Fret Noise"},
        {121, "Breath Noise"}, {122, "Seashore"}, {123, "Bird Tweet"}, {124, "Telephone Ring"},
        {125, "Helicopter"}, {126, "Applause"}, {127, "Gunshot"},
    };
    return instruments;
}

const std::vector<CatalogRoot>& catalog_roots() {
    static const std::vector<CatalogRoot> roots = {
        {0, "C"},  {1, "Db"}, {2, "D"},  {3, "Eb"}, {4, "E"},  {5, "F"},
        {6, "Gb"}, {7, "G"},  {8, "Ab"}, {9, "A"},  {10, "Bb"}, {11, "B"},
    };
    return roots;
}

const std::vector<CatalogChordType>& catalog_chord_types() {
    static const std::vector<CatalogChordType> chords = {
        {"major", "Major", {0, 4, 7}},
        {"minor", "Minor", {0, 3, 7}},
        {"dim", "Diminished", {0, 3, 6}},
        {"aug", "Augmented", {0, 4, 8}},
        {"sus2", "Suspended 2", {0, 2, 7}},
        {"sus4", "Suspended 4", {0, 5, 7}},
        {"power", "Power 5", {0, 7}},
        {"maj7", "Major 7", {0, 4, 7, 11}},
        {"min7", "Minor 7", {0, 3, 7, 10}},
        {"dom7", "Dominant 7", {0, 4, 7, 10}},
        {"dim7", "Diminished 7", {0, 3, 6, 9}},
        {"half_dim7", "Half Diminished 7", {0, 3, 6, 10}},
        {"min_maj7", "Minor Major 7", {0, 3, 7, 11}},
        {"six", "Six", {0, 4, 7, 9}},
        {"min6", "Minor 6", {0, 3, 7, 9}},
        {"add9", "Add 9", {0, 4, 7, 14}},
        {"min_add9", "Minor Add 9", {0, 3, 7, 14}},
        {"nine", "Dominant 9", {0, 4, 7, 10, 14}},
        {"maj9", "Major 9", {0, 4, 7, 11, 14}},
        {"min9", "Minor 9", {0, 3, 7, 10, 14}},
    };
    return chords;
}

std::vector<std::uint8_t> make_catalog_chord_midi(
    const GmInstrument& instrument,
    const CatalogRoot& root,
    const CatalogChordType& chord,
    const MidiCatalogConfig& config) {
    if (instrument.program < 0 || instrument.program > 127) {
        throw std::invalid_argument("GM program must be in [0, 127]");
    }
    if (chord.intervals.empty()) {
        throw std::invalid_argument("chord must contain at least one interval");
    }
    if (config.duration_seconds <= 0.0 || config.bpm <= 0.0) {
        throw std::invalid_argument("duration and bpm must be positive");
    }

    const int root_midi = pitch_class_to_midi(root.pitch_class, config.root_octave);
    std::vector<GeneratedNote> notes;
    notes.reserve(chord.intervals.size());
    for (int interval : chord.intervals) {
        GeneratedNote note;
        note.start_seconds = 0.0;
        note.end_seconds = config.duration_seconds;
        note.midi_note = clamp_int(root_midi + interval, 0, 127);
        note.velocity = clamp_int(config.velocity, 1, 127);
        note.channel = clamp_int(config.channel, 0, 15);
        notes.push_back(note);
    }

    MidiWriterConfig midi;
    midi.bpm = config.bpm;
    midi.melody_channel = clamp_int(config.channel, 0, 15);
    midi.melody_velocity = clamp_int(config.velocity, 1, 127);
    midi.program = instrument.program;
    return write_midi(notes.data(), notes.size(), midi);
}

std::string midi_catalog_file_name(
    const GmInstrument& instrument,
    const CatalogRoot& root,
    const CatalogChordType& chord) {
    const std::string program = instrument.program < 10
                                    ? "00" + std::to_string(instrument.program)
                                    : instrument.program < 100
                                          ? "0" + std::to_string(instrument.program)
                                          : std::to_string(instrument.program);
    return program + "_" + slugify(instrument.name) + "_" + slugify(root.name) + "_" +
           slugify(chord.id) + ".mid";
}

}  // namespace music_elf

