#include "music_elf/audio_io.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <stdexcept>

namespace music_elf {
namespace {

constexpr std::uint16_t kWaveFormatPcm = 0x0001;
constexpr std::uint16_t kWaveFormatIeeeFloat = 0x0003;
constexpr std::uint16_t kWaveFormatExtensible = 0xfffe;

std::uint16_t read_u16(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return static_cast<std::uint16_t>(data[offset] | (data[offset + 1] << 8));
}

std::uint32_t read_u32(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return static_cast<std::uint32_t>(data[offset]) |
           (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(data[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(data[offset + 3]) << 24);
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

bool chunk_id_is(const std::vector<std::uint8_t>& data, std::size_t offset, const char* id) {
    return data[offset] == static_cast<std::uint8_t>(id[0]) &&
           data[offset + 1] == static_cast<std::uint8_t>(id[1]) &&
           data[offset + 2] == static_cast<std::uint8_t>(id[2]) &&
           data[offset + 3] == static_cast<std::uint8_t>(id[3]);
}

bool extensible_subformat_is(const std::vector<std::uint8_t>& data,
                             std::size_t offset,
                             std::uint16_t format_tag) {
    static constexpr std::uint8_t kWaveSubformatTail[] = {
        0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
        0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
    };

    if (read_u32(data, offset) != static_cast<std::uint32_t>(format_tag)) {
        return false;
    }
    return std::equal(std::begin(kWaveSubformatTail),
                      std::end(kWaveSubformatTail),
                      data.begin() + static_cast<std::ptrdiff_t>(offset + 4));
}

float read_pcm_sample(const std::vector<std::uint8_t>& data, std::size_t offset, std::uint16_t bits_per_sample) {
    if (bits_per_sample == 16) {
        const auto value = static_cast<std::int16_t>(read_u16(data, offset));
        return static_cast<float>(value) / 32768.0f;
    }
    if (bits_per_sample == 24) {
        std::int32_t value = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(data[offset]) |
            (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
            (static_cast<std::uint32_t>(data[offset + 2]) << 16));
        if ((value & 0x00800000) != 0) {
            value |= static_cast<std::int32_t>(0xff000000);
        }
        return static_cast<float>(value) / 8388608.0f;
    }
    if (bits_per_sample == 32) {
        const auto value = static_cast<std::int32_t>(read_u32(data, offset));
        return static_cast<float>(value) / 2147483648.0f;
    }
    throw std::runtime_error("unsupported PCM bit depth");
}

float read_float32_sample(const std::vector<std::uint8_t>& data, std::size_t offset) {
    static_assert(sizeof(float) == 4, "float must be 32-bit");
    std::uint32_t raw = read_u32(data, offset);
    float value = 0.0f;
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

std::int16_t float_to_pcm16(float sample) {
    const float clamped = std::max(-1.0f, std::min(1.0f, sample));
    return static_cast<std::int16_t>(std::lround(clamped * 32767.0f));
}

std::uint32_t checked_u32(std::size_t value, const char* field) {
    if (value > 0xffffffffu) {
        throw std::runtime_error(std::string(field) + " is too large for WAV");
    }
    return static_cast<std::uint32_t>(value);
}

}  // namespace

AudioBuffer read_wav_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("failed to open WAV file: " + path);
    }

    std::vector<std::uint8_t> data(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    if (data.size() < 44 || !chunk_id_is(data, 0, "RIFF") || !chunk_id_is(data, 8, "WAVE")) {
        throw std::runtime_error("not a RIFF/WAVE file: " + path);
    }

    std::uint16_t audio_format = 0;
    std::uint16_t channels = 0;
    std::uint32_t sample_rate = 0;
    std::uint16_t bits_per_sample = 0;
    std::size_t data_offset = 0;
    std::size_t data_size = 0;

    std::size_t offset = 12;
    while (offset + 8 <= data.size()) {
        const std::uint32_t chunk_size = read_u32(data, offset + 4);
        const std::size_t payload = offset + 8;
        if (payload + chunk_size > data.size()) {
            throw std::runtime_error("WAV chunk extends past end of file");
        }

        if (chunk_id_is(data, offset, "fmt ")) {
            if (chunk_size < 16) {
                throw std::runtime_error("WAV fmt chunk is too small");
            }
            audio_format = read_u16(data, payload);
            channels = read_u16(data, payload + 2);
            sample_rate = read_u32(data, payload + 4);
            bits_per_sample = read_u16(data, payload + 14);
            if (audio_format == kWaveFormatExtensible) {
                if (chunk_size < 40) {
                    throw std::runtime_error("WAVE_FORMAT_EXTENSIBLE fmt chunk is too small");
                }
                const std::uint16_t extension_size = read_u16(data, payload + 16);
                if (extension_size < 22) {
                    throw std::runtime_error("WAVE_FORMAT_EXTENSIBLE extension is too small");
                }
                const std::size_t subformat_offset = payload + 24;
                if (extensible_subformat_is(data, subformat_offset, kWaveFormatPcm)) {
                    audio_format = kWaveFormatPcm;
                } else if (extensible_subformat_is(data, subformat_offset, kWaveFormatIeeeFloat)) {
                    audio_format = kWaveFormatIeeeFloat;
                } else {
                    throw std::runtime_error("unsupported WAVE_FORMAT_EXTENSIBLE subformat");
                }
            }
        } else if (chunk_id_is(data, offset, "data")) {
            data_offset = payload;
            data_size = chunk_size;
        }

        offset = payload + chunk_size + (chunk_size % 2);
    }

    if (audio_format == 0 || channels == 0 || sample_rate == 0 || bits_per_sample == 0 || data_size == 0) {
        throw std::runtime_error("WAV file is missing fmt or data chunk");
    }
    if (audio_format != kWaveFormatPcm && audio_format != kWaveFormatIeeeFloat) {
        throw std::runtime_error("only PCM, IEEE float, and WAVE_FORMAT_EXTENSIBLE PCM/float WAV files are supported");
    }
    if (audio_format == kWaveFormatIeeeFloat && bits_per_sample != 32) {
        throw std::runtime_error("only float32 IEEE WAV files are supported");
    }

    const std::size_t bytes_per_sample = bits_per_sample / 8;
    if (bytes_per_sample == 0 || data_size % bytes_per_sample != 0) {
        throw std::runtime_error("invalid WAV data size");
    }

    AudioBuffer audio;
    audio.sample_rate = static_cast<int>(sample_rate);
    audio.channels = static_cast<int>(channels);
    audio.samples.reserve(data_size / bytes_per_sample);
    for (std::size_t sample_offset = data_offset; sample_offset + bytes_per_sample <= data_offset + data_size;
         sample_offset += bytes_per_sample) {
        if (audio_format == kWaveFormatIeeeFloat) {
            audio.samples.push_back(read_float32_sample(data, sample_offset));
        } else {
            audio.samples.push_back(read_pcm_sample(data, sample_offset, bits_per_sample));
        }
    }
    return audio;
}

void write_wav_file(const std::string& path, const AudioBuffer& audio) {
    if (audio.sample_rate <= 0) {
        throw std::invalid_argument("sample_rate must be positive");
    }
    if (audio.channels <= 0) {
        throw std::invalid_argument("channels must be positive");
    }
    if (audio.samples.size() % static_cast<std::size_t>(audio.channels) != 0) {
        throw std::invalid_argument("sample count must be divisible by channel count");
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to create WAV file: " + path);
    }

    const std::uint16_t bits_per_sample = 16;
    const std::uint16_t block_align = static_cast<std::uint16_t>(audio.channels * bits_per_sample / 8);
    const std::uint32_t byte_rate = static_cast<std::uint32_t>(audio.sample_rate * block_align);
    const std::uint32_t data_size = checked_u32(audio.samples.size() * sizeof(std::int16_t), "data chunk");
    const std::uint32_t riff_size = 36u + data_size;

    out.write("RIFF", 4);
    write_u32(out, riff_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    write_u32(out, 16);
    write_u16(out, 1);
    write_u16(out, static_cast<std::uint16_t>(audio.channels));
    write_u32(out, static_cast<std::uint32_t>(audio.sample_rate));
    write_u32(out, byte_rate);
    write_u16(out, block_align);
    write_u16(out, bits_per_sample);
    out.write("data", 4);
    write_u32(out, data_size);

    for (float sample : audio.samples) {
        write_u16(out, static_cast<std::uint16_t>(float_to_pcm16(sample)));
    }
}

std::vector<float> downmix_to_mono(const AudioBuffer& audio) {
    if (audio.channels <= 0) {
        throw std::invalid_argument("channels must be positive");
    }
    if (audio.samples.size() % static_cast<std::size_t>(audio.channels) != 0) {
        throw std::invalid_argument("sample count must be divisible by channel count");
    }
    if (audio.channels == 1) {
        return audio.samples;
    }

    const std::size_t frames = audio.samples.size() / static_cast<std::size_t>(audio.channels);
    std::vector<float> mono(frames, 0.0f);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        double sum = 0.0;
        for (int channel = 0; channel < audio.channels; ++channel) {
            sum += audio.samples[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(channel)];
        }
        mono[frame] = static_cast<float>(sum / static_cast<double>(audio.channels));
    }
    return mono;
}

AudioBuffer make_mono_audio(int sample_rate, std::vector<float> samples) {
    AudioBuffer audio;
    audio.sample_rate = sample_rate;
    audio.channels = 1;
    audio.samples = std::move(samples);
    return audio;
}

}  // namespace music_elf
