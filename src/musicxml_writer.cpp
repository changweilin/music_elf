#include "music_elf/musicxml_writer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace music_elf {
namespace {

struct PitchSpelling {
    const char* step = "C";
    int alter = 0;
};

struct DurationToken {
    int ticks = 0;
    const char* type = "quarter";
    bool dotted = false;
};

struct LogicalScoreEvent {
    bool rest = false;
    std::size_t note_index = 0;
    int start_ticks = 0;
    int duration_ticks = 0;
    int midi_note = 60;
};

struct XmlScoreEvent {
    bool rest = false;
    std::size_t note_index = 0;
    int start_ticks = 0;
    DurationToken duration;
    int midi_note = 60;
    bool tie_start = false;
    bool tie_stop = false;
    bool lyric_allowed = false;
};

struct ChordSymbolEvent {
    int start_ticks = 0;
    Chord chord;
};

PitchSpelling spell_pitch_class(int pitch_class) {
    switch (((pitch_class % 12) + 12) % 12) {
        case 0:
            return {"C", 0};
        case 1:
            return {"D", -1};
        case 2:
            return {"D", 0};
        case 3:
            return {"E", -1};
        case 4:
            return {"E", 0};
        case 5:
            return {"F", 0};
        case 6:
            return {"G", -1};
        case 7:
            return {"G", 0};
        case 8:
            return {"A", -1};
        case 9:
            return {"A", 0};
        case 10:
            return {"B", -1};
        case 11:
            return {"B", 0};
    }
    return {"C", 0};
}

std::string escape_xml(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            case '\'':
                escaped += "&apos;";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

bool is_power_of_two(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

int ticks_for_denominator(int denominator, const MusicXmlWriterConfig& config) {
    const int numerator = config.divisions_per_quarter * 4;
    if (numerator % denominator != 0) {
        throw std::invalid_argument("divisions_per_quarter must divide the requested note value grid");
    }
    return numerator / denominator;
}

int ticks_per_beat(const MusicXmlWriterConfig& config) {
    return ticks_for_denominator(config.time_signature_denominator, config);
}

int ticks_per_measure(const MusicXmlWriterConfig& config) {
    return ticks_per_beat(config) * config.time_signature_numerator;
}

int minimum_ticks(const MusicXmlWriterConfig& config) {
    return ticks_for_denominator(config.minimum_note_value_denominator, config);
}

int round_to_step_ticks(int ticks, int step_ticks) {
    if (step_ticks <= 0) {
        return ticks;
    }
    return std::max(0, static_cast<int>(std::llround(
                           static_cast<double>(ticks) / static_cast<double>(step_ticks))) *
                           step_ticks);
}

int quantize_duration_ticks(int ticks, int step_ticks) {
    return std::max(step_ticks, round_to_step_ticks(ticks, step_ticks));
}

const char* note_type_for_denominator(int denominator) {
    switch (denominator) {
        case 1:
            return "whole";
        case 2:
            return "half";
        case 4:
            return "quarter";
        case 8:
            return "eighth";
        case 16:
            return "16th";
        case 32:
            return "32nd";
        case 64:
            return "64th";
    }
    return "quarter";
}

std::vector<DurationToken> duration_tokens(const MusicXmlWriterConfig& config) {
    const int min_ticks = minimum_ticks(config);
    const int measure_ticks = ticks_per_measure(config);
    std::vector<DurationToken> tokens;

    for (int denominator = 1; denominator <= config.minimum_note_value_denominator; denominator *= 2) {
        const int base_ticks = ticks_for_denominator(denominator, config);
        if (base_ticks <= measure_ticks) {
            tokens.push_back({base_ticks, note_type_for_denominator(denominator), false});
        }

        const int dotted_ticks = base_ticks + base_ticks / 2;
        if (denominator > 1 && base_ticks % 2 == 0 && dotted_ticks <= measure_ticks &&
            dotted_ticks % min_ticks == 0) {
            tokens.push_back({dotted_ticks, note_type_for_denominator(denominator), true});
        }
    }

    std::stable_sort(
        tokens.begin(),
        tokens.end(),
        [](const DurationToken& left, const DurationToken& right) {
            if (left.ticks != right.ticks) {
                return left.ticks > right.ticks;
            }
            return left.dotted && !right.dotted;
        });
    return tokens;
}

std::vector<DurationToken> split_duration(int ticks, const MusicXmlWriterConfig& config) {
    const auto allowed = duration_tokens(config);
    std::vector<DurationToken> result;
    int remaining = ticks;
    while (remaining > 0) {
        const auto found = std::find_if(
            allowed.begin(),
            allowed.end(),
            [remaining](const DurationToken& token) {
                return token.ticks <= remaining;
            });
        if (found == allowed.end()) {
            throw std::runtime_error("duration cannot be represented on the configured note grid");
        }
        result.push_back(*found);
        remaining -= found->ticks;
    }
    return result;
}

const LyricAlignment* lyric_for_note(
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    std::size_t note_index) {
    for (std::size_t i = 0; i < lyric_count; ++i) {
        if (lyrics[i].note_index == note_index) {
            return &lyrics[i];
        }
    }
    return nullptr;
}

const char* chord_kind_value(ChordQuality quality) {
    switch (quality) {
        case ChordQuality::Major:
            return "major";
        case ChordQuality::Minor:
            return "minor";
        case ChordQuality::Diminished:
            return "diminished";
        case ChordQuality::Major7:
            return "major-seventh";
        case ChordQuality::Minor7:
            return "minor-seventh";
        case ChordQuality::Dominant7:
            return "dominant";
    }
    return "major";
}

const char* chord_kind_text(ChordQuality quality) {
    switch (quality) {
        case ChordQuality::Major:
            return "";
        case ChordQuality::Minor:
            return "m";
        case ChordQuality::Diminished:
            return "dim";
        case ChordQuality::Major7:
            return "maj7";
        case ChordQuality::Minor7:
            return "m7";
        case ChordQuality::Dominant7:
            return "7";
    }
    return "";
}

void validate_config(const MusicXmlWriterConfig& config) {
    if (config.divisions_per_quarter <= 0) {
        throw std::invalid_argument("divisions_per_quarter must be positive");
    }
    if (config.bpm <= 0.0) {
        throw std::invalid_argument("bpm must be positive");
    }
    if (config.time_signature_numerator <= 0) {
        throw std::invalid_argument("time_signature_numerator must be positive");
    }
    if (!is_power_of_two(config.time_signature_denominator)) {
        throw std::invalid_argument("time_signature_denominator must be a positive power of two");
    }
    if (config.key_signature_fifths < -7 || config.key_signature_fifths > 7) {
        throw std::invalid_argument("key_signature_fifths must be in [-7, 7]");
    }
    if (!is_power_of_two(config.minimum_note_value_denominator)) {
        throw std::invalid_argument("minimum_note_value_denominator must be a positive power of two");
    }
    if (config.minimum_note_value_denominator < config.time_signature_denominator) {
        throw std::invalid_argument("minimum_note_value_denominator must be no coarser than the beat unit");
    }
    (void)ticks_per_measure(config);
    (void)minimum_ticks(config);
}

RhythmAnalysis rhythm_from_notes(
    const NoteEvent* notes,
    std::size_t note_count,
    const MusicXmlWriterConfig& config) {
    RhythmAnalysis rhythm;
    rhythm.beat_grid.bpm = config.bpm;
    rhythm.beat_grid.beats_per_bar = config.time_signature_numerator;
    rhythm.beat_grid.beat_duration_seconds = 60.0 / config.bpm;
    rhythm.beat_grid.first_beat_seconds = note_count > 0 ? notes[0].start_seconds : 0.0;
    rhythm.quantized_notes.reserve(note_count);

    const double subdivisions =
        static_cast<double>(config.minimum_note_value_denominator) /
        static_cast<double>(config.time_signature_denominator);
    const double min_beats = 1.0 / subdivisions;
    for (std::size_t i = 0; i < note_count; ++i) {
        QuantizedNote quantized;
        quantized.note = notes[i];
        const double start_beats =
            (notes[i].start_seconds - rhythm.beat_grid.first_beat_seconds) /
            rhythm.beat_grid.beat_duration_seconds;
        const double duration_beats = notes[i].duration_seconds / rhythm.beat_grid.beat_duration_seconds;
        quantized.start_beats = std::round(start_beats * subdivisions) / subdivisions;
        quantized.duration_beats = std::max(min_beats, std::round(duration_beats * subdivisions) / subdivisions);
        quantized.quantized_start_seconds =
            rhythm.beat_grid.first_beat_seconds + quantized.start_beats * rhythm.beat_grid.beat_duration_seconds;
        quantized.quantized_duration_seconds = quantized.duration_beats * rhythm.beat_grid.beat_duration_seconds;
        rhythm.quantized_notes.push_back(quantized);
    }

    return rhythm;
}

std::vector<LogicalScoreEvent> logical_events_from_rhythm(
    const RhythmAnalysis& rhythm,
    const MusicXmlWriterConfig& config) {
    struct IndexedQuantizedNote {
        std::size_t index = 0;
        QuantizedNote note;
    };

    const int beat_ticks = ticks_per_beat(config);
    const int min_ticks = minimum_ticks(config);
    std::vector<IndexedQuantizedNote> notes;
    notes.reserve(rhythm.quantized_notes.size());
    for (std::size_t i = 0; i < rhythm.quantized_notes.size(); ++i) {
        notes.push_back({i, rhythm.quantized_notes[i]});
    }
    std::stable_sort(
        notes.begin(),
        notes.end(),
        [beat_ticks](const IndexedQuantizedNote& left, const IndexedQuantizedNote& right) {
            const int left_start =
                static_cast<int>(std::llround(left.note.start_beats * static_cast<double>(beat_ticks)));
            const int right_start =
                static_cast<int>(std::llround(right.note.start_beats * static_cast<double>(beat_ticks)));
            return left_start < right_start;
        });

    std::vector<LogicalScoreEvent> events;
    int cursor = 0;
    for (const auto& indexed : notes) {
        int start_ticks = static_cast<int>(
            std::llround(indexed.note.start_beats * static_cast<double>(beat_ticks)));
        int duration_ticks = static_cast<int>(
            std::llround(indexed.note.duration_beats * static_cast<double>(beat_ticks)));
        start_ticks = round_to_step_ticks(start_ticks, min_ticks);
        duration_ticks = quantize_duration_ticks(duration_ticks, min_ticks);
        int end_ticks = start_ticks + duration_ticks;
        if (end_ticks <= cursor) {
            continue;
        }

        if (start_ticks > cursor) {
            if (config.include_rests) {
                events.push_back({true, 0, cursor, start_ticks - cursor, 60});
            }
            cursor = start_ticks;
        } else if (start_ticks < cursor) {
            start_ticks = cursor;
        }

        if (end_ticks > start_ticks) {
            events.push_back(
                {false, indexed.index, start_ticks, end_ticks - start_ticks, indexed.note.note.midi_note});
            cursor = end_ticks;
        }
    }
    return events;
}

std::vector<ChordSymbolEvent> chord_events_from_chords(
    const RhythmAnalysis& rhythm,
    const Chord* chords,
    std::size_t chord_count,
    const MusicXmlWriterConfig& config) {
    if (chord_count > 0 && chords == nullptr) {
        throw std::invalid_argument("chords must not be null when chord_count is non-zero");
    }

    std::vector<ChordSymbolEvent> events;
    events.reserve(chord_count);
    const int beat_ticks = ticks_per_beat(config);
    const int min_ticks = minimum_ticks(config);
    const double beat_duration = rhythm.beat_grid.beat_duration_seconds > 0.0
                                     ? rhythm.beat_grid.beat_duration_seconds
                                     : 60.0 / config.bpm;
    const double first_beat = rhythm.beat_grid.first_beat_seconds;
    for (std::size_t i = 0; i < chord_count; ++i) {
        if (chords[i].end_seconds <= chords[i].start_seconds) {
            continue;
        }
        const double start_beats = (chords[i].start_seconds - first_beat) / beat_duration;
        int start_ticks = static_cast<int>(std::llround(start_beats * static_cast<double>(beat_ticks)));
        start_ticks = round_to_step_ticks(std::max(0, start_ticks), min_ticks);
        events.push_back({start_ticks, chords[i]});
    }

    std::stable_sort(
        events.begin(),
        events.end(),
        [](const ChordSymbolEvent& left, const ChordSymbolEvent& right) {
            return left.start_ticks < right.start_ticks;
        });
    events.erase(
        std::unique(
            events.begin(),
            events.end(),
            [](const ChordSymbolEvent& left, const ChordSymbolEvent& right) {
                return left.start_ticks == right.start_ticks;
            }),
        events.end());
    return events;
}

std::vector<int> split_boundaries_from_chords(
    const std::vector<ChordSymbolEvent>& chord_events,
    const MusicXmlWriterConfig& config) {
    std::vector<int> boundaries;
    boundaries.reserve(chord_events.size());
    const int measure_ticks = ticks_per_measure(config);
    for (const auto& event : chord_events) {
        if (event.start_ticks > 0 && event.start_ticks % measure_ticks != 0) {
            boundaries.push_back(event.start_ticks);
        }
    }
    boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());
    return boundaries;
}

int next_split_boundary(int start, int end, const std::vector<int>& split_boundaries) {
    const auto found = std::upper_bound(split_boundaries.begin(), split_boundaries.end(), start);
    if (found != split_boundaries.end() && *found < end) {
        return *found;
    }
    return end;
}

void append_xml_events_for_logical_event(
    const LogicalScoreEvent& logical,
    const MusicXmlWriterConfig& config,
    const std::vector<int>& split_boundaries,
    std::vector<XmlScoreEvent>& out) {
    const int measure_ticks = ticks_per_measure(config);
    int start = logical.start_ticks;
    int remaining = logical.duration_ticks;
    std::vector<XmlScoreEvent> pieces;

    while (remaining > 0) {
        const int measure_remaining = measure_ticks - (start % measure_ticks);
        const int natural_segment_end = start + std::min(remaining, measure_remaining);
        const int segment_end = next_split_boundary(start, natural_segment_end, split_boundaries);
        const int segment_ticks = segment_end - start;
        int token_start = start;
        for (const auto& token : split_duration(segment_ticks, config)) {
            XmlScoreEvent event;
            event.rest = logical.rest;
            event.note_index = logical.note_index;
            event.start_ticks = token_start;
            event.duration = token;
            event.midi_note = logical.midi_note;
            pieces.push_back(event);
            token_start += token.ticks;
        }
        start += segment_ticks;
        remaining -= segment_ticks;
    }

    for (std::size_t i = 0; i < pieces.size(); ++i) {
        if (!pieces[i].rest && pieces.size() > 1) {
            pieces[i].tie_stop = i > 0;
            pieces[i].tie_start = i + 1 < pieces.size();
        }
        pieces[i].lyric_allowed = !pieces[i].rest && i == 0;
        out.push_back(pieces[i]);
    }
}

std::vector<XmlScoreEvent> xml_events_from_rhythm(
    const RhythmAnalysis& rhythm,
    const MusicXmlWriterConfig& config,
    const std::vector<int>& split_boundaries) {
    std::vector<XmlScoreEvent> events;
    for (const auto& logical : logical_events_from_rhythm(rhythm, config)) {
        append_xml_events_for_logical_event(logical, config, split_boundaries, events);
    }
    return events;
}

void write_measure_header(std::ostringstream& xml, int measure_number, const MusicXmlWriterConfig& config) {
    xml << "    <measure number=\"" << measure_number << "\">\n";
    if (measure_number == 1) {
        xml << "      <attributes><divisions>" << config.divisions_per_quarter << "</divisions>"
            << "<key><fifths>" << config.key_signature_fifths << "</fifths><mode>"
            << (config.key_signature_is_minor ? "minor" : "major") << "</mode></key>"
            << "<time><beats>" << config.time_signature_numerator << "</beats><beat-type>"
            << config.time_signature_denominator << "</beat-type></time>"
            << "<clef><sign>G</sign><line>2</line></clef></attributes>\n";
        xml << "      <direction placement=\"above\"><direction-type><metronome><beat-unit>quarter</beat-unit>"
            << "<per-minute>" << static_cast<int>(std::llround(config.bpm))
            << "</per-minute></metronome></direction-type></direction>\n";
    }
}

void write_pitch(std::ostringstream& xml, int midi_note) {
    const int midi = std::max(0, std::min(127, midi_note));
    const int pitch_class = midi % 12;
    const int octave = midi / 12 - 1;
    const PitchSpelling spelling = spell_pitch_class(pitch_class);
    xml << "        <pitch><step>" << spelling.step << "</step>";
    if (spelling.alter != 0) {
        xml << "<alter>" << spelling.alter << "</alter>";
    }
    xml << "<octave>" << octave << "</octave></pitch>\n";
}

void write_harmony(std::ostringstream& xml, const Chord& chord) {
    const PitchSpelling root = spell_pitch_class(chord.root_pitch_class);
    xml << "      <harmony>\n";
    xml << "        <root><root-step>" << root.step << "</root-step>";
    if (root.alter != 0) {
        xml << "<root-alter>" << root.alter << "</root-alter>";
    }
    xml << "</root>\n";
    xml << "        <kind text=\"" << escape_xml(chord_kind_text(chord.quality)) << "\">"
        << chord_kind_value(chord.quality) << "</kind>\n";
    xml << "      </harmony>\n";
}

void write_xml_event(
    std::ostringstream& xml,
    const XmlScoreEvent& event,
    const LyricAlignment* lyrics,
    std::size_t lyric_count) {
    xml << "      <note>\n";
    if (event.rest) {
        xml << "        <rest/>\n";
    } else {
        write_pitch(xml, event.midi_note);
    }
    xml << "        <duration>" << event.duration.ticks << "</duration>\n";
    if (!event.rest && event.tie_stop) {
        xml << "        <tie type=\"stop\"/>\n";
    }
    if (!event.rest && event.tie_start) {
        xml << "        <tie type=\"start\"/>\n";
    }
    xml << "        <voice>1</voice><type>" << event.duration.type << "</type>\n";
    if (event.duration.dotted) {
        xml << "        <dot/>\n";
    }
    if (!event.rest && (event.tie_start || event.tie_stop)) {
        xml << "        <notations>";
        if (event.tie_stop) {
            xml << "<tied type=\"stop\"/>";
        }
        if (event.tie_start) {
            xml << "<tied type=\"start\"/>";
        }
        xml << "</notations>\n";
    }

    const LyricAlignment* lyric =
        event.lyric_allowed ? lyric_for_note(lyrics, lyric_count, event.note_index) : nullptr;
    if (lyric != nullptr) {
        xml << "        <lyric><text>" << escape_xml(lyric->text) << "</text></lyric>\n";
    }
    xml << "      </note>\n";
}

}  // namespace

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(notes, note_count, nullptr, 0, nullptr, 0, config);
}

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(notes, note_count, nullptr, 0, lyrics, lyric_count, config);
}

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const Chord* chords,
    std::size_t chord_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(notes, note_count, chords, chord_count, nullptr, 0, config);
}

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const Chord* chords,
    std::size_t chord_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config) {
    if (note_count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when note_count is non-zero");
    }
    if (chord_count > 0 && chords == nullptr) {
        throw std::invalid_argument("chords must not be null when chord_count is non-zero");
    }
    if (lyric_count > 0 && lyrics == nullptr) {
        throw std::invalid_argument("lyrics must not be null when lyric_count is non-zero");
    }
    validate_config(config);
    const RhythmAnalysis rhythm = rhythm_from_notes(notes, note_count, config);
    return write_musicxml(rhythm, chords, chord_count, lyrics, lyric_count, config);
}

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(rhythm, nullptr, 0, nullptr, 0, config);
}

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(rhythm, nullptr, 0, lyrics, lyric_count, config);
}

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const Chord* chords,
    std::size_t chord_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(rhythm, chords, chord_count, nullptr, 0, config);
}

std::string write_musicxml(
    const RhythmAnalysis& rhythm,
    const Chord* chords,
    std::size_t chord_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config) {
    if (chord_count > 0 && chords == nullptr) {
        throw std::invalid_argument("chords must not be null when chord_count is non-zero");
    }
    if (lyric_count > 0 && lyrics == nullptr) {
        throw std::invalid_argument("lyrics must not be null when lyric_count is non-zero");
    }
    validate_config(config);

    const auto chord_events = chord_events_from_chords(rhythm, chords, chord_count, config);
    const auto split_boundaries = split_boundaries_from_chords(chord_events, config);
    const auto events = xml_events_from_rhythm(rhythm, config, split_boundaries);
    const int measure_ticks = ticks_per_measure(config);
    int last_tick = events.empty() ? 0 : events.back().start_ticks;
    if (!chord_events.empty()) {
        last_tick = std::max(last_tick, chord_events.back().start_ticks);
    }
    const int last_measure = last_tick / measure_ticks;

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<score-partwise version=\"3.1\">\n";
    xml << "  <part-list>\n";
    xml << "    <score-part id=\"P1\"><part-name>" << escape_xml(config.part_name)
        << "</part-name></score-part>\n";
    xml << "  </part-list>\n";
    xml << "  <part id=\"P1\">\n";

    std::size_t event_index = 0;
    std::size_t chord_index = 0;
    for (int measure = 0; measure <= last_measure; ++measure) {
        write_measure_header(xml, measure + 1, config);
        const int measure_start = measure * measure_ticks;
        const int measure_end = measure_start + measure_ticks;
        while (event_index < events.size() &&
               events[event_index].start_ticks / measure_ticks == measure) {
            while (chord_index < chord_events.size() &&
                   chord_events[chord_index].start_ticks < measure_end &&
                   chord_events[chord_index].start_ticks <= events[event_index].start_ticks) {
                write_harmony(xml, chord_events[chord_index].chord);
                ++chord_index;
            }
            write_xml_event(xml, events[event_index], lyrics, lyric_count);
            ++event_index;
        }
        while (chord_index < chord_events.size() &&
               chord_events[chord_index].start_ticks >= measure_start &&
               chord_events[chord_index].start_ticks < measure_end) {
            write_harmony(xml, chord_events[chord_index].chord);
            ++chord_index;
        }
        xml << "    </measure>\n";
    }

    xml << "  </part>\n";
    xml << "</score-partwise>\n";
    return xml.str();
}

}  // namespace music_elf
