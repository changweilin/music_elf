#include "music_elf/accompaniment_generator.hpp"

#include <algorithm>
#include <stdexcept>

namespace music_elf {
namespace {

int clamp_midi(int note) {
    return std::max(0, std::min(127, note));
}

int clamp_velocity(int velocity) {
    return std::max(1, std::min(127, velocity));
}

int pitch_class_to_midi(int pitch_class, int octave) {
    const int pc = ((pitch_class % 12) + 12) % 12;
    return (octave + 1) * 12 + pc;
}

std::vector<int> quality_intervals(ChordQuality quality) {
    switch (quality) {
        case ChordQuality::Major:
            return {0, 4, 7};
        case ChordQuality::Minor:
            return {0, 3, 7};
        case ChordQuality::Diminished:
            return {0, 3, 6};
        case ChordQuality::Major7:
            return {0, 4, 7, 11};
        case ChordQuality::Minor7:
            return {0, 3, 7, 10};
        case ChordQuality::Dominant7:
            return {0, 4, 7, 10};
    }
    return {0, 4, 7};
}

}  // namespace

AccompanimentGenerator::AccompanimentGenerator(const AccompanimentGeneratorConfig& config)
    : config_(config) {
    if (config_.bpm <= 0.0) {
        throw std::invalid_argument("bpm must be positive");
    }
    if (config_.arpeggio_step_beats <= 0.0) {
        throw std::invalid_argument("arpeggio_step_beats must be positive");
    }
    if (config_.channel < 0 || config_.channel > 15) {
        throw std::invalid_argument("channel must be in [0, 15]");
    }
}

const AccompanimentGeneratorConfig& AccompanimentGenerator::config() const noexcept {
    return config_;
}

std::vector<GeneratedNote> AccompanimentGenerator::generate(const Chord* chords, std::size_t count) const {
    if (count > 0 && chords == nullptr) {
        throw std::invalid_argument("chords must not be null when count is non-zero");
    }

    std::vector<GeneratedNote> notes;
    for (std::size_t i = 0; i < count; ++i) {
        switch (config_.pattern) {
            case AccompanimentPattern::BlockChord:
            case AccompanimentPattern::Pad:
                add_block_or_pad(chords[i], notes);
                break;
            case AccompanimentPattern::Arpeggio:
                add_arpeggio(chords[i], notes);
                break;
            case AccompanimentPattern::BrokenChord:
                add_broken_chord(chords[i], notes);
                break;
        }
    }
    return notes;
}

std::vector<int> AccompanimentGenerator::chord_midis(const Chord& chord) const {
    const int root = pitch_class_to_midi(chord.root_pitch_class, config_.root_octave);
    std::vector<int> notes;
    for (int interval : quality_intervals(chord.quality)) {
        notes.push_back(clamp_midi(root + interval));
    }
    return notes;
}

void AccompanimentGenerator::add_block_or_pad(const Chord& chord, std::vector<GeneratedNote>& out) const {
    const int velocity = config_.pattern == AccompanimentPattern::Pad
                             ? clamp_velocity(config_.velocity - 12)
                             : clamp_velocity(config_.velocity);
    for (int midi : chord_midis(chord)) {
        GeneratedNote note;
        note.start_seconds = chord.start_seconds;
        note.end_seconds = chord.end_seconds;
        note.midi_note = midi;
        note.velocity = velocity;
        note.channel = config_.channel;
        out.push_back(note);
    }
}

void AccompanimentGenerator::add_arpeggio(const Chord& chord, std::vector<GeneratedNote>& out) const {
    const std::vector<int> midis = chord_midis(chord);
    const double step = 60.0 / config_.bpm * config_.arpeggio_step_beats;
    std::size_t index = 0;
    for (double time = chord.start_seconds; time < chord.end_seconds - 0.000001; time += step) {
        GeneratedNote note;
        note.start_seconds = time;
        note.end_seconds = std::min(chord.end_seconds, time + step * 0.95);
        note.midi_note = midis[index % midis.size()];
        note.velocity = clamp_velocity(config_.velocity);
        note.channel = config_.channel;
        out.push_back(note);
        index += 1;
    }
}

void AccompanimentGenerator::add_broken_chord(const Chord& chord, std::vector<GeneratedNote>& out) const {
    std::vector<int> midis = chord_midis(chord);
    if (midis.size() >= 3) {
        std::swap(midis[1], midis[2]);
    }
    const double step = 60.0 / config_.bpm * config_.arpeggio_step_beats;
    std::size_t index = 0;
    for (double time = chord.start_seconds; time < chord.end_seconds - 0.000001; time += step) {
        GeneratedNote note;
        note.start_seconds = time;
        note.end_seconds = std::min(chord.end_seconds, time + step * 0.90);
        note.midi_note = midis[index % midis.size()];
        note.velocity = clamp_velocity(config_.velocity);
        note.channel = config_.channel;
        out.push_back(note);
        index += 1;
    }
}

const char* accompaniment_pattern_name(AccompanimentPattern pattern) noexcept {
    switch (pattern) {
        case AccompanimentPattern::BlockChord:
            return "Block chord";
        case AccompanimentPattern::Arpeggio:
            return "Arpeggio";
        case AccompanimentPattern::BrokenChord:
            return "Broken chord";
        case AccompanimentPattern::Pad:
            return "Pad";
    }
    return "Block chord";
}

}  // namespace music_elf

