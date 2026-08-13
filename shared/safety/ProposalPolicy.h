#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::safety {

enum class ProposalOrigin {
    localRefHub,
    externalMcp,
    importedPreset
};

enum class ProposalState {
    draft,
    readyForReview,
    approved,
    applied,
    rejected,
    expired,
    invalid
};

enum class TargetKind {
    refEq,
    refComp,
    refColor,
    refTransient,
    refWidth,
    abletonMixer,
    unknown
};

struct ParameterChange {
    std::string targetInstanceId;
    TargetKind targetKind { TargetKind::unknown };
    std::string parameterId;
    double currentValue {};
    double proposedValue {};
    double minimumAllowed {};
    double maximumAllowed {};
};

struct Proposal {
    std::string id;
    ProposalOrigin origin { ProposalOrigin::localRefHub };
    ProposalState state { ProposalState::draft };
    std::string evidenceId;
    std::vector<ParameterChange> changes;
    bool userExplicitlyApproved {};
    bool bridgeFresh {};
    bool targetStateFresh {};
    bool undoAvailable {};
};

struct ValidationResult {
    bool valid {};
    std::vector<std::string> reasons;
};

ValidationResult validateProposal(const Proposal& proposal);
bool mayApply(const Proposal& proposal) noexcept;
std::string_view proposalStateName(ProposalState state) noexcept;

} // namespace ehvota::safety
