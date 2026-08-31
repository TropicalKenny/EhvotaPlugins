#pragma once

#include "ProposalPolicy.h"
#include "shared/analysis/ComparisonAnalysis.h"
#include "shared/communication/SharedLinkProtocol.h"

#include <cstdint>
#include <optional>
#include <string>

namespace ehvota::safety {

struct ReferenceProposalSettings {
    double minimumBandDifferenceDb { 1.0 };
    double correctionFraction { 0.5 };
    double maximumEqStepDb { 2.5 };
    std::uint64_t maximumTargetAgeMs { 5000 };
};

[[nodiscard]] std::optional<Proposal> generateRefEqProposal(
    const ehvota::analysis::SectionComparison& comparison,
    ehvota::communication::SharedInstanceId target,
    const ehvota::communication::SharedParameterSnapshot& targetParameters,
    std::uint64_t targetHeartbeatAgeMs,
    std::string evidenceId,
    const ReferenceProposalSettings& settings = {});

} // namespace ehvota::safety
