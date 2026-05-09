#include "music_elf/lyric_aligner.hpp"
#include "music_elf/musicxml_writer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::LyricAligner;
using music_elf::LyricToken;
using music_elf::MusicXmlWriterConfig;
using music_elf::NoteEvent;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

NoteEvent make_note(double start, double end, int midi_note) {
    NoteEvent note;
    note.start_seconds = start;
    note.end_seconds = end;
    note.duration_seconds = end - start;
    note.midi_note = midi_note;
    note.average_confidence = 0.95f;
    return note;
}

std::size_t count_occurrences(const std::string& text, const std::string& needle) {
    std::size_t count = 0;
    std::size_t offset = 0;
    while ((offset = text.find(needle, offset)) != std::string::npos) {
        ++count;
        offset += needle.size();
    }
    return count;
}

void test_lyric_alignment_equal_or_fewer_tokens() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.5, 60),
        make_note(0.5, 1.0, 62),
        make_note(1.0, 1.5, 64),
        make_note(1.5, 2.0, 65),
    };
    const std::vector<LyricToken> tokens = {{"I"}, {"sing"}};

    LyricAligner aligner;
    const auto alignments = aligner.align(tokens.data(), tokens.size(), notes.data(), notes.size());

    require(alignments.size() == 2, "alignment count");
    require(alignments[0].text == "I", "first lyric text");
    require(alignments[0].note_index == 0, "first lyric note");
    require(alignments[0].end_seconds == 1.0, "first lyric spans two notes");
    require(alignments[1].note_index == 2, "second lyric note");
    require(alignments[1].end_seconds == 2.0, "second lyric spans two notes");
}

void test_lyric_alignment_more_tokens_than_notes() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 1.0, 60),
        make_note(1.0, 2.0, 62),
    };
    const std::vector<LyricToken> tokens = {{"a"}, {"b"}, {"c"}, {"d"}};

    LyricAligner aligner;
    const auto alignments = aligner.align(tokens.data(), tokens.size(), notes.data(), notes.size());

    require(alignments.size() == 4, "dense lyric alignment count");
    require(alignments[0].note_index == 0, "first dense lyric note");
    require(alignments[2].note_index == 1, "third dense lyric note");
}

void test_musicxml_writer_outputs_notes_and_lyrics() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.5, 60),
        make_note(0.5, 1.0, 64),
    };
    const std::vector<LyricToken> tokens = {{"Sing"}, {"&"}};

    LyricAligner aligner;
    const auto lyrics = aligner.align(tokens.data(), tokens.size(), notes.data(), notes.size());
    MusicXmlWriterConfig config;
    config.bpm = 120.0;
    config.part_name = "Lead";
    const std::string xml = write_musicxml(notes.data(), notes.size(), lyrics.data(), lyrics.size(), config);

    require(xml.find("<score-partwise") != std::string::npos, "MusicXML root");
    require(xml.find("<part-name>Lead</part-name>") != std::string::npos, "part name");
    require(xml.find("<step>C</step><octave>4</octave>") != std::string::npos, "C4 pitch");
    require(xml.find("<step>E</step><octave>4</octave>") != std::string::npos, "E4 pitch");
    require(xml.find("<text>Sing</text>") != std::string::npos, "lyric text");
    require(xml.find("<text>&amp;</text>") != std::string::npos, "escaped lyric text");
}

void test_musicxml_writer_outputs_measures_and_rests() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.5, 60),
        make_note(2.0, 2.5, 62),
    };
    MusicXmlWriterConfig config;
    config.bpm = 120.0;

    const std::string xml = write_musicxml(notes.data(), notes.size(), config);

    require(xml.find("<measure number=\"1\">") != std::string::npos, "first measure");
    require(xml.find("<measure number=\"2\">") != std::string::npos, "second measure");
    require(xml.find("<rest/>") != std::string::npos, "gap should produce rest");
    require(xml.find("<type>half</type>") != std::string::npos ||
                xml.find("<type>quarter</type>") != std::string::npos,
            "rest should be spelled with normal note values");
}

void test_musicxml_writer_outputs_ties_across_measure() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.5, 60),
        make_note(1.5, 2.5, 64),
    };
    const std::vector<music_elf::LyricAlignment> lyrics = {
        {"Hold", 1, 1.5, 2.5},
    };
    MusicXmlWriterConfig config;
    config.bpm = 120.0;

    const std::string xml = write_musicxml(notes.data(), notes.size(), lyrics.data(), lyrics.size(), config);

    require(xml.find("<tie type=\"start\"/>") != std::string::npos, "tie start");
    require(xml.find("<tie type=\"stop\"/>") != std::string::npos, "tie stop");
    require(xml.find("<tied type=\"start\"/>") != std::string::npos, "notation tied start");
    require(xml.find("<tied type=\"stop\"/>") != std::string::npos, "notation tied stop");
    require(count_occurrences(xml, "<lyric>") == 1, "lyric should only appear on first tied segment");
}

void test_musicxml_writer_spells_dots_and_sixteenths() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.75, 60),
        make_note(0.75, 0.875, 62),
    };
    MusicXmlWriterConfig config;
    config.bpm = 120.0;

    const std::string xml = write_musicxml(notes.data(), notes.size(), config);

    require(xml.find("<type>quarter</type>\n        <dot/>") != std::string::npos, "dotted quarter");
    require(xml.find("<type>16th</type>") != std::string::npos, "sixteenth note");
}

void test_musicxml_writer_outputs_key_time_and_clef() {
    const std::vector<NoteEvent> notes = {
        make_note(0.0, 0.5, 66),
    };
    MusicXmlWriterConfig config;
    config.bpm = 120.0;
    config.key_signature_fifths = 2;
    config.time_signature_numerator = 3;
    config.time_signature_denominator = 4;

    const std::string xml = write_musicxml(notes.data(), notes.size(), config);

    require(xml.find("<key><fifths>2</fifths><mode>major</mode></key>") != std::string::npos, "key signature");
    require(xml.find("<time><beats>3</beats><beat-type>4</beat-type></time>") != std::string::npos, "time");
    require(xml.find("<clef><sign>G</sign><line>2</line></clef>") != std::string::npos, "treble clef");
}

}  // namespace

int main() {
    try {
        test_lyric_alignment_equal_or_fewer_tokens();
        test_lyric_alignment_more_tokens_than_notes();
        test_musicxml_writer_outputs_notes_and_lyrics();
        test_musicxml_writer_outputs_measures_and_rests();
        test_musicxml_writer_outputs_ties_across_measure();
        test_musicxml_writer_spells_dots_and_sixteenths();
        test_musicxml_writer_outputs_key_time_and_clef();
    } catch (const std::exception& error) {
        std::cerr << "lyrics_musicxml_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "lyrics_musicxml_tests passed\n";
    return EXIT_SUCCESS;
}
