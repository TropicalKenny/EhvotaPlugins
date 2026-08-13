#pragma once

#include "shared/communication/SharedLinkProtocol.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <cmath>
#include <cstddef>

namespace ehvota::plugins {

struct SharedParameterBinding { ehvota::communication::SharedParameterKey key; const char* parameterId; };

enum class SharedProposalApplyResult : std::uint32_t { applied = 0, invalidProposal = 1, unresolvedParameter = 2, staleTarget = 3 };

template <std::size_t BindingCount>
SharedProposalApplyResult applySharedProposal(
    juce::AudioProcessorValueTreeState& state,
    const ehvota::communication::SharedPluginKind kind,
    const ehvota::communication::SharedProposalMailbox& proposal,
    const std::array<SharedParameterBinding, BindingCount>& bindings)
{
    using namespace ehvota::communication;
    if (!validateSharedProposal(kind, proposal)) return SharedProposalApplyResult::invalidProposal;

    std::array<juce::RangedAudioParameter*, sharedLinkMaxOperations> parameters {};
    std::array<float, sharedLinkMaxOperations> normalised {};
    for (std::uint32_t i = 0; i < proposal.operationCount; ++i) {
        const auto& op = proposal.operations[i];
        const SharedParameterBinding* binding = nullptr;
        for (const auto& candidate : bindings) if (candidate.key == op.key) { binding = &candidate; break; }
        if (binding == nullptr) return SharedProposalApplyResult::unresolvedParameter;
        auto* parameter = state.getParameter(binding->parameterId);
        auto* raw = state.getRawParameterValue(binding->parameterId);
        if (parameter == nullptr || raw == nullptr) return SharedProposalApplyResult::unresolvedParameter;
        const auto actual = raw->load(std::memory_order_relaxed);
        if (std::abs(actual - op.expectedCurrentValue) > op.expectedTolerance) return SharedProposalApplyResult::staleTarget;
        parameters[i] = parameter;
        normalised[i] = parameter->convertTo0to1(op.value);
    }

    for (std::uint32_t i = 0; i < proposal.operationCount; ++i) {
        parameters[i]->beginChangeGesture();
        parameters[i]->setValueNotifyingHost(normalised[i]);
        parameters[i]->endChangeGesture();
    }
    return SharedProposalApplyResult::applied;
}

} // namespace ehvota::plugins
