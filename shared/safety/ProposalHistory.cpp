#include "ProposalHistory.h"

#include <algorithm>

namespace ehvota::safety {

ProposalHistory::ProposalHistory(const std::size_t maximumEntries) noexcept
    : maximumEntries_(std::max<std::size_t>(1, maximumEntries))
{
}

bool ProposalHistory::recordApplied(
    const Proposal& proposal,
    const ehvota::communication::SharedInstanceId target)
{
    if (proposal.state != ProposalState::applied || !target.valid() || proposal.changes.empty())
        return false;
    if (!entries_.empty() && entries_.back().proposal.id == proposal.id)
        return false;

    if (entries_.size() >= maximumEntries_)
        entries_.erase(entries_.begin());
    entries_.push_back({ proposal, target });
    return true;
}

std::optional<ProposalHistoryEntry> ProposalHistory::latestApplied() const
{
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it)
        if (it->proposal.state == ProposalState::applied)
            return *it;
    return std::nullopt;
}

std::optional<ProposalHistoryEntry> ProposalHistory::makeApprovedUndoForLatest() const
{
    const auto latest = latestApplied();
    if (!latest || !latest->proposal.undoAvailable || latest->proposal.changes.empty())
        return std::nullopt;

    Proposal undo;
    undo.id = "undo:" + latest->proposal.id;
    undo.origin = ProposalOrigin::localRefHub;
    undo.state = ProposalState::approved;
    undo.evidenceId = "undo:" + latest->proposal.evidenceId;
    undo.userExplicitlyApproved = true; // Clicking Undo is the explicit action.
    undo.bridgeFresh = true;
    undo.targetStateFresh = true;
    undo.undoAvailable = true;
    undo.changes.reserve(latest->proposal.changes.size());
    for (const auto& original : latest->proposal.changes) {
        undo.changes.push_back({
            original.targetInstanceId,
            original.targetKind,
            original.parameterId,
            original.proposedValue,
            original.currentValue,
            original.minimumAllowed,
            original.maximumAllowed
        });
    }

    if (!validateProposal(undo).valid || !mayApply(undo))
        return std::nullopt;
    return ProposalHistoryEntry { std::move(undo), latest->target };
}

} // namespace ehvota::safety
