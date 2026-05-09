#ifndef MUSIC_ELF_MODEL_INTERFACES_HPP
#define MUSIC_ELF_MODEL_INTERFACES_HPP

#include "music_elf/audio_io.hpp"
#include "music_elf/lyric_aligner.hpp"
#include "music_elf/note_segmenter.hpp"

#include <string>
#include <vector>

namespace music_elf {

enum class ModelFeature {
    SourceSeparation,
    UnknownLyricsAsr,
    NeuralAudioToMidi,
    PolyphonicPitchDetection,
    Orchestration,
};

struct ModelSupportStatus {
    ModelFeature feature = ModelFeature::SourceSeparation;
    bool available = false;
    std::string provider_name;
    std::string message;
};

struct SeparatedStems {
    AudioBuffer vocals;
    AudioBuffer drums;
    AudioBuffer bass;
    AudioBuffer other;
};

struct TranscribedLyric {
    std::string text;
    double start_seconds = 0.0;
    double end_seconds = 0.0;
    float confidence = 0.0f;
};

class SourceSeparationModel {
public:
    virtual ~SourceSeparationModel() = default;
    virtual ModelSupportStatus status() const = 0;
    virtual SeparatedStems separate(const AudioBuffer& audio) = 0;
};

class LyricsTranscriptionModel {
public:
    virtual ~LyricsTranscriptionModel() = default;
    virtual ModelSupportStatus status() const = 0;
    virtual std::vector<TranscribedLyric> transcribe(const AudioBuffer& audio) = 0;
};

class NeuralAudioToMidiModel {
public:
    virtual ~NeuralAudioToMidiModel() = default;
    virtual ModelSupportStatus status() const = 0;
    virtual std::vector<NoteEvent> transcribe_notes(const AudioBuffer& audio) = 0;
};

ModelSupportStatus unsupported_model_status(ModelFeature feature);
const char* model_feature_name(ModelFeature feature) noexcept;

}  // namespace music_elf

#endif  // MUSIC_ELF_MODEL_INTERFACES_HPP

