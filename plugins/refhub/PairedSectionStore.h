#pragma once

#include "shared/state/PairedSectionModel.h"

#include <juce_core/juce_core.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ehvota::refhub {

class PairedSectionStore {
public:
    static constexpr std::size_t maximumSections = 64;

    [[nodiscard]] bool replace(
        std::vector<ehvota::state::PairedSection> sections,
        std::optional<double> mixDurationSeconds = std::nullopt,
        std::optional<double> referenceDurationSeconds = std::nullopt);

    [[nodiscard]] std::vector<ehvota::state::PairedSection> copy() const;
    [[nodiscard]] std::optional<ehvota::state::PairedSection> find(const std::string& id) const;
    void clear();

    void writeState(juce::MemoryOutputStream& stream) const;
    [[nodiscard]] bool readState(
        juce::MemoryInputStream& stream,
        std::optional<double> mixDurationSeconds = std::nullopt,
        std::optional<double> referenceDurationSeconds = std::nullopt);

private:
    mutable juce::CriticalSection lock_;
    std::vector<ehvota::state::PairedSection> sections_;
};

} // namespace ehvota::refhub
