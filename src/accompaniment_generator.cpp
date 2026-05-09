#include "music_elf/accompaniment_generator.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace music_elf {
namespace {

int clamp_midi(int note) {
    return std::max(0, std::min(127, note));
}

int clamp_velocity(int velocity) {
    return std::max(1, std::min(127, velocity));
}

int clamp_channel(int channel) {
    return std::max(0, std::min(15, channel));
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

int fit_into_range(int midi, int min_midi, int max_midi) {
    while (midi < min_midi) {
        midi += 12;
    }
    while (midi > max_midi) {
        midi -= 12;
    }
    return clamp_midi(midi);
}

int nearest_pitch_class_to_target(int midi, int target, int min_midi, int max_midi) {
    int best = fit_into_range(midi, min_midi, max_midi);
    int best_distance = std::abs(best - target);
    for (int candidate = midi - 60; candidate <= midi + 60; candidate += 12) {
        if (candidate < min_midi || candidate > max_midi) {
            continue;
        }
        const int distance = std::abs(candidate - target);
        if (distance < best_distance) {
            best = candidate;
            best_distance = distance;
        }
    }
    return clamp_midi(best);
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
    if (config_.bass_channel < 0 || config_.bass_channel > 15) {
        throw std::invalid_argument("bass_channel must be in [0, 15]");
    }
    if (config_.min_midi_note < 0 || config_.max_midi_note > 127 ||
        config_.min_midi_note > config_.max_midi_note) {
        throw std::invalid_argument("MIDI note range is invalid");
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
    std::vector<int> previous_voicing;
    for (std::size_t i = 0; i < count; ++i) {
        std::vector<int> midis = chord_midis(
            chords[i],
            config_.smooth_voice_leading && !previous_voicing.empty() ? &previous_voicing : nullptr);
        switch (config_.pattern) {
            case AccompanimentPattern::BlockChord:
            case AccompanimentPattern::Pad:
                add_block_or_pad(chords[i], midis, notes);
                break;
            case AccompanimentPattern::Arpeggio:
                add_arpeggio(chords[i], midis, notes);
                break;
            case AccompanimentPattern::BrokenChord:
                add_broken_chord(chords[i], midis, notes);
                break;
            case AccompanimentPattern::LeftHandBassRightHandChord:
                add_left_hand_bass_right_hand_chord(chords[i], midis, notes);
                break;
        }
        previous_voicing = midis;
    }
    return notes;
}

std::vector<int> AccompanimentGenerator::chord_midis(
    const Chord& chord,
    const std::vector<int>* previous_voicing) const {
    const int root = pitch_class_to_midi(chord.root_pitch_class, config_.root_octave);
    std::vector<int> notes;
    for (int interval : quality_intervals(chord.quality)) {
        notes.push_back(fit_into_range(root + interval, config_.min_midi_note, config_.max_midi_note));
    }
    std::sort(notes.begin(), notes.end());

    for (int i = 0; i < config_.inversion && !notes.empty(); ++i) {
        int moved = notes.front() + 12;
        notes.erase(notes.begin());
        notes.push_back(fit_into_range(moved, config_.min_midi_note, config_.max_midi_note));
        std::sort(notes.begin(), notes.end());
    }

    if (previous_voicing != nullptr) {
        for (std::size_t i = 0; i < notes.size() && i < previous_voicing->size(); ++i) {
            notes[i] = nearest_pitch_class_to_target(
                notes[i],
                (*previous_voicing)[i],
                config_.min_midi_note,
                config_.max_midi_note);
        }
        std::sort(notes.begin(), notes.end());
    }
    return notes;
}

int AccompanimentGenerator::bass_midi(const Chord& chord) const {
    return clamp_midi(pitch_class_to_midi(chord.root_pitch_class, config_.bass_octave));
}

void AccompanimentGenerator::add_block_or_pad(
    const Chord& chord,
    const std::vector<int>& midis,
    std::vector<GeneratedNote>& out) const {
    const int velocity = config_.pattern == AccompanimentPattern::Pad
                             ? clamp_velocity(config_.velocity - 12)
                             : clamp_velocity(config_.velocity);
    for (int midi : midis) {
        GeneratedNote note;
        note.start_seconds = chord.start_seconds;
        note.end_seconds = chord.end_seconds;
        note.midi_note = midi;
        note.velocity = velocity;
        note.channel = clamp_channel(config_.channel);
        out.push_back(note);
    }
}

void AccompanimentGenerator::add_arpeggio(
    const Chord& chord,
    const std::vector<int>& midis,
    std::vector<GeneratedNote>& out) const {
    const double step = 60.0 / config_.bpm * config_.arpeggio_step_beats;
    std::size_t index = 0;
    for (double time = chord.start_seconds; time < chord.end_seconds - 0.000001; time += step) {
        GeneratedNote note;
        note.start_seconds = time;
        note.end_seconds = std::min(chord.end_seconds, time + step * 0.95);
        note.midi_note = midis[index % midis.size()];
        note.velocity = clamp_velocity(config_.velocity);
        note.channel = clamp_channel(config_.channel);
        out.push_back(note);
        index += 1;
    }
}

void AccompanimentGenerator::add_broken_chord(
    const Chord& chord,
    const std::vector<int>& source_midis,
    std::vector<GeneratedNote>& out) const {
    std::vector<int> midis = source_midis;
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
        note.channel = clamp_channel(config_.channel);
        out.push_back(note);
        index += 1;
    }
}

void AccompanimentGenerator::add_left_hand_bass_right_hand_chord(
    const Chord& chord,
    const std::vector<int>& midis,
    std::vector<GeneratedNote>& out) const {
    GeneratedNote bass;
    bass.start_seconds = chord.start_seconds;
    bass.end_seconds = chord.end_seconds;
    bass.midi_note = bass_midi(chord);
    bass.velocity = clamp_velocity(config_.bass_velocity);
    bass.channel = clamp_channel(config_.bass_channel);
    out.push_back(bass);

    add_block_or_pad(chord, midis, out);
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
        case AccompanimentPattern::LeftHandBassRightHandChord:
            return "Left-hand bass + right-hand chord";
    }
    return "Block chord";
}

}  // namespace music_elf
