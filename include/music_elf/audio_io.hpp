#ifndef MUSIC_ELF_AUDIO_IO_HPP
#define MUSIC_ELF_AUDIO_IO_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace music_elf {

struct AudioBuffer {
    int sample_rate = 48000;
    int channels = 1;
    std::vector<float> samples;
};

AudioBuffer read_wav_file(const std::string& path);
void write_wav_file(const std::string& path, const AudioBuffer& audio);

std::vector<float> downmix_to_mono(const AudioBuffer& audio);
AudioBuffer make_mono_audio(int sample_rate, std::vector<float> samples);

}  // namespace music_elf

#endif  // MUSIC_ELF_AUDIO_IO_HPP

