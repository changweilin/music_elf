#ifndef MUSIC_ELF_DYNAMICS_ANALYZER_HPP
#define MUSIC_ELF_DYNAMICS_ANALYZER_HPP

#include "music_elf/note_segmenter.hpp"

#include <cstddef>
#include <vector>

namespace music_elf {

enum class DynamicMark {
    Piano,
    MezzoPiano,
    MezzoForte,
    Forte,
    Fortissimo,
};

struct DynamicsAnalyzerConfig {
    int sample_rate = 48000;
    float silence_floor_db = -90.0f;
};

struct NoteDynamics {
    NoteEvent note;
    float rms_db = -90.0f;
    float peak_db = -90.0f;
    int velocity = 1;
    DynamicMark dynamic_mark = DynamicMark::MezzoForte;
};

class DynamicsAnalyzer {
public:
    explicit DynamicsAnalyzer(const DynamicsAnalyzerConfig& config = DynamicsAnalyzerConfig{});

    const DynamicsAnalyzerConfig& config() const noexcept;

    std::vector<NoteDynamics> analyze(
        const float* samples,
        std::size_t sample_count,
        const NoteEvent* notes,
        std::size_t note_count) const;

private:
    DynamicsAnalyzerConfig config_;
};

const char* dynamic_mark_name(DynamicMark mark) noexcept;

}  // namespace music_elf

#endif  // MUSIC_ELF_DYNAMICS_ANALYZER_HPP

