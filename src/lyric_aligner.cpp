#include "music_elf/lyric_aligner.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

std::size_t note_index_at_time(const NoteEvent* notes, std::size_t note_count, double time_seconds) {
    for (std::size_t i = 0; i < note_count; ++i) {
        if (time_seconds >= notes[i].start_seconds && time_seconds <= notes[i].end_seconds) {
            return i;
        }
    }
    return note_count - 1;
}

}  // namespace

std::vector<LyricAlignment> LyricAligner::align(
    const LyricToken* tokens,
    std::size_t token_count,
    const NoteEvent* notes,
    std::size_t note_count) const {
    if (token_count > 0 && tokens == nullptr) {
        throw std::invalid_argument("tokens must not be null when token_count is non-zero");
    }
    if (note_count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when note_count is non-zero");
    }

    std::vector<LyricAlignment> alignments;
    if (token_count == 0 || note_count == 0) {
        return alignments;
    }

    alignments.reserve(token_count);

    if (token_count <= note_count) {
        for (std::size_t i = 0; i < token_count; ++i) {
            const std::size_t start_index = i * note_count / token_count;
            std::size_t end_index = ((i + 1) * note_count / token_count);
            end_index = end_index == 0 ? 0 : end_index - 1;
            end_index = std::max(start_index, std::min(end_index, note_count - 1));

            LyricAlignment alignment;
            alignment.text = tokens[i].text;
            alignment.note_index = start_index;
            alignment.start_seconds = notes[start_index].start_seconds;
            alignment.end_seconds = notes[end_index].end_seconds;
            alignments.push_back(alignment);
        }
        return alignments;
    }

    const double start = notes[0].start_seconds;
    const double end = notes[note_count - 1].end_seconds;
    const double duration = std::max(0.001, end - start);
    for (std::size_t i = 0; i < token_count; ++i) {
        const double token_start = start + duration * static_cast<double>(i) / static_cast<double>(token_count);
        const double token_end = start + duration * static_cast<double>(i + 1) / static_cast<double>(token_count);
        const double midpoint = (token_start + token_end) * 0.5;

        LyricAlignment alignment;
        alignment.text = tokens[i].text;
        alignment.note_index = note_index_at_time(notes, note_count, midpoint);
        alignment.start_seconds = token_start;
        alignment.end_seconds = token_end;
        alignments.push_back(alignment);
    }
    return alignments;
}

}  // namespace music_elf

