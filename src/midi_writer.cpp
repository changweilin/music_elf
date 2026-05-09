#include "music_elf/midi_writer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace music_elf {
namespace {

struct MidiEvent {
    std::uint32_t tick = 0;
    std::uint8_t status = 0;
    std::uint8_t data1 = 0;
    std::uint8_t data2 = 0;
    bool note_off = false;
};

int clamp_int(int value, int low, int high) {
    return std::max(low, std::min(high, value));
}

std::uint32_t seconds_to_ticks(double seconds, const MidiWriterConfig& config) {
    const double beats = seconds * config.bpm / 60.0;
    const double ticks = std::max(0.0, beats * static_cast<double>(config.ticks_per_quarter));
    return static_cast<std::uint32_t>(std::llround(ticks));
}

void write_u16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xff));
}

void write_u32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>((value >> 24) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xff));
}

void write_ascii(std::vector<std::uint8_t>& bytes, const char* text) {
    while (*text != '\0') {
        bytes.push_back(static_cast<std::uint8_t>(*text));
        ++text;
    }
}

void write_varlen(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    std::uint8_t buffer[5]{};
    int index = 4;
    buffer[index] = static_cast<std::uint8_t>(value & 0x7f);
    while ((value >>= 7) != 0) {
        --index;
        buffer[index] = static_cast<std::uint8_t>((value & 0x7f) | 0x80);
    }
    for (; index < 5; ++index) {
        bytes.push_back(buffer[index]);
    }
}

void validate_config(const MidiWriterConfig& config) {
    if (config.ticks_per_quarter <= 0 || config.ticks_per_quarter > 32767) {
        throw std::invalid_argument("ticks_per_quarter must be in [1, 32767]");
    }
    if (config.bpm <= 0.0) {
        throw std::invalid_argument("bpm must be positive");
    }
    if (config.melody_channel < 0 || config.melody_channel > 15) {
        throw std::invalid_argument("melody_channel must be in [0, 15]");
    }
}

std::vector<std::uint8_t> write_events_to_midi(std::vector<MidiEvent> events, const MidiWriterConfig& config) {
    std::stable_sort(
        events.begin(),
        events.end(),
        [](const MidiEvent& left, const MidiEvent& right) {
            if (left.tick != right.tick) {
                return left.tick < right.tick;
            }
            return left.note_off && !right.note_off;
        });

    std::vector<std::uint8_t> track;
    write_varlen(track, 0);
    track.push_back(0xff);
    track.push_back(0x51);
    track.push_back(0x03);
    const auto microseconds_per_quarter =
        static_cast<std::uint32_t>(std::llround(60000000.0 / config.bpm));
    track.push_back(static_cast<std::uint8_t>((microseconds_per_quarter >> 16) & 0xff));
    track.push_back(static_cast<std::uint8_t>((microseconds_per_quarter >> 8) & 0xff));
    track.push_back(static_cast<std::uint8_t>(microseconds_per_quarter & 0xff));

    std::uint32_t previous_tick = 0;
    for (const auto& event : events) {
        write_varlen(track, event.tick - previous_tick);
        track.push_back(event.status);
        track.push_back(event.data1);
        track.push_back(event.data2);
        previous_tick = event.tick;
    }

    write_varlen(track, 0);
    track.push_back(0xff);
    track.push_back(0x2f);
    track.push_back(0x00);

    std::vector<std::uint8_t> bytes;
    write_ascii(bytes, "MThd");
    write_u32(bytes, 6);
    write_u16(bytes, 0);
    write_u16(bytes, 1);
    write_u16(bytes, static_cast<std::uint16_t>(config.ticks_per_quarter));
    write_ascii(bytes, "MTrk");
    write_u32(bytes, static_cast<std::uint32_t>(track.size()));
    bytes.insert(bytes.end(), track.begin(), track.end());
    return bytes;
}

}  // namespace

std::vector<std::uint8_t> write_midi(
    const GeneratedNote* notes,
    std::size_t count,
    const MidiWriterConfig& config) {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }
    validate_config(config);

    std::vector<MidiEvent> events;
    events.reserve(count * 2);
    for (std::size_t i = 0; i < count; ++i) {
        if (notes[i].end_seconds <= notes[i].start_seconds) {
            continue;
        }
        const int channel = clamp_int(notes[i].channel, 0, 15);
        const auto note = static_cast<std::uint8_t>(clamp_int(notes[i].midi_note, 0, 127));
        const auto velocity = static_cast<std::uint8_t>(clamp_int(notes[i].velocity, 1, 127));
        events.push_back({seconds_to_ticks(notes[i].start_seconds, config),
                          static_cast<std::uint8_t>(0x90 | channel), note, velocity, false});
        events.push_back({seconds_to_ticks(notes[i].end_seconds, config),
                          static_cast<std::uint8_t>(0x80 | channel), note, 0, true});
    }
    return write_events_to_midi(std::move(events), config);
}

std::vector<std::uint8_t> write_midi(
    const NoteEvent* notes,
    std::size_t count,
    const MidiWriterConfig& config) {
    if (count > 0 && notes == nullptr) {
        throw std::invalid_argument("notes must not be null when count is non-zero");
    }
    validate_config(config);

    std::vector<GeneratedNote> generated;
    generated.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        GeneratedNote note;
        note.start_seconds = notes[i].start_seconds;
        note.end_seconds = notes[i].end_seconds;
        note.midi_note = notes[i].midi_note;
        note.velocity = clamp_int(config.melody_velocity, 1, 127);
        note.channel = config.melody_channel;
        generated.push_back(note);
    }
    return write_midi(generated.data(), generated.size(), config);
}

}  // namespace music_elf
