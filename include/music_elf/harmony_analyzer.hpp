#ifndef MUSIC_ELF_HARMONY_ANALYZER_HPP
#define MUSIC_ELF_HARMONY_ANALYZER_HPP

#include "music_elf/note_segmenter.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace music_elf {

enum class ScaleMode {
    Major,
    Minor,
};

enum class ChordQuality {
    Major,
    Minor,
    Diminished,
    Major7,
    Minor7,
    Dominant7,
};

enum class HarmonyStyle {
    Pop,
    Ballad,
    Jazz,
    Cinematic,
    Classical,
};

struct KeyEstimate {
    int tonic_pitch_class = 0;
    ScaleMode mode = ScaleMode::Major;
    float confidence = 0.0f;
};

struct Chord {
    double start_seconds = 0.0;
    double end_seconds = 0.0;
    int root_pitch_class = 0;
    ChordQuality quality = ChordQuality::Major;
};

struct ChordProgression {
    HarmonyStyle style = HarmonyStyle::Pop;
    KeyEstimate key;
    float score = 0.0f;
    std::vector<Chord> chords;
};

struct HarmonyAnalyzerConfig {
    double chord_duration_seconds = 2.0;
    std::size_t max_progressions = 5;
};

class HarmonyAnalyzer {
public:
    explicit HarmonyAnalyzer(const HarmonyAnalyzerConfig& config = HarmonyAnalyzerConfig{});

    const HarmonyAnalyzerConfig& config() const noexcept;

    KeyEstimate detect_key(const NoteEvent* notes, std::size_t count) const;

    std::vector<ChordProgression> generate_chord_progressions(
        const NoteEvent* notes,
        std::size_t count) const;

private:
    ChordProgression build_progression(
        HarmonyStyle style,
        const KeyEstimate& key,
        const NoteEvent* notes,
        std::size_t count) const;

    float score_progression(
        const ChordProgression& progression,
        const NoteEvent* notes,
        std::size_t count) const;

    HarmonyAnalyzerConfig config_;
};

const char* note_name(int pitch_class) noexcept;
const char* scale_mode_name(ScaleMode mode) noexcept;
const char* harmony_style_name(HarmonyStyle style) noexcept;
std::string chord_symbol(const Chord& chord);

}  // namespace music_elf

#endif  // MUSIC_ELF_HARMONY_ANALYZER_HPP

