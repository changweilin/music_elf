#include "music_elf/pitch_detector.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kPi = 3.1415926535897932384626433832795;

using music_elf::PitchDetector;
using music_elf::PitchDetectorConfig;
using music_elf::PitchEstimate;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

double cents_between(double measured_hz, double expected_hz) {
    return 1200.0 * std::log2(measured_hz / expected_hz);
}

std::vector<float> make_sine(int sample_rate, double seconds, double frequency_hz, double amplitude = 0.5) {
    const auto count = static_cast<std::size_t>(std::llround(seconds * sample_rate));
    std::vector<float> samples(count);
    double phase = 0.0;
    const double increment = 2.0 * kPi * frequency_hz / static_cast<double>(sample_rate);
    for (auto& sample : samples) {
        sample = static_cast<float>(amplitude * std::sin(phase));
        phase += increment;
    }
    return samples;
}

std::vector<float> make_chirp(int sample_rate, double seconds, double start_hz, double end_hz) {
    const auto count = static_cast<std::size_t>(std::llround(seconds * sample_rate));
    std::vector<float> samples(count);
    double phase = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(sample_rate);
        const double progress = t / seconds;
        const double frequency = start_hz + (end_hz - start_hz) * progress;
        samples[i] = static_cast<float>(0.5 * std::sin(phase));
        phase += 2.0 * kPi * frequency / static_cast<double>(sample_rate);
    }
    return samples;
}

std::vector<float> make_vibrato(
    int sample_rate,
    double seconds,
    double center_hz,
    double depth_cents,
    double rate_hz) {
    const auto count = static_cast<std::size_t>(std::llround(seconds * sample_rate));
    std::vector<float> samples(count);
    double phase = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(sample_rate);
        const double cents = depth_cents * std::sin(2.0 * kPi * rate_hz * t);
        const double frequency = center_hz * std::pow(2.0, cents / 1200.0);
        samples[i] = static_cast<float>(0.5 * std::sin(phase));
        phase += 2.0 * kPi * frequency / static_cast<double>(sample_rate);
    }
    return samples;
}

std::vector<float> make_white_noise(int sample_rate, double seconds, double amplitude) {
    const auto count = static_cast<std::size_t>(std::llround(seconds * sample_rate));
    std::vector<float> samples(count);
    std::uint32_t state = 0x12345678u;
    for (auto& sample : samples) {
        state = state * 1664525u + 1013904223u;
        const double unit = static_cast<double>((state >> 8) & 0x00ffffffu) / 16777215.0;
        sample = static_cast<float>((unit * 2.0 - 1.0) * amplitude);
    }
    return samples;
}

std::vector<PitchEstimate> run_detector(
    const std::vector<float>& samples,
    const PitchDetectorConfig& config,
    std::size_t chunk_size) {
    PitchDetector detector(config);
    std::vector<PitchEstimate> estimates;
    std::vector<PitchEstimate> scratch(4096);

    for (std::size_t offset = 0; offset < samples.size(); offset += chunk_size) {
        const std::size_t count = std::min(chunk_size, samples.size() - offset);
        const std::size_t written = detector.process(
            samples.data() + offset,
            count,
            scratch.data(),
            scratch.size());
        estimates.insert(estimates.end(), scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(written));
    }

    return estimates;
}

std::vector<double> voiced_cents_errors(
    const std::vector<PitchEstimate>& estimates,
    double expected_hz,
    double start_time = 0.08,
    double end_time = 1000000.0) {
    std::vector<double> errors;
    for (const auto& estimate : estimates) {
        if (!estimate.voiced || estimate.time_seconds < start_time || estimate.time_seconds > end_time) {
            continue;
        }
        errors.push_back(std::fabs(cents_between(estimate.frequency_hz, expected_hz)));
    }
    return errors;
}

double median(std::vector<double> values) {
    require(!values.empty(), "cannot compute median of empty set");
    const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
    std::nth_element(values.begin(), middle, values.end());
    return *middle;
}

void test_sine_accuracy() {
    const struct Case {
        int sample_rate;
        double frequency;
        const char* name;
    } cases[] = {
        {48000, 440.0, "A4 48 kHz"},
        {48000, 261.625565, "C4 48 kHz"},
        {44100, 329.627557, "E4 44.1 kHz"},
        {44100, 440.0, "A4 44.1 kHz"},
    };

    for (const auto& test_case : cases) {
        PitchDetectorConfig config;
        config.sample_rate = test_case.sample_rate;
        const auto samples = make_sine(test_case.sample_rate, 1.5, test_case.frequency);
        const auto estimates = run_detector(samples, config, samples.size());
        const auto errors = voiced_cents_errors(estimates, test_case.frequency, 0.1, 1.4);
        require(errors.size() > 50, std::string(test_case.name) + " produced too few voiced frames");
        const double med = median(errors);
        require(med <= 5.0, std::string(test_case.name) + " median error was " + std::to_string(med));
    }
}

void test_chirp_accuracy() {
    PitchDetectorConfig config;
    config.sample_rate = 48000;
    const double seconds = 2.0;
    const double start_hz = 220.0;
    const double end_hz = 880.0;
    const auto samples = make_chirp(config.sample_rate, seconds, start_hz, end_hz);
    const auto estimates = run_detector(samples, config, 257);

    std::vector<double> errors;
    for (const auto& estimate : estimates) {
        if (!estimate.voiced || estimate.time_seconds < 0.12 || estimate.time_seconds > 1.9) {
            continue;
        }
        const double expected = start_hz + (end_hz - start_hz) * (estimate.time_seconds / seconds);
        errors.push_back(std::fabs(cents_between(estimate.frequency_hz, expected)));
    }
    require(errors.size() > 100, "chirp produced too few voiced frames");
    const double med = median(errors);
    require(med <= 20.0, "chirp median error was " + std::to_string(med));
}

void test_vibrato_tracking() {
    PitchDetectorConfig config;
    config.sample_rate = 48000;
    const auto samples = make_vibrato(config.sample_rate, 2.0, 440.0, 30.0, 5.0);
    const auto estimates = run_detector(samples, config, 128);

    int stable = 0;
    int voiced = 0;
    std::vector<double> centered_errors;
    for (const auto& estimate : estimates) {
        if (estimate.time_seconds < 0.1 || estimate.time_seconds > 1.9) {
            continue;
        }
        stable += 1;
        if (estimate.voiced) {
            voiced += 1;
            centered_errors.push_back(cents_between(estimate.frequency_hz, 440.0));
        }
    }

    require(stable > 100, "vibrato produced too few frames");
    const double voiced_ratio = static_cast<double>(voiced) / static_cast<double>(stable);
    require(voiced_ratio >= 0.95, "vibrato voiced ratio was " + std::to_string(voiced_ratio));
    const double mean = std::accumulate(centered_errors.begin(), centered_errors.end(), 0.0) /
                        static_cast<double>(centered_errors.size());
    require(std::fabs(mean) <= 8.0, "vibrato mean pitch drift was " + std::to_string(mean));
}

void test_unvoiced_inputs() {
    PitchDetectorConfig config;
    config.sample_rate = 48000;

    const std::vector<float> silence(static_cast<std::size_t>(config.sample_rate), 0.0f);
    const auto silence_estimates = run_detector(silence, config, 300);
    require(!silence_estimates.empty(), "silence produced no frames");
    require(std::none_of(silence_estimates.begin(), silence_estimates.end(),
                         [](const PitchEstimate& estimate) { return estimate.voiced; }),
            "silence should remain unvoiced");

    const auto noise = make_white_noise(config.sample_rate, 1.0, 0.4);
    const auto noise_estimates = run_detector(noise, config, 511);
    int voiced = 0;
    double confidence_sum = 0.0;
    for (const auto& estimate : noise_estimates) {
        if (estimate.voiced) {
            voiced += 1;
        }
        confidence_sum += estimate.confidence;
    }
    const double voiced_ratio = static_cast<double>(voiced) / static_cast<double>(noise_estimates.size());
    const double mean_confidence = confidence_sum / static_cast<double>(noise_estimates.size());
    require(voiced_ratio <= 0.05 || mean_confidence <= 0.35,
            "noise looked voiced: ratio=" + std::to_string(voiced_ratio) +
                " confidence=" + std::to_string(mean_confidence));
}

void test_streaming_consistency() {
    PitchDetectorConfig config;
    config.sample_rate = 48000;
    const auto samples = make_vibrato(config.sample_rate, 2.0, 440.0, 20.0, 4.0);
    const auto full = run_detector(samples, config, samples.size());

    for (std::size_t chunk : {std::size_t{64}, std::size_t{128}, std::size_t{511}}) {
        const auto streamed = run_detector(samples, config, chunk);
        require(streamed.size() == full.size(), "chunk " + std::to_string(chunk) + " changed frame count");
        for (std::size_t i = 0; i < full.size(); ++i) {
            require(streamed[i].voiced == full[i].voiced,
                    "chunk " + std::to_string(chunk) + " changed voiced flag");
            if (full[i].voiced) {
                const double diff = std::fabs(streamed[i].frequency_hz - full[i].frequency_hz);
                require(diff <= 0.001, "chunk " + std::to_string(chunk) + " changed frequency");
            }
        }
    }
}

void test_realtime_budget() {
    PitchDetectorConfig config;
    config.sample_rate = 48000;
    const double seconds = 10.0;
    auto samples = make_vibrato(config.sample_rate, seconds, 440.0, 25.0, 5.5);

    const auto start = std::chrono::high_resolution_clock::now();
    const auto estimates = run_detector(samples, config, 128);
    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed = std::chrono::duration<double>(end - start).count();
    const double realtime_factor = elapsed / seconds;
    std::cout << "processed " << seconds << "s in " << elapsed
              << "s, realtime factor=" << realtime_factor
              << ", frames=" << estimates.size() << '\n';

    require(!estimates.empty(), "performance test produced no estimates");
    require(elapsed < seconds, "processing exceeded realtime budget");
}

}  // namespace

int main() {
    try {
        test_sine_accuracy();
        test_chirp_accuracy();
        test_vibrato_tracking();
        test_unvoiced_inputs();
        test_streaming_consistency();
        test_realtime_budget();
    } catch (const std::exception& error) {
        std::cerr << "pitch_detector_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "pitch_detector_tests passed\n";
    return EXIT_SUCCESS;
}

