#include "music_elf/harmony_analyzer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::ChordProgression;
using music_elf::HarmonyAnalyzer;
using music_elf::HarmonyAnalyzerConfig;
using music_elf::HarmonyStyle;
using music_elf::NoteEvent;
using music_elf::ScaleMode;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

NoteEvent make_note(double start, double end, int midi_note) {
    NoteEvent note;
    note.start_seconds = start;
    note.end_seconds = end;
    note.duration_seconds = end - start;
    note.midi_note = midi_note;
    note.average_confidence = 0.95f;
    return note;
}

std::vector<NoteEvent> make_c_major_melody() {
    return {
        make_note(0.0, 0.5, 60),
        make_note(0.5, 1.0, 64),
        make_note(1.0, 1.5, 67),
        make_note(1.5, 2.0, 72),
        make_note(2.0, 2.5, 67),
        make_note(2.5, 3.0, 71),
        make_note(3.0, 3.5, 74),
        make_note(3.5, 4.0, 67),
        make_note(4.0, 4.5, 69),
        make_note(4.5, 5.0, 72),
        make_note(5.0, 5.5, 64),
        make_note(5.5, 6.0, 69),
        make_note(6.0, 6.5, 65),
        make_note(6.5, 7.0, 69),
        make_note(7.0, 7.5, 72),
        make_note(7.5, 8.0, 65),
    };
}

const ChordProgression* find_style(const std::vector<ChordProgression>& progressions, HarmonyStyle style) {
    for (const auto& progression : progressions) {
        if (progression.style == style) {
            return &progression;
        }
    }
    return nullptr;
}

void test_detects_c_major_key() {
    const auto melody = make_c_major_melody();
    HarmonyAnalyzer analyzer;
    const auto key = analyzer.detect_key(melody.data(), melody.size());
    require(key.tonic_pitch_class == 0, "key tonic should be C");
    require(key.mode == ScaleMode::Major, "key mode should be major");
    require(key.confidence >= 0.0f && key.confidence <= 1.0f, "key confidence range");
}

void test_generates_style_candidates() {
    const auto melody = make_c_major_melody();
    HarmonyAnalyzerConfig config;
    config.chord_duration_seconds = 2.0;
    config.max_progressions = 5;
    HarmonyAnalyzer analyzer(config);

    const auto progressions = analyzer.generate_chord_progressions(melody.data(), melody.size());
    require(progressions.size() == 5, "should return five progressions");

    const auto* pop = find_style(progressions, HarmonyStyle::Pop);
    require(pop != nullptr, "should include pop progression");
    require(pop->chords.size() == 4, "pop progression should have four chords");
    require(chord_symbol(pop->chords[0]) == "C", "first pop chord");
    require(chord_symbol(pop->chords[1]) == "G", "second pop chord");
    require(chord_symbol(pop->chords[2]) == "Am", "third pop chord");
    require(chord_symbol(pop->chords[3]) == "F", "fourth pop chord");
}

void test_chord_symbols() {
    const auto melody = make_c_major_melody();
    HarmonyAnalyzerConfig config;
    config.chord_duration_seconds = 2.0;
    HarmonyAnalyzer analyzer(config);
    const auto progressions = analyzer.generate_chord_progressions(melody.data(), melody.size());
    const auto* jazz = find_style(progressions, HarmonyStyle::Jazz);
    require(jazz != nullptr, "should include jazz progression");
    require(chord_symbol(jazz->chords[0]) == "Dm7", "jazz ii chord");
    require(chord_symbol(jazz->chords[1]) == "G7", "jazz V chord");
    require(chord_symbol(jazz->chords[2]) == "Cmaj7", "jazz I chord");
}

void test_empty_input_has_no_progressions() {
    HarmonyAnalyzer analyzer;
    const auto progressions = analyzer.generate_chord_progressions(nullptr, 0);
    require(progressions.empty(), "empty input should not invent chord progressions");
}

}  // namespace

int main() {
    try {
        test_detects_c_major_key();
        test_generates_style_candidates();
        test_chord_symbols();
        test_empty_input_has_no_progressions();
    } catch (const std::exception& error) {
        std::cerr << "harmony_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "harmony_tests passed\n";
    return EXIT_SUCCESS;
}
