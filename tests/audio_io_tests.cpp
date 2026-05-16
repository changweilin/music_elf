#include "music_elf/audio_io.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kPi = 3.1415926535897932384626433832795;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void write_u16(std::ostream& out, std::uint16_t value) {
    const char bytes[] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
    };
    out.write(bytes, sizeof(bytes));
}

void write_u32(std::ostream& out, std::uint32_t value) {
    const char bytes[] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff),
    };
    out.write(bytes, sizeof(bytes));
}

std::vector<float> make_sine(int sample_rate, double seconds, double frequency_hz) {
    std::vector<float> samples(static_cast<std::size_t>(seconds * sample_rate));
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const double time = static_cast<double>(i) / static_cast<double>(sample_rate);
        samples[i] = static_cast<float>(0.5 * std::sin(2.0 * kPi * frequency_hz * time));
    }
    return samples;
}

void test_wav_roundtrip_mono() {
    const std::string path = "audio_io_roundtrip.wav";
    const auto source = music_elf::make_mono_audio(48000, make_sine(48000, 0.25, 440.0));
    music_elf::write_wav_file(path, source);
    const auto loaded = music_elf::read_wav_file(path);
    std::remove(path.c_str());

    require(loaded.sample_rate == source.sample_rate, "sample rate should roundtrip");
    require(loaded.channels == 1, "channel count should roundtrip");
    require(loaded.samples.size() == source.samples.size(), "sample count should roundtrip");
    require(std::fabs(loaded.samples[100] - source.samples[100]) < 0.001f, "PCM16 sample should be close");
}

void write_extensible_pcm16_wav(const std::string& path) {
    constexpr std::uint16_t format_extensible = 0xfffe;
    constexpr std::uint16_t channels = 2;
    constexpr std::uint32_t sample_rate = 49400;
    constexpr std::uint16_t bits_per_sample = 16;
    constexpr std::uint16_t block_align = channels * bits_per_sample / 8;
    constexpr std::uint32_t byte_rate = sample_rate * block_align;
    constexpr std::uint32_t fmt_size = 40;
    const std::vector<std::int16_t> samples = {0, 32767, -32768, 8192};
    const std::uint32_t data_size = static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
    const std::uint32_t riff_size = 4u + (8u + fmt_size) + (8u + data_size);
    const std::uint8_t pcm_guid[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
    };

    std::ofstream out(path, std::ios::binary);
    require(static_cast<bool>(out), "create extensible wav");
    out.write("RIFF", 4);
    write_u32(out, riff_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    write_u32(out, fmt_size);
    write_u16(out, format_extensible);
    write_u16(out, channels);
    write_u32(out, sample_rate);
    write_u32(out, byte_rate);
    write_u16(out, block_align);
    write_u16(out, bits_per_sample);
    write_u16(out, 22);               // extension size
    write_u16(out, bits_per_sample);  // valid bits per sample
    write_u32(out, 0x00000003);       // front left + front right
    out.write(reinterpret_cast<const char*>(pcm_guid), sizeof(pcm_guid));
    out.write("data", 4);
    write_u32(out, data_size);
    for (const auto sample : samples) {
        write_u16(out, static_cast<std::uint16_t>(sample));
    }
}

void test_reads_wave_format_extensible_pcm16_stereo() {
    const std::string path = "audio_io_extensible_pcm.wav";
    write_extensible_pcm16_wav(path);
    const auto loaded = music_elf::read_wav_file(path);
    std::remove(path.c_str());

    require(loaded.sample_rate == 49400, "extensible sample rate");
    require(loaded.channels == 2, "extensible channel count");
    require(loaded.samples.size() == 4, "extensible sample count");
    require(std::fabs(loaded.samples[1] - (32767.0f / 32768.0f)) < 0.0001f,
            "extensible positive sample");
    require(std::fabs(loaded.samples[2] + 1.0f) < 0.0001f,
            "extensible negative sample");
}

void test_downmix_stereo() {
    music_elf::AudioBuffer audio;
    audio.sample_rate = 48000;
    audio.channels = 2;
    audio.samples = {1.0f, 0.0f, 0.25f, -0.25f};
    const auto mono = music_elf::downmix_to_mono(audio);
    require(mono.size() == 2, "downmix frame count");
    require(std::fabs(mono[0] - 0.5f) < 0.0001f, "downmix first frame");
    require(std::fabs(mono[1] - 0.0f) < 0.0001f, "downmix second frame");
}

}  // namespace

int main() {
    try {
        test_wav_roundtrip_mono();
        test_reads_wave_format_extensible_pcm16_stereo();
        test_downmix_stereo();
    } catch (const std::exception& error) {
        std::cerr << "audio_io_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "audio_io_tests passed\n";
    return EXIT_SUCCESS;
}
