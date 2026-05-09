#include "music_elf/musicxml_writer.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace music_elf {
namespace {

struct PitchSpelling {
    const char* step = "C";
    int alter = 0;
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

int duration_to_divisions(double seconds, const MusicXmlWriterConfig& config) {
    const double beats = seconds * config.bpm / 60.0;
    return std::max(1, static_cast<int>(std::llround(beats * config.divisions_per_quarter)));
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

void validate_config(const MusicXmlWriterConfig& config) {
    if (config.divisions_per_quarter <= 0) {
        throw std::invalid_argument("divisions_per_quarter must be positive");
    }
    if (config.bpm <= 0.0) {
        throw std::invalid_argument("bpm must be positive");
    }
}

}  // namespace

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const MusicXmlWriterConfig& config) {
    return write_musicxml(notes, note_count, nullptr, 0, config);
}

std::string write_musicxml(
    const NoteEvent* notes,
    std::size_t note_count,
    const LyricAlignment* lyrics,
    std::size_t lyric_count,
    const MusicXmlWriterConfig& config) {
    if (note_count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when note_count is non-zero");
    }
    if (lyric_count > 0 && lyrics == nullptr) {
        throw std::invalid_argument("lyrics must not be null when lyric_count is non-zero");
    }
    validate_config(config);

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<score-partwise version=\"3.1\">\n";
    xml << "  <part-list>\n";
    xml << "    <score-part id=\"P1\"><part-name>" << escape_xml(config.part_name)
        << "</part-name></score-part>\n";
    xml << "  </part-list>\n";
    xml << "  <part id=\"P1\">\n";
    xml << "    <measure number=\"1\">\n";
    xml << "      <attributes><divisions>" << config.divisions_per_quarter
        << "</divisions><time><beats>4</beats><beat-type>4</beat-type></time></attributes>\n";
    xml << "      <direction placement=\"above\"><direction-type><metronome><beat-unit>quarter</beat-unit>"
        << "<per-minute>" << static_cast<int>(std::llround(config.bpm))
        << "</per-minute></metronome></direction-type></direction>\n";

    for (std::size_t i = 0; i < note_count; ++i) {
        const NoteEvent& note = notes[i];
        const int midi = std::max(0, std::min(127, note.midi_note));
        const int pitch_class = midi % 12;
        const int octave = midi / 12 - 1;
        const PitchSpelling spelling = spell_pitch_class(pitch_class);
        const int duration = duration_to_divisions(note.duration_seconds, config);

        xml << "      <note>\n";
        xml << "        <pitch><step>" << spelling.step << "</step>";
        if (spelling.alter != 0) {
            xml << "<alter>" << spelling.alter << "</alter>";
        }
        xml << "<octave>" << octave << "</octave></pitch>\n";
        xml << "        <duration>" << duration << "</duration>\n";
        xml << "        <voice>1</voice><type>quarter</type>\n";

        const LyricAlignment* lyric = lyric_for_note(lyrics, lyric_count, i);
        if (lyric != nullptr) {
            xml << "        <lyric><text>" << escape_xml(lyric->text) << "</text></lyric>\n";
        }
        xml << "      </note>\n";
    }

    xml << "    </measure>\n";
    xml << "  </part>\n";
    xml << "</score-partwise>\n";
    return xml.str();
}

}  // namespace music_elf

