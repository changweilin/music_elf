#include "music_elf/audio_renderer.hpp"
#include "music_elf/core_pipeline.hpp"
#include "music_elf/midi_catalog.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using music_elf::AccompanimentPattern;
using music_elf::AudioRendererConfig;
using music_elf::CatalogChordType;
using music_elf::CatalogRoot;
using music_elf::CorePipelineConfig;
using music_elf::GmInstrument;
using music_elf::LyricToken;
using music_elf::MidiCatalogConfig;

struct PipelineOptions {
    std::vector<LyricToken> lyrics;
    CorePipelineConfig config;
};

struct PitchStabilitySummary {
    std::size_t voiced_frames = 0;
    std::size_t stable_frames = 0;
    double mean_abs_cents = 0.0;
    double stable_ratio = 0.0;
};

void print_usage() {
    std::cout
        << "Usage: music_elf_cli <input.wav> [--out-midi file.mid] [--out-musicxml file.musicxml]\n"
        << "                     [--out-wav vocal_band.wav]\n"
        << "                     [--lyrics \"word word\"] [--pattern block|arpeggio|broken|pad]\n"
        << "                     [--analysis-wav file.wav]\n"
        << "       music_elf_cli inspect <input.wav> [--out-summary file]\n"
        << "                     [--lyrics \"word word\"] [--pattern block|arpeggio|broken|pad]\n"
        << "                     [--analysis-wav file.wav]\n"
        << "       music_elf_cli benchmark <input.wav> [--iterations count] [--out-summary file]\n"
        << "                     [--lyrics \"word word\"] [--pattern block|arpeggio|broken|pad]\n"
        << "                     [--analysis-wav file.wav]\n"
        << "       music_elf_cli render-preview <input.wav> [--out-wav preview.wav]\n"
        << "                     [--lyrics \"word word\"] [--pattern block|arpeggio|broken|pad]\n"
        << "                     [--waveform sine|square|saw|triangle]\n"
        << "                     [--analysis-wav file.wav]\n"
        << "       music_elf_cli render-beats <input.wav> [--out-wav beats.wav]\n"
        << "                     [--click-gain 0.5] [--source-gain 1.0]\n"
        << "                     [--beat-freq 1000] [--downbeat-freq 1600] [--click-duration 0.06]\n"
        << "                     [--analysis-wav file.wav]\n"
        << "       music_elf_cli generate-catalog [--out-dir dir]\n"
        << "                     [--instruments all|0,40] [--roots all|C,D,E]\n"
        << "                     [--chords all|major,minor,dom7] [--duration seconds]\n"
        << "                     [--bpm bpm] [--octave octave]\n"
        << "       music_elf_cli render-demo [--out-wav preview.wav]\n"
        << "                     [--program 0] [--root C] [--chord major]\n"
        << "                     [--waveform sine|square|saw|triangle]\n";
}

std::vector<LyricToken> split_lyrics(const std::string& text) {
    std::vector<LyricToken> tokens;
    std::istringstream stream(text);
    std::string token;
    while (stream >> token) {
        tokens.push_back({token});
    }
    return tokens;
}

AccompanimentPattern parse_pattern(const std::string& value) {
    if (value == "block") {
        return AccompanimentPattern::BlockChord;
    }
    if (value == "arpeggio") {
        return AccompanimentPattern::Arpeggio;
    }
    if (value == "broken") {
        return AccompanimentPattern::BrokenChord;
    }
    if (value == "pad") {
        return AccompanimentPattern::Pad;
    }
    throw std::invalid_argument("unknown pattern: " + value);
}

music_elf::PreviewWaveform parse_waveform(const std::string& value) {
    if (value == "sine") {
        return music_elf::PreviewWaveform::Sine;
    }
    if (value == "square") {
        return music_elf::PreviewWaveform::Square;
    }
    if (value == "saw") {
        return music_elf::PreviewWaveform::Saw;
    }
    if (value == "triangle") {
        return music_elf::PreviewWaveform::Triangle;
    }
    throw std::invalid_argument("unknown waveform: " + value);
}

std::string require_value(int& index, int argc, char** argv, const std::string& arg) {
    if (index + 1 >= argc) {
        throw std::invalid_argument("missing value for argument: " + arg);
    }
    return argv[++index];
}

bool parse_pipeline_option(const std::string& arg, int& index, int argc, char** argv, PipelineOptions& options) {
    if (arg == "--lyrics") {
        options.lyrics = split_lyrics(require_value(index, argc, argv, arg));
        return true;
    }
    if (arg == "--pattern") {
        options.config.accompaniment.pattern = parse_pattern(require_value(index, argc, argv, arg));
        return true;
    }
    return false;
}

bool parse_audio_input_option(
    const std::string& arg,
    int& index,
    int argc,
    char** argv,
    std::string& analysis_wav_path) {
    if (arg == "--analysis-wav") {
        analysis_wav_path = require_value(index, argc, argv, arg);
        return true;
    }
    return false;
}

void write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to write file: " + path);
    }
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_text_file(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to write file: " + path);
    }
    out << text;
}

std::vector<std::string> split_csv(const std::string& value) {
    std::vector<std::string> items;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ',')) {
        if (!item.empty()) {
            items.push_back(item);
        }
    }
    return items;
}

std::vector<GmInstrument> select_instruments(const std::string& value) {
    const auto& all = music_elf::gm_instruments();
    if (value == "all") {
        return all;
    }

    std::vector<GmInstrument> selected;
    for (const auto& item : split_csv(value)) {
        const int program = std::stoi(item);
        const auto found = std::find_if(
            all.begin(),
            all.end(),
            [program](const GmInstrument& instrument) {
                return instrument.program == program;
            });
        if (found == all.end()) {
            throw std::invalid_argument("unknown GM program: " + item);
        }
        selected.push_back(*found);
    }
    return selected;
}

std::vector<CatalogRoot> select_roots(const std::string& value) {
    const auto& all = music_elf::catalog_roots();
    if (value == "all") {
        return all;
    }

    std::vector<CatalogRoot> selected;
    for (const auto& item : split_csv(value)) {
        const auto found = std::find_if(
            all.begin(),
            all.end(),
            [&item](const CatalogRoot& root) {
                return root.name == item;
            });
        if (found == all.end()) {
            throw std::invalid_argument("unknown root: " + item);
        }
        selected.push_back(*found);
    }
    return selected;
}

std::vector<CatalogChordType> select_chords(const std::string& value) {
    const auto& all = music_elf::catalog_chord_types();
    if (value == "all") {
        return all;
    }

    std::vector<CatalogChordType> selected;
    for (const auto& item : split_csv(value)) {
        const auto found = std::find_if(
            all.begin(),
            all.end(),
            [&item](const CatalogChordType& chord) {
                return chord.id == item;
            });
        if (found == all.end()) {
            throw std::invalid_argument("unknown chord type: " + item);
        }
        selected.push_back(*found);
    }
    return selected;
}

PitchStabilitySummary summarize_pitch_stability(
    const std::vector<music_elf::PitchEstimate>& estimates) {
    PitchStabilitySummary summary;
    double total_abs_cents = 0.0;
    for (const auto& estimate : estimates) {
        if (!estimate.voiced || estimate.midi_note <= 0 ||
            !std::isfinite(estimate.cents)) {
            continue;
        }
        summary.voiced_frames += 1;
        const double abs_cents = std::fabs(static_cast<double>(estimate.cents));
        total_abs_cents += abs_cents;
        if (abs_cents <= 25.0) {
            summary.stable_frames += 1;
        }
    }
    if (summary.voiced_frames > 0) {
        summary.mean_abs_cents =
            total_abs_cents / static_cast<double>(summary.voiced_frames);
        summary.stable_ratio =
            static_cast<double>(summary.stable_frames) /
            static_cast<double>(summary.voiced_frames);
    }
    return summary;
}

void write_pipeline_summary(
    std::ostream& out,
    const std::string& command,
    const std::string& input_path,
    const std::string& analysis_input_path,
    const music_elf::AudioBuffer& audio,
    const music_elf::CorePipelineResult& result) {
    const std::size_t frames = audio.channels > 0
                                   ? audio.samples.size() / static_cast<std::size_t>(audio.channels)
                                   : 0;
    const double duration_seconds = audio.sample_rate > 0
                                        ? static_cast<double>(frames) / static_cast<double>(audio.sample_rate)
                                        : 0.0;
    const auto voiced_frames = std::count_if(
        result.pitch_estimates.begin(),
        result.pitch_estimates.end(),
        [](const music_elf::PitchEstimate& estimate) {
            return estimate.voiced;
        });
    const auto stability = summarize_pitch_stability(result.pitch_estimates);

    out << std::fixed << std::setprecision(3);
    out << "command: " << command << "\n";
    out << "input: " << input_path << "\n";
    if (analysis_input_path != input_path) {
        out << "analysis_input: " << analysis_input_path << "\n";
    }
    out << "sample_rate: " << audio.sample_rate << "\n";
    out << "channels: " << audio.channels << "\n";
    out << "duration_seconds: " << duration_seconds << "\n";
    out << "pitch_frames: " << result.pitch_estimates.size() << "\n";
    out << "voiced_pitch_frames: " << voiced_frames << "\n";
    out << "pitch_stability_cents: " << stability.mean_abs_cents << "\n";
    out << "stable_pitch_frames: " << stability.stable_frames << "\n";
    out << "stable_pitch_frame_ratio: " << stability.stable_ratio << "\n";
    out << "notes: " << result.notes.size() << "\n";
    out << "estimated_bpm: " << result.rhythm.beat_grid.bpm << "\n";
    out << "key: " << music_elf::note_name(result.key.tonic_pitch_class) << " "
        << music_elf::scale_mode_name(result.key.mode) << "\n";
    out << "key_confidence: " << result.key.confidence << "\n";
    out << "chord_progressions: " << result.chord_progressions.size() << "\n";
    if (!result.chord_progressions.empty() && !result.chord_progressions.front().chords.empty()) {
        out << "top_progression_style: "
            << music_elf::harmony_style_name(result.chord_progressions.front().style) << "\n";
        out << "first_chord: " << music_elf::chord_symbol(result.chord_progressions.front().chords.front())
            << "\n";
    }
    out << "accompaniment_notes: " << result.accompaniment_notes.size() << "\n";
    out << "lyric_alignments: " << result.lyric_alignments.size() << "\n";
    out << "midi_bytes: " << result.midi_bytes.size() << "\n";
    out << "musicxml_chars: " << result.musicxml.size() << "\n";
    out << "instrumental_samples: " << result.instrumental_audio.samples.size() << "\n";
    out << "vocal_band_samples: " << result.vocal_band_audio.samples.size() << "\n";
}

void emit_summary(const std::string& summary, const std::string& output_path) {
    std::cout << summary;
    if (!output_path.empty()) {
        write_text_file(output_path, summary);
    }
}

int run_inspect(int argc, char** argv) {
    if (argc < 3) {
        throw std::invalid_argument("inspect requires an input WAV path");
    }
    std::string input_path = argv[2];
    std::string analysis_wav_path;
    std::string out_summary;
    PipelineOptions options;

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--out-summary") {
            out_summary = require_value(i, argc, argv, arg);
        } else if (parse_audio_input_option(arg, i, argc, argv, analysis_wav_path)) {
        } else if (!parse_pipeline_option(arg, i, argc, argv, options)) {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }

    const std::string analysis_path = analysis_wav_path.empty() ? input_path : analysis_wav_path;
    const auto audio = music_elf::read_wav_file(analysis_path);
    const auto result = music_elf::run_core_pipeline(audio, options.lyrics, options.config);

    std::ostringstream summary;
    write_pipeline_summary(summary, "inspect", input_path, analysis_path, audio, result);
    emit_summary(summary.str(), out_summary);
    return 0;
}

int run_benchmark(int argc, char** argv) {
    if (argc < 3) {
        throw std::invalid_argument("benchmark requires an input WAV path");
    }
    std::string input_path = argv[2];
    std::string analysis_wav_path;
    std::string out_summary;
    int iterations = 3;
    PipelineOptions options;

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--iterations") {
            iterations = std::stoi(require_value(i, argc, argv, arg));
        } else if (arg == "--out-summary") {
            out_summary = require_value(i, argc, argv, arg);
        } else if (parse_audio_input_option(arg, i, argc, argv, analysis_wav_path)) {
        } else if (!parse_pipeline_option(arg, i, argc, argv, options)) {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }
    if (iterations <= 0) {
        throw std::invalid_argument("iterations must be positive");
    }

    const std::string analysis_path = analysis_wav_path.empty() ? input_path : analysis_wav_path;
    const auto audio = music_elf::read_wav_file(analysis_path);
    music_elf::CorePipelineResult last_result;
    double total_ms = 0.0;
    double min_ms = std::numeric_limits<double>::max();
    double max_ms = 0.0;

    for (int iteration = 0; iteration < iterations; ++iteration) {
        const auto start = std::chrono::steady_clock::now();
        last_result = music_elf::run_core_pipeline(audio, options.lyrics, options.config);
        const auto stop = std::chrono::steady_clock::now();
        const double elapsed_ms =
            std::chrono::duration<double, std::milli>(stop - start).count();
        total_ms += elapsed_ms;
        min_ms = std::min(min_ms, elapsed_ms);
        max_ms = std::max(max_ms, elapsed_ms);
    }

    std::ostringstream summary;
    write_pipeline_summary(summary, "benchmark", input_path, analysis_path, audio, last_result);
    summary << std::fixed << std::setprecision(3);
    summary << "benchmark_iterations: " << iterations << "\n";
    summary << "benchmark_average_ms: " << (total_ms / static_cast<double>(iterations)) << "\n";
    summary << "benchmark_min_ms: " << min_ms << "\n";
    summary << "benchmark_max_ms: " << max_ms << "\n";
    emit_summary(summary.str(), out_summary);
    return 0;
}

int run_render_preview(int argc, char** argv) {
    if (argc < 3) {
        throw std::invalid_argument("render-preview requires an input WAV path");
    }
    std::string input_path = argv[2];
    std::string analysis_wav_path;
    std::string out_wav = "music_elf_pipeline_preview.wav";
    PipelineOptions options;
    AudioRendererConfig render_config;

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--out-wav") {
            out_wav = require_value(i, argc, argv, arg);
        } else if (arg == "--waveform") {
            render_config.waveform = parse_waveform(require_value(i, argc, argv, arg));
        } else if (parse_audio_input_option(arg, i, argc, argv, analysis_wav_path)) {
        } else if (!parse_pipeline_option(arg, i, argc, argv, options)) {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }

    const std::string analysis_path = analysis_wav_path.empty() ? input_path : analysis_wav_path;
    const auto audio = music_elf::read_wav_file(analysis_path);
    render_config.sample_rate = audio.sample_rate;
    options.config.renderer = render_config;
    options.config.render_preview_audio = true;
    const auto result = music_elf::run_core_pipeline(audio, options.lyrics, options.config);
    music_elf::write_wav_file(out_wav, result.vocal_band_audio);

    std::cout << "Rendered vocal-band preview WAV\n";
    std::cout << "Input: " << input_path << "\n";
    if (analysis_path != input_path) {
        std::cout << "Analysis input: " << analysis_path << "\n";
    }
    std::cout << "Waveform: " << music_elf::preview_waveform_name(render_config.waveform) << "\n";
    std::cout << "Band notes: " << result.accompaniment_notes.size() << "\n";
    std::cout << "Vocal-band samples: " << result.vocal_band_audio.samples.size() << "\n";
    std::cout << "Output: " << out_wav << "\n";
    return 0;
}

int run_generate_catalog(int argc, char** argv) {
    std::filesystem::path out_dir = "midi_catalog";
    std::string instrument_filter = "all";
    std::string root_filter = "all";
    std::string chord_filter = "all";
    MidiCatalogConfig config;

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--out-dir" && i + 1 < argc) {
            out_dir = require_value(i, argc, argv, arg);
        } else if (arg == "--instruments" && i + 1 < argc) {
            instrument_filter = require_value(i, argc, argv, arg);
        } else if (arg == "--roots" && i + 1 < argc) {
            root_filter = require_value(i, argc, argv, arg);
        } else if (arg == "--chords" && i + 1 < argc) {
            chord_filter = require_value(i, argc, argv, arg);
        } else if (arg == "--duration" && i + 1 < argc) {
            config.duration_seconds = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--bpm" && i + 1 < argc) {
            config.bpm = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--octave" && i + 1 < argc) {
            config.root_octave = std::stoi(require_value(i, argc, argv, arg));
        } else {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }

    const auto instruments = select_instruments(instrument_filter);
    const auto roots = select_roots(root_filter);
    const auto chords = select_chords(chord_filter);
    std::filesystem::create_directories(out_dir);

    std::size_t written = 0;
    for (const auto& instrument : instruments) {
        const std::filesystem::path instrument_dir =
            out_dir / (std::to_string(instrument.program) + "_" + instrument.name);
        std::filesystem::create_directories(instrument_dir);
        for (const auto& root : roots) {
            for (const auto& chord : chords) {
                const auto bytes = music_elf::make_catalog_chord_midi(instrument, root, chord, config);
                const std::filesystem::path path =
                    instrument_dir / music_elf::midi_catalog_file_name(instrument, root, chord);
                write_binary_file(path.string(), bytes);
                written += 1;
            }
        }
    }

    std::cout << "Generated MIDI catalog\n";
    std::cout << "Output dir: " << out_dir.string() << "\n";
    std::cout << "Instruments: " << instruments.size() << "\n";
    std::cout << "Roots: " << roots.size() << "\n";
    std::cout << "Chord types: " << chords.size() << "\n";
    std::cout << "Files: " << written << "\n";
    return 0;
}

void mix_click_into_audio(
    music_elf::AudioBuffer& audio,
    double click_time_seconds,
    double frequency_hz,
    double duration_seconds,
    float gain) {
    if (audio.sample_rate <= 0 || audio.channels <= 0 || audio.samples.empty()) {
        return;
    }
    const int channels = audio.channels;
    const std::size_t total_frames = audio.samples.size() / static_cast<std::size_t>(channels);
    const auto start_frame_signed =
        static_cast<long long>(std::llround(click_time_seconds * audio.sample_rate));
    if (start_frame_signed < 0) {
        return;
    }
    const std::size_t start_frame = static_cast<std::size_t>(start_frame_signed);
    if (start_frame >= total_frames) {
        return;
    }
    const std::size_t click_frames = static_cast<std::size_t>(
        std::max(1.0, duration_seconds * audio.sample_rate));
    const std::size_t end_frame = std::min(total_frames, start_frame + click_frames);
    const double decay_constant = 25.0;  // ~40 ms perceived tail
    constexpr double kTwoPi = 6.28318530717958647692;
    for (std::size_t frame = start_frame; frame < end_frame; ++frame) {
        const double t = static_cast<double>(frame - start_frame) / static_cast<double>(audio.sample_rate);
        const double envelope = std::exp(-decay_constant * t);
        const double sample = std::sin(kTwoPi * frequency_hz * t) * envelope * gain;
        const std::size_t base_index = frame * static_cast<std::size_t>(channels);
        for (int channel = 0; channel < channels; ++channel) {
            float& target = audio.samples[base_index + static_cast<std::size_t>(channel)];
            const double mixed = static_cast<double>(target) + sample;
            target = static_cast<float>(std::clamp(mixed, -1.0, 1.0));
        }
    }
}

int run_render_beats(int argc, char** argv) {
    if (argc < 3) {
        throw std::invalid_argument("render-beats requires an input WAV path");
    }
    std::string input_path = argv[2];
    std::string analysis_wav_path;
    std::string out_wav;
    PipelineOptions options;
    double click_gain = 0.5;
    double source_gain = 1.0;
    double beat_freq = 1000.0;
    double downbeat_freq = 1600.0;
    double click_duration_seconds = 0.06;

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--out-wav") {
            out_wav = require_value(i, argc, argv, arg);
        } else if (arg == "--click-gain") {
            click_gain = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--source-gain") {
            source_gain = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--beat-freq") {
            beat_freq = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--downbeat-freq") {
            downbeat_freq = std::stod(require_value(i, argc, argv, arg));
        } else if (arg == "--click-duration") {
            click_duration_seconds = std::stod(require_value(i, argc, argv, arg));
        } else if (parse_audio_input_option(arg, i, argc, argv, analysis_wav_path)) {
        } else if (!parse_pipeline_option(arg, i, argc, argv, options)) {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }

    if (out_wav.empty()) {
        std::filesystem::path base(input_path);
        out_wav = base.replace_extension("").string() + "_beats.wav";
    }

    const std::string analysis_path = analysis_wav_path.empty() ? input_path : analysis_wav_path;
    auto audio = music_elf::read_wav_file(analysis_path);
    const auto result = music_elf::run_core_pipeline(audio, options.lyrics, options.config);
    const auto& beats = result.rhythm.beat_grid.beat_times_seconds;
    const int beats_per_bar = std::max(1, result.rhythm.beat_grid.beats_per_bar);

    if (source_gain != 1.0) {
        for (auto& sample : audio.samples) {
            const double scaled = static_cast<double>(sample) * source_gain;
            sample = static_cast<float>(std::clamp(scaled, -1.0, 1.0));
        }
    }

    for (std::size_t i = 0; i < beats.size(); ++i) {
        const bool is_downbeat = (static_cast<int>(i) % beats_per_bar) == 0;
        const double frequency = is_downbeat ? downbeat_freq : beat_freq;
        const float gain = is_downbeat
                               ? static_cast<float>(click_gain)
                               : static_cast<float>(click_gain * 0.75);
        mix_click_into_audio(audio, beats[i], frequency, click_duration_seconds, gain);
    }

    music_elf::write_wav_file(out_wav, audio);

    std::cout << "Rendered beat-overlay WAV\n";
    std::cout << "Input: " << input_path << "\n";
    if (analysis_path != input_path) {
        std::cout << "Analysis input: " << analysis_path << "\n";
    }
    std::cout << "Estimated BPM: " << result.rhythm.beat_grid.bpm << "\n";
    std::cout << "Beats per bar: " << beats_per_bar << "\n";
    std::cout << "Beats: " << beats.size() << "\n";
    std::cout << "Output: " << out_wav << "\n";
    return 0;
}

int run_render_demo(int argc, char** argv) {
    std::string out_wav = "music_elf_preview.wav";
    int program = 0;
    std::string root_name = "C";
    std::string chord_id = "major";
    MidiCatalogConfig catalog_config;
    AudioRendererConfig render_config;

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--out-wav" && i + 1 < argc) {
            out_wav = require_value(i, argc, argv, arg);
        } else if (arg == "--program" && i + 1 < argc) {
            program = std::stoi(require_value(i, argc, argv, arg));
        } else if (arg == "--root" && i + 1 < argc) {
            root_name = require_value(i, argc, argv, arg);
        } else if (arg == "--chord" && i + 1 < argc) {
            chord_id = require_value(i, argc, argv, arg);
        } else if (arg == "--waveform" && i + 1 < argc) {
            render_config.waveform = parse_waveform(require_value(i, argc, argv, arg));
        } else {
            throw std::invalid_argument("unknown or incomplete argument: " + arg);
        }
    }

    const auto instruments = select_instruments(std::to_string(program));
    const auto roots = select_roots(root_name);
    const auto chords = select_chords(chord_id);
    const GmInstrument& instrument = instruments.front();
    const CatalogRoot& root = roots.front();
    const CatalogChordType& chord = chords.front();

    const int root_midi = (catalog_config.root_octave + 1) * 12 + root.pitch_class;
    std::vector<music_elf::GeneratedNote> notes;
    for (int interval : chord.intervals) {
        music_elf::GeneratedNote note;
        note.start_seconds = 0.0;
        note.end_seconds = catalog_config.duration_seconds;
        note.midi_note = root_midi + interval;
        note.velocity = catalog_config.velocity;
        note.channel = catalog_config.channel;
        notes.push_back(note);
    }

    const auto audio = music_elf::render_notes_to_audio(notes.data(), notes.size(), render_config);
    music_elf::write_wav_file(out_wav, audio);

    std::cout << "Rendered preview WAV\n";
    std::cout << "Program: " << instrument.program << " " << instrument.name << "\n";
    std::cout << "Chord: " << root.name << " " << chord.display_name << "\n";
    std::cout << "Waveform: " << music_elf::preview_waveform_name(render_config.waveform) << "\n";
    std::cout << "Output: " << out_wav << "\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            print_usage();
            return argc < 2 ? 1 : 0;
        }
        if (std::string(argv[1]) == "generate-catalog") {
            return run_generate_catalog(argc, argv);
        }
        if (std::string(argv[1]) == "render-demo") {
            return run_render_demo(argc, argv);
        }
        if (std::string(argv[1]) == "inspect") {
            return run_inspect(argc, argv);
        }
        if (std::string(argv[1]) == "benchmark") {
            return run_benchmark(argc, argv);
        }
        if (std::string(argv[1]) == "render-preview") {
            return run_render_preview(argc, argv);
        }
        if (std::string(argv[1]) == "render-beats") {
            return run_render_beats(argc, argv);
        }

        std::string input_path = argv[1];
        std::filesystem::path base(input_path);
        std::string out_midi = base.replace_extension(".mid").string();
        base = std::filesystem::path(input_path);
        std::string out_musicxml = base.replace_extension(".musicxml").string();
        std::string out_wav;
        PipelineOptions options;
        std::string analysis_wav_path;

        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help" || arg == "-h") {
                print_usage();
                return 0;
            }
            if (arg == "--out-midi" && i + 1 < argc) {
                out_midi = require_value(i, argc, argv, arg);
            } else if (arg == "--out-musicxml" && i + 1 < argc) {
                out_musicxml = require_value(i, argc, argv, arg);
            } else if (arg == "--out-wav" && i + 1 < argc) {
                out_wav = require_value(i, argc, argv, arg);
            } else if (parse_audio_input_option(arg, i, argc, argv, analysis_wav_path)) {
            } else if (parse_pipeline_option(arg, i, argc, argv, options)) {
            } else {
                throw std::invalid_argument("unknown or incomplete argument: " + arg);
            }
        }

        const std::string analysis_path = analysis_wav_path.empty() ? input_path : analysis_wav_path;
        const auto audio = music_elf::read_wav_file(analysis_path);
        options.config.render_preview_audio = !out_wav.empty();
        const auto result = music_elf::run_core_pipeline(audio, options.lyrics, options.config);

        write_binary_file(out_midi, result.midi_bytes);
        write_text_file(out_musicxml, result.musicxml);
        if (!out_wav.empty()) {
            music_elf::write_wav_file(out_wav, result.vocal_band_audio);
        }

        std::cout << "Music Elf analysis complete\n";
        std::cout << "Input: " << input_path << "\n";
        if (analysis_path != input_path) {
            std::cout << "Analysis input: " << analysis_path << "\n";
        }
        std::cout << "Pitch frames: " << result.pitch_estimates.size() << "\n";
        std::cout << "Notes: " << result.notes.size() << "\n";
        std::cout << "Estimated BPM: " << result.rhythm.beat_grid.bpm << "\n";
        std::cout << "Chord candidates: " << result.chord_progressions.size() << "\n";
        std::cout << "Accompaniment notes: " << result.accompaniment_notes.size() << "\n";
        std::cout << "Wrote MIDI: " << out_midi << "\n";
        std::cout << "Wrote MusicXML: " << out_musicxml << "\n";
        if (!out_wav.empty()) {
            std::cout << "Wrote Vocal-band WAV: " << out_wav << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "music_elf_cli failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
