#include "ProposalPolicy.h"

namespace ehvota::safety {

ValidationResult validateProposal(const Proposal& proposal)
{
    ValidationResult result { true, {} };

    if (proposal.id.empty()) {
        result.valid = false;
        result.reasons.emplace_back("Proposal id is missing.");
    }
    if (proposal.evidenceId.empty()) {
        result.valid = false;
        result.reasons.emplace_back("Proposal requires captured evidence.");
    }
    if (proposal.changes.empty()) {
        result.valid = false;
        result.reasons.emplace_back("Proposal contains no changes.");
    }
    if (!proposal.targetStateFresh) {
        result.valid = false;
        result.reasons.emplace_back("Target state is stale.");
    }
    if (!proposal.undoAvailable) {
        result.valid = false;
        result.reasons.emplace_back("Undo is unavailable.");
    }

    for (const auto& change : proposal.changes) {
        if (change.targetInstanceId.empty() || change.parameterId.empty()) {
            result.valid = false;
            result.reasons.emplace_back("A change has an unresolved target.");
        }
        if (change.minimumAllowed > change.maximumAllowed
            || change.proposedValue < change.minimumAllowed
            || change.proposedValue > change.maximumAllowed) {
            result.valid = false;
            result.reasons.emplace_back("A proposed value is outside its safe bounds.");
        }
    }

    return result;
}

bool mayApply(const Proposal& proposal) noexcept
{
    if (proposal.state != ProposalState::approved)
        return false;
    if (!proposal.userExplicitlyApproved || !proposal.targetStateFresh || !proposal.undoAvailable)
        return false;
    if (proposal.changes.empty() || proposal.evidenceId.empty())
        return false;

    for (const auto& change : proposal.changes) {
        if (change.targetInstanceId.empty() || change.parameterId.empty())
            return false;
        if (change.minimumAllowed > change.maximumAllowed)
            return false;
        if (change.proposedValue < change.minimumAllowed || change.proposedValue > change.maximumAllowed)
            return false;
    }

    return true;
}

std::string_view proposalStateName(const ProposalState state) noexcept
{
    switch (state) {
    case ProposalState::draft: return "Draft";
    case ProposalState::readyForReview: return "Ready for review";
    case ProposalState::approved: return "Approved";
    case ProposalState::applied: return "Applied";
    case ProposalState::rejected: return "Rejected";
    case ProposalState::expired: return "Expired";
    case ProposalState::invalid: return "Invalid";
    }
    return "Unknown";
}

} // namespace ehvota::safety
