#pragma once

#include "ProposalPolicy.h"
#include "shared/communication/SharedLinkProtocol.h"

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace ehvota::safety {

struct ProposalHistoryEntry {
    Proposal proposal;
    ehvota::communication::SharedInstanceId target;
};

class ProposalHistory {
public:
    explicit ProposalHistory(std::size_t maximumEntries = 64) noexcept;

    [[nodiscard]] bool recordApplied(
        const Proposal& proposal,
        ehvota::communication::SharedInstanceId target);

    [[nodiscard]] std::optional<ProposalHistoryEntry> latestApplied() const;
    [[nodiscard]] std::optional<ProposalHistoryEntry> makeApprovedUndoForLatest() const;
    [[nodiscard]] std::vector<ProposalHistoryEntry> snapshot() const { return entries_; }
    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }
    void clear() { entries_.clear(); }

private:
    std::size_t maximumEntries_ { 64 };
    std::vector<ProposalHistoryEntry> entries_;
};

} // namespace ehvota::safety
