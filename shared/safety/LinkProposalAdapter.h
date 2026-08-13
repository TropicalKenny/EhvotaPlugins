#pragma once

#include "ProposalPolicy.h"
#include "shared/communication/SharedLinkProtocol.h"

#include <optional>
#include <string>

namespace ehvota::safety {

[[nodiscard]] std::string sharedInstanceTargetId(ehvota::communication::SharedInstanceId target);
[[nodiscard]] std::optional<ehvota::communication::SharedProposalMailbox> encodeApprovedSharedProposal(
    const Proposal& proposal,
    ehvota::communication::SharedInstanceId target,
    ehvota::communication::SharedPluginKind pluginKind);

} // namespace ehvota::safety
