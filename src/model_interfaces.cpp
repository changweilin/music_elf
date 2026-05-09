#include "music_elf/model_interfaces.hpp"

namespace music_elf {

const char* model_feature_name(ModelFeature feature) noexcept {
    switch (feature) {
        case ModelFeature::SourceSeparation:
            return "source separation";
        case ModelFeature::UnknownLyricsAsr:
            return "unknown lyrics ASR";
        case ModelFeature::NeuralAudioToMidi:
            return "neural audio-to-MIDI";
        case ModelFeature::PolyphonicPitchDetection:
            return "polyphonic pitch detection";
        case ModelFeature::Orchestration:
            return "orchestration";
    }
    return "model feature";
}

ModelSupportStatus unsupported_model_status(ModelFeature feature) {
    ModelSupportStatus status;
    status.feature = feature;
    status.available = false;
    status.provider_name = "none";
    status.message = std::string(model_feature_name(feature)) +
                     " requires an external model provider and is not implemented in the native MVP core";
    return status;
}

}  // namespace music_elf

