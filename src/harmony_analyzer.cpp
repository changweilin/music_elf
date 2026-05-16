#include "music_elf/harmony_analyzer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace music_elf {
namespace {

using Profile = std::array<double, 12>;

constexpr Profile kMajorProfile = {
    6.35, 2.23, 3.48, 2.33, 4.38, 4.09,
    2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
constexpr Profile kMinorProfile = {
    6.33, 2.68, 3.52, 5.38, 2.60, 3.53,
    2.54, 4.75, 3.98, 2.69, 3.34, 3.17};

int pitch_class(int midi_note) {
    const int pc = midi_note % 12;
    return pc < 0 ? pc + 12 : pc;
}

int transpose_pc(int root, int interval) {
    return (root + interval + 120) % 12;
}

std::vector<int> chord_tones(const Chord& chord) {
    switch (chord.quality) {
        case ChordQuality::Major:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 4),
                    transpose_pc(chord.root_pitch_class, 7)};
        case ChordQuality::Minor:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 3),
                    transpose_pc(chord.root_pitch_class, 7)};
        case ChordQuality::Diminished:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 3),
                    transpose_pc(chord.root_pitch_class, 6)};
        case ChordQuality::Major7:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 4),
                    transpose_pc(chord.root_pitch_class, 7), transpose_pc(chord.root_pitch_class, 11)};
        case ChordQuality::Minor7:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 3),
                    transpose_pc(chord.root_pitch_class, 7), transpose_pc(chord.root_pitch_class, 10)};
        case ChordQuality::Dominant7:
            return {chord.root_pitch_class, transpose_pc(chord.root_pitch_class, 4),
                    transpose_pc(chord.root_pitch_class, 7), transpose_pc(chord.root_pitch_class, 10)};
    }
    return {};
}

bool contains_pitch_class(const std::vector<int>& pitch_classes, int target) {
    return std::find(pitch_classes.begin(), pitch_classes.end(), target) != pitch_classes.end();
}

double profile_score(const std::array<double, 12>& histogram, const Profile& profile, int tonic) {
    double score = 0.0;
    for (int pc = 0; pc < 12; ++pc) {
        const int profile_index = (pc - tonic + 12) % 12;
        score += histogram[static_cast<std::size_t>(pc)] * profile[static_cast<std::size_t>(profile_index)];
    }
    return score;
}

Chord make_chord(double start, double end, int root, ChordQuality quality) {
    Chord chord;
    chord.start_seconds = start;
    chord.end_seconds = end;
    chord.root_pitch_class = pitch_class(root);
    chord.quality = quality;
    return chord;
}

struct TemplateChord {
    int interval_from_tonic = 0;
    ChordQuality quality = ChordQuality::Major;
};

std::vector<TemplateChord> major_template(HarmonyStyle style) {
    switch (style) {
        case HarmonyStyle::Pop:
            return {{0, ChordQuality::Major}, {7, ChordQuality::Major}, {9, ChordQuality::Minor},
                    {5, ChordQuality::Major}};
        case HarmonyStyle::Ballad:
            return {{0, ChordQuality::Major}, {9, ChordQuality::Minor}, {5, ChordQuality::Major},
                    {7, ChordQuality::Major}};
        case HarmonyStyle::Jazz:
            return {{2, ChordQuality::Minor7}, {7, ChordQuality::Dominant7}, {0, ChordQuality::Major7},
                    {9, ChordQuality::Minor7}};
        case HarmonyStyle::Cinematic:
            return {{0, ChordQuality::Major}, {8, ChordQuality::Major}, {5, ChordQuality::Major},
                    {7, ChordQuality::Major}};
        case HarmonyStyle::Classical:
            return {{0, ChordQuality::Major}, {5, ChordQuality::Major}, {7, ChordQuality::Major},
                    {0, ChordQuality::Major}};
    }
    return {};
}

std::vector<TemplateChord> minor_template(HarmonyStyle style) {
    switch (style) {
        case HarmonyStyle::Pop:
            return {{0, ChordQuality::Minor}, {8, ChordQuality::Major}, {3, ChordQuality::Major},
                    {10, ChordQuality::Major}};
        case HarmonyStyle::Ballad:
            return {{0, ChordQuality::Minor}, {5, ChordQuality::Minor}, {8, ChordQuality::Major},
                    {7, ChordQuality::Major}};
        case HarmonyStyle::Jazz:
            return {{2, ChordQuality::Diminished}, {7, ChordQuality::Dominant7}, {0, ChordQuality::Minor7},
                    {5, ChordQuality::Minor7}};
        case HarmonyStyle::Cinematic:
            return {{0, ChordQuality::Minor}, {8, ChordQuality::Major}, {3, ChordQuality::Major},
                    {10, ChordQuality::Major}};
        case HarmonyStyle::Classical:
            return {{0, ChordQuality::Minor}, {5, ChordQuality::Minor}, {7, ChordQuality::Major},
                    {0, ChordQuality::Minor}};
    }
    return {};
}

}  // namespace

HarmonyAnalyzer::HarmonyAnalyzer(const HarmonyAnalyzerConfig& config) : config_(config) {
    if (config_.chord_duration_seconds <= 0.0) {
        throw std::invalid_argument("chord_duration_seconds must be positive");
    }
    if (config_.max_progressions == 0) {
        throw std::invalid_argument("max_progressions must be positive");
    }
}

const HarmonyAnalyzerConfig& HarmonyAnalyzer::config() const noexcept {
    return config_;
}

KeyEstimate HarmonyAnalyzer::detect_key(const NoteEvent* notes, std::size_t count) const {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }

    std::array<double, 12> histogram{};
    for (std::size_t i = 0; i < count; ++i) {
        if (notes[i].midi_note <= 0 || notes[i].duration_seconds <= 0.0) {
            continue;
        }
        histogram[static_cast<std::size_t>(pitch_class(notes[i].midi_note))] += notes[i].duration_seconds;
    }

    const double total = std::accumulate(histogram.begin(), histogram.end(), 0.0);
    if (total <= 0.0) {
        return {};
    }
    for (double& value : histogram) {
        value /= total;
    }

    KeyEstimate best;
    double best_score = -1.0;
    double second_score = -1.0;
    for (int tonic = 0; tonic < 12; ++tonic) {
        const double major = profile_score(histogram, kMajorProfile, tonic);
        const double minor = profile_score(histogram, kMinorProfile, tonic);
        const bool use_major = major >= minor;
        const double score = use_major ? major : minor;
        if (score > best_score) {
            second_score = best_score;
            best_score = score;
            best.tonic_pitch_class = tonic;
            best.mode = use_major ? ScaleMode::Major : ScaleMode::Minor;
        } else if (score > second_score) {
            second_score = score;
        }
    }

    best.confidence = static_cast<float>(
        std::max(0.0, std::min(1.0, (best_score - second_score) / std::max(1.0, best_score))));
    return best;
}

std::vector<ChordProgression> HarmonyAnalyzer::generate_chord_progressions(
    const NoteEvent* notes,
    std::size_t count) const {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }
    if (count == 0) {
        return {};
    }

    const KeyEstimate key = detect_key(notes, count);
    std::vector<ChordProgression> progressions;
    for (const HarmonyStyle style : {
             HarmonyStyle::Pop,
             HarmonyStyle::Ballad,
             HarmonyStyle::Jazz,
             HarmonyStyle::Cinematic,
             HarmonyStyle::Classical,
         }) {
        progressions.push_back(build_progression(style, key, notes, count));
    }

    std::stable_sort(
        progressions.begin(),
        progressions.end(),
        [](const ChordProgression& left, const ChordProgression& right) {
            return left.score > right.score;
        });

    if (progressions.size() > config_.max_progressions) {
        progressions.resize(config_.max_progressions);
    }
    return progressions;
}

ChordProgression HarmonyAnalyzer::build_progression(
    HarmonyStyle style,
    const KeyEstimate& key,
    const NoteEvent* notes,
    std::size_t count) const {
    const auto template_chords =
        key.mode == ScaleMode::Major ? major_template(style) : minor_template(style);
    const double start = count > 0 ? notes[0].start_seconds : 0.0;
    const double end = count > 0 ? notes[count - 1].end_seconds : config_.chord_duration_seconds;
    const auto chord_count = static_cast<std::size_t>(
        std::max(1.0, std::ceil((end - start) / config_.chord_duration_seconds)));

    ChordProgression progression;
    progression.style = style;
    progression.key = key;
    progression.chords.reserve(chord_count);
    for (std::size_t i = 0; i < chord_count; ++i) {
        const TemplateChord& item = template_chords[i % template_chords.size()];
        const double chord_start = start + static_cast<double>(i) * config_.chord_duration_seconds;
        const double chord_end = std::min(end, chord_start + config_.chord_duration_seconds);
        progression.chords.push_back(
            make_chord(chord_start, chord_end, key.tonic_pitch_class + item.interval_from_tonic, item.quality));
    }
    progression.score = score_progression(progression, notes, count);
    return progression;
}

float HarmonyAnalyzer::score_progression(
    const ChordProgression& progression,
    const NoteEvent* notes,
    std::size_t count) const {
    if (progression.chords.empty() || count == 0) {
        return 0.0f;
    }

    double weighted_score = 0.0;
    double weight_total = 0.0;
    for (std::size_t note_index = 0; note_index < count; ++note_index) {
        const NoteEvent& note = notes[note_index];
        const double midpoint = (note.start_seconds + note.end_seconds) * 0.5;
        const auto chord_it = std::find_if(
            progression.chords.begin(),
            progression.chords.end(),
            [midpoint](const Chord& chord) {
                return midpoint >= chord.start_seconds && midpoint <= chord.end_seconds;
            });
        if (chord_it == progression.chords.end()) {
            continue;
        }
        const std::vector<int> tones = chord_tones(*chord_it);
        const double duration = std::max(0.001, note.duration_seconds);
        const double note_score = contains_pitch_class(tones, pitch_class(note.midi_note)) ? 1.0 : 0.25;
        weighted_score += note_score * duration;
        weight_total += duration;
    }

    return weight_total > 0.0 ? static_cast<float>(weighted_score / weight_total) : 0.0f;
}

const char* note_name(int pc) noexcept {
    switch (pitch_class(pc)) {
        case 0:
            return "C";
        case 1:
            return "Db";
        case 2:
            return "D";
        case 3:
            return "Eb";
        case 4:
            return "E";
        case 5:
            return "F";
        case 6:
            return "Gb";
        case 7:
            return "G";
        case 8:
            return "Ab";
        case 9:
            return "A";
        case 10:
            return "Bb";
        case 11:
            return "B";
    }
    return "C";
}

const char* scale_mode_name(ScaleMode mode) noexcept {
    return mode == ScaleMode::Major ? "major" : "minor";
}

const char* harmony_style_name(HarmonyStyle style) noexcept {
    switch (style) {
        case HarmonyStyle::Pop:
            return "Pop";
        case HarmonyStyle::Ballad:
            return "Ballad";
        case HarmonyStyle::Jazz:
            return "Jazz";
        case HarmonyStyle::Cinematic:
            return "Cinematic";
        case HarmonyStyle::Classical:
            return "Classical";
    }
    return "Pop";
}

std::string chord_symbol(const Chord& chord) {
    std::string symbol = note_name(chord.root_pitch_class);
    switch (chord.quality) {
        case ChordQuality::Major:
            break;
        case ChordQuality::Minor:
            symbol += "m";
            break;
        case ChordQuality::Diminished:
            symbol += "dim";
            break;
        case ChordQuality::Major7:
            symbol += "maj7";
            break;
        case ChordQuality::Minor7:
            symbol += "m7";
            break;
        case ChordQuality::Dominant7:
            symbol += "7";
            break;
    }
    return symbol;
}

}  // namespace music_elf
