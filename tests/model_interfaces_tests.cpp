#include "music_elf/model_interfaces.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_unsupported_statuses_are_explicit() {
    const auto source = music_elf::unsupported_model_status(music_elf::ModelFeature::SourceSeparation);
    const auto lyrics = music_elf::unsupported_model_status(music_elf::ModelFeature::UnknownLyricsAsr);
    const auto midi = music_elf::unsupported_model_status(music_elf::ModelFeature::NeuralAudioToMidi);

    require(!source.available, "source separation should be unavailable by default");
    require(!lyrics.available, "lyrics ASR should be unavailable by default");
    require(!midi.available, "neural audio-to-MIDI should be unavailable by default");
    require(source.message.find("external model provider") != std::string::npos, "source status message");
    require(std::string(music_elf::model_feature_name(music_elf::ModelFeature::Orchestration)) == "orchestration",
            "feature name");
}

}  // namespace

int main() {
    try {
        test_unsupported_statuses_are_explicit();
    } catch (const std::exception& error) {
        std::cerr << "model_interfaces_tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "model_interfaces_tests passed\n";
    return EXIT_SUCCESS;
}

