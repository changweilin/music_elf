#ifndef MUSIC_ELF_ACCOMPANIMENT_GENERATOR_HPP
#define MUSIC_ELF_ACCOMPANIMENT_GENERATOR_HPP

#include "music_elf/harmony_analyzer.hpp"

#include <vector>

namespace music_elf {

enum class AccompanimentPattern {
    BlockChord,
    Arpeggio,
    BrokenChord,
    Pad,
};

struct GeneratedNote {
    double start_seconds = 0.0;
    double end_seconds = 0.0;
    int midi_note = 60;
    int velocity = 80;
    int channel = 1;
};

struct AccompanimentGeneratorConfig {
    AccompanimentPattern pattern = AccompanimentPattern::BlockChord;
    double bpm = 120.0;
    double arpeggio_step_beats = 0.5;
    int root_octave = 3;
    int velocity = 72;
    int channel = 1;
};

class AccompanimentGenerator {
public:
    explicit AccompanimentGenerator(
        const AccompanimentGeneratorConfig& config = AccompanimentGeneratorConfig{});

    const AccompanimentGeneratorConfig& config() const noexcept;

    std::vector<GeneratedNote> generate(const Chord* chords, std::size_t count) const;

private:
    std::vector<int> chord_midis(const Chord& chord) const;
    void add_block_or_pad(const Chord& chord, std::vector<GeneratedNote>& out) const;
    void add_arpeggio(const Chord& chord, std::vector<GeneratedNote>& out) const;
    void add_broken_chord(const Chord& chord, std::vector<GeneratedNote>& out) const;

    AccompanimentGeneratorConfig config_;
};

const char* accompaniment_pattern_name(AccompanimentPattern pattern) noexcept;

}  // namespace music_elf

#endif  // MUSIC_ELF_ACCOMPANIMENT_GENERATOR_HPP

