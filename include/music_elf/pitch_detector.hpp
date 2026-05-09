#ifndef MUSIC_ELF_PITCH_DETECTOR_HPP
#define MUSIC_ELF_PITCH_DETECTOR_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace music_elf {

struct PitchDetectorConfig {
    int sample_rate = 48000;
    float min_frequency_hz = 50.0f;
    float max_frequency_hz = 1000.0f;
    std::size_t frame_size = 1024;
    std::size_t hop_size = 128;
    float yin_threshold = 0.12f;
    float silence_rms = 0.0005f;
};

struct PitchEstimate {
    double time_seconds = 0.0;
    float frequency_hz = 0.0f;
    int midi_note = 0;
    float cents = 0.0f;
    float confidence = 0.0f;
    bool voiced = false;
};

class PitchDetector {
public:
    explicit PitchDetector(const PitchDetectorConfig& config);

    const PitchDetectorConfig& config() const noexcept;

    std::size_t process(
        const float* samples,
        std::size_t count,
        PitchEstimate* out,
        std::size_t out_capacity);

private:
    PitchEstimate analyze_current_frame(std::uint64_t frame_start_sample);

    PitchDetectorConfig config_;
    int downsample_factor_ = 1;
    int analysis_sample_rate_ = 48000;
    std::size_t analysis_frame_size_ = 1024;
    std::size_t min_tau_ = 1;
    std::size_t max_tau_ = 1;

    std::size_t ring_write_index_ = 0;
    std::size_t samples_in_ring_ = 0;
    std::uint64_t samples_seen_ = 0;
    std::uint64_t next_frame_start_ = 0;

    std::vector<float> ring_;
    std::vector<float> raw_frame_;
    std::vector<float> analysis_frame_;
    std::vector<float> difference_;
    std::vector<float> cmnd_;
};

}  // namespace music_elf

#endif  // MUSIC_ELF_PITCH_DETECTOR_HPP
