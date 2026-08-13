#include "LinkProposalAdapter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace ehvota::safety {
namespace {
using namespace ehvota::communication;

TargetKind expectedTargetKind(const SharedPluginKind kind) noexcept
{
    switch (kind) {
    case SharedPluginKind::refEQ: return TargetKind::refEq;
    case SharedPluginKind::refComp: return TargetKind::refComp;
    case SharedPluginKind::refColor: return TargetKind::refColor;
    case SharedPluginKind::refTransient: return TargetKind::refTransient;
    default: return TargetKind::unknown;
    }
}

std::optional<SharedParameterKey> keyFor(const SharedPluginKind kind, const std::string_view id) noexcept
{
    if (kind == SharedPluginKind::refEQ) {
        if (id == "hpEnabled") return SharedParameterKey::eqHighPassEnabled; if (id == "hpFreq") return SharedParameterKey::eqHighPassFrequency;
        if (id == "lpEnabled") return SharedParameterKey::eqLowPassEnabled; if (id == "lpFreq") return SharedParameterKey::eqLowPassFrequency;
        if (id == "b1Freq") return SharedParameterKey::eqBand1Frequency; if (id == "b1Gain") return SharedParameterKey::eqBand1Gain; if (id == "b1Q") return SharedParameterKey::eqBand1Q;
        if (id == "b2Freq") return SharedParameterKey::eqBand2Frequency; if (id == "b2Gain") return SharedParameterKey::eqBand2Gain; if (id == "b2Q") return SharedParameterKey::eqBand2Q;
        if (id == "b3Freq") return SharedParameterKey::eqBand3Frequency; if (id == "b3Gain") return SharedParameterKey::eqBand3Gain; if (id == "b3Q") return SharedParameterKey::eqBand3Q;
        if (id == "b4Freq") return SharedParameterKey::eqBand4Frequency; if (id == "b4Gain") return SharedParameterKey::eqBand4Gain; if (id == "b4Q") return SharedParameterKey::eqBand4Q;
        if (id == "b5Freq") return SharedParameterKey::eqBand5Frequency; if (id == "b5Gain") return SharedParameterKey::eqBand5Gain; if (id == "b5Q") return SharedParameterKey::eqBand5Q;
        if (id == "b6Freq") return SharedParameterKey::eqBand6Frequency; if (id == "b6Gain") return SharedParameterKey::eqBand6Gain; if (id == "b6Q") return SharedParameterKey::eqBand6Q;
        if (id == "output") return SharedParameterKey::eqOutput;
    }
    if (kind == SharedPluginKind::refComp) {
        if (id == "mode") return SharedParameterKey::compMode; if (id == "threshold") return SharedParameterKey::compThreshold; if (id == "ratio") return SharedParameterKey::compRatio;
        if (id == "attack") return SharedParameterKey::compAttack; if (id == "attackSync") return SharedParameterKey::compAttackSync; if (id == "attackNote") return SharedParameterKey::compAttackNote;
        if (id == "attackModifier") return SharedParameterKey::compAttackModifier; if (id == "release") return SharedParameterKey::compRelease; if (id == "releaseSync") return SharedParameterKey::compReleaseSync;
        if (id == "releaseNote") return SharedParameterKey::compReleaseNote; if (id == "releaseModifier") return SharedParameterKey::compReleaseModifier; if (id == "knee") return SharedParameterKey::compKnee;
        if (id == "makeup") return SharedParameterKey::compMakeup; if (id == "mix") return SharedParameterKey::compMix;
    }
    if (kind == SharedPluginKind::refColor) {
        if (id == "mode") return SharedParameterKey::colorMode; if (id == "amount") return SharedParameterKey::colorAmount; if (id == "drive") return SharedParameterKey::colorDrive;
        if (id == "asymmetry") return SharedParameterKey::colorAsymmetry; if (id == "output") return SharedParameterKey::colorOutput;
    }
    if (kind == SharedPluginKind::refTransient) {
        if (id == "attack") return SharedParameterKey::transientAttack; if (id == "sustain") return SharedParameterKey::transientSustain;
        if (id == "sensitivity") return SharedParameterKey::transientSensitivity; if (id == "output") return SharedParameterKey::transientOutput;
    }
    return std::nullopt;
}

std::uint64_t stableProposalId(const std::string_view text) noexcept
{
    std::uint64_t hash = 1469598103934665603ull;
    for (const unsigned char ch : text) { hash ^= ch; hash *= 1099511628211ull; }
    return hash == 0 ? 1 : hash;
}
} // namespace

std::string sharedInstanceTargetId(const ehvota::communication::SharedInstanceId target)
{
    return std::to_string(target.slot) + ":" + std::to_string(target.generation);
}

std::optional<ehvota::communication::SharedProposalMailbox> encodeApprovedSharedProposal(
    const Proposal& proposal,
    const ehvota::communication::SharedInstanceId target,
    const ehvota::communication::SharedPluginKind pluginKind)
{
    using namespace ehvota::communication;
    if (!target.valid() || !mayApply(proposal) || proposal.changes.size() > sharedLinkMaxOperations) return std::nullopt;
    const auto expectedKind = expectedTargetKind(pluginKind);
    if (expectedKind == TargetKind::unknown) return std::nullopt;
    const auto targetId = sharedInstanceTargetId(target);

    SharedProposalMailbox mailbox;
    mailbox.proposalId = stableProposalId(proposal.id);
    mailbox.targetGeneration = target.generation;
    mailbox.operationCount = static_cast<std::uint32_t>(proposal.changes.size());

    for (std::size_t i = 0; i < proposal.changes.size(); ++i) {
        const auto& change = proposal.changes[i];
        if (change.targetKind != expectedKind || change.targetInstanceId != targetId) return std::nullopt;
        const auto key = keyFor(pluginKind, change.parameterId);
        if (!key) return std::nullopt;
        const auto bounds = sharedParameterBounds(pluginKind, *key);
        if (!bounds) return std::nullopt;
        const auto span = bounds->maximum - bounds->minimum;
        const auto tolerance = bounds->integerOnly ? 0.001f : std::max(0.0001f, span * 0.00001f);
        SharedParameterOperation op { *key, static_cast<float>(change.currentValue), tolerance, static_cast<float>(change.proposedValue) };
        if (!validateSharedOperation(pluginKind, op)) return std::nullopt;
        mailbox.operations[i] = op;
    }
    if (!validateSharedProposal(pluginKind, mailbox)) return std::nullopt;
    return mailbox;
}

} // namespace ehvota::safety
