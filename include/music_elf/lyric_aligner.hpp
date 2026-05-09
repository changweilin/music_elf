#ifndef MUSIC_ELF_LYRIC_ALIGNER_HPP
#define MUSIC_ELF_LYRIC_ALIGNER_HPP

#include "music_elf/note_segmenter.hpp"

#include <string>
#include <vector>

namespace music_elf {

struct LyricToken {
    std::string text;
};

struct LyricAlignment {
    std::string text;
    std::size_t note_index = 0;
    double start_seconds = 0.0;
    double end_seconds = 0.0;
};

class LyricAligner {
public:
    std::vector<LyricAlignment> align(
        const LyricToken* tokens,
        std::size_t token_count,
        const NoteEvent* notes,
        std::size_t note_count) const;
};

}  // namespace music_elf

#endif  // MUSIC_ELF_LYRIC_ALIGNER_HPP

