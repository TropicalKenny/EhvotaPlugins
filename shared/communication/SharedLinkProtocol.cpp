#include "SharedLinkProtocol.h"

#include <cmath>

namespace ehvota::communication {
namespace {
std::optional<ParameterBounds> eqBounds(const SharedParameterKey key) noexcept
{
    switch (key) {
    case SharedParameterKey::eqHighPassEnabled: case SharedParameterKey::eqLowPassEnabled: return ParameterBounds{0,1,true};
    case SharedParameterKey::eqHighPassFrequency: return ParameterBounds{20,800,false};
    case SharedParameterKey::eqLowPassFrequency: return ParameterBounds{2000,22000,false};
    case SharedParameterKey::eqBand1Frequency: case SharedParameterKey::eqBand2Frequency: case SharedParameterKey::eqBand3Frequency:
    case SharedParameterKey::eqBand4Frequency: case SharedParameterKey::eqBand5Frequency: case SharedParameterKey::eqBand6Frequency: return ParameterBounds{20,20000,false};
    case SharedParameterKey::eqBand1Gain: case SharedParameterKey::eqBand2Gain: case SharedParameterKey::eqBand3Gain:
    case SharedParameterKey::eqBand4Gain: case SharedParameterKey::eqBand5Gain: case SharedParameterKey::eqBand6Gain:
    case SharedParameterKey::eqOutput: return ParameterBounds{-18,18,false};
    case SharedParameterKey::eqBand1Q: case SharedParameterKey::eqBand2Q: case SharedParameterKey::eqBand3Q:
    case SharedParameterKey::eqBand4Q: case SharedParameterKey::eqBand5Q: case SharedParameterKey::eqBand6Q: return ParameterBounds{0.15f,18,false};
    default: return std::nullopt;
    }
}
std::optional<ParameterBounds> compBounds(const SharedParameterKey key) noexcept
{
    switch (key) {
    case SharedParameterKey::compMode: return ParameterBounds{0,2,true};
    case SharedParameterKey::compThreshold: return ParameterBounds{-60,0,false};
    case SharedParameterKey::compRatio: return ParameterBounds{1,20,false};
    case SharedParameterKey::compAttack: return ParameterBounds{0.05f,200,false};
    case SharedParameterKey::compAttackSync: case SharedParameterKey::compReleaseSync: return ParameterBounds{0,1,true};
    case SharedParameterKey::compAttackNote: case SharedParameterKey::compReleaseNote: return ParameterBounds{0,4,true};
    case SharedParameterKey::compAttackModifier: case SharedParameterKey::compReleaseModifier: return ParameterBounds{0,2,true};
    case SharedParameterKey::compRelease: return ParameterBounds{5,2000,false};
    case SharedParameterKey::compKnee: return ParameterBounds{0,24,false};
    case SharedParameterKey::compMakeup: return ParameterBounds{-12,18,false};
    case SharedParameterKey::compMix: return ParameterBounds{0,100,false};
    default: return std::nullopt;
    }
}
std::optional<ParameterBounds> colorBounds(const SharedParameterKey key) noexcept
{
    switch (key) {
    case SharedParameterKey::colorMode: return ParameterBounds{0,2,true};
    case SharedParameterKey::colorAmount: return ParameterBounds{0,100,false};
    case SharedParameterKey::colorDrive: return ParameterBounds{0,18,false};
    case SharedParameterKey::colorAsymmetry: return ParameterBounds{-1,1,false};
    case SharedParameterKey::colorOutput: return ParameterBounds{-12,12,false};
    default: return std::nullopt;
    }
}
std::optional<ParameterBounds> transientBounds(const SharedParameterKey key) noexcept
{
    switch (key) {
    case SharedParameterKey::transientAttack: case SharedParameterKey::transientSustain: return ParameterBounds{-100,100,false};
    case SharedParameterKey::transientSensitivity: return ParameterBounds{0,100,false};
    case SharedParameterKey::transientOutput: return ParameterBounds{-12,12,false};
    default: return std::nullopt;
    }
}
} // namespace

std::optional<ParameterBounds> sharedParameterBounds(const SharedPluginKind kind, const SharedParameterKey key) noexcept
{
    switch (kind) {
    case SharedPluginKind::refEQ: return eqBounds(key);
    case SharedPluginKind::refComp: return compBounds(key);
    case SharedPluginKind::refColor: return colorBounds(key);
    case SharedPluginKind::refTransient: return transientBounds(key);
    default: return std::nullopt;
    }
}

bool sharedParameterAllowed(const SharedPluginKind kind, const SharedParameterKey key) noexcept { return sharedParameterBounds(kind,key).has_value(); }

bool validateSharedOperation(const SharedPluginKind kind, const SharedParameterOperation& op) noexcept
{
    const auto bounds = sharedParameterBounds(kind, op.key);
    if (!bounds || !std::isfinite(op.value) || !std::isfinite(op.expectedCurrentValue) || !std::isfinite(op.expectedTolerance)) return false;
    if (op.value < bounds->minimum || op.value > bounds->maximum) return false;
    if (op.expectedCurrentValue < bounds->minimum || op.expectedCurrentValue > bounds->maximum) return false;
    if (op.expectedTolerance < 0.0f || op.expectedTolerance > (bounds->maximum - bounds->minimum)) return false;
    if (bounds->integerOnly) {
        if (std::abs(op.value - std::round(op.value)) > 1.0e-5f) return false;
        if (std::abs(op.expectedCurrentValue - std::round(op.expectedCurrentValue)) > 1.0e-5f) return false;
    }
    return true;
}

bool validateSharedProposal(const SharedPluginKind kind, const SharedProposalMailbox& proposal) noexcept
{
    if (proposal.proposalId == 0 || proposal.targetGeneration == 0 || proposal.operationCount == 0 || proposal.operationCount > sharedLinkMaxOperations) return false;
    for (std::uint32_t i = 0; i < proposal.operationCount; ++i) if (!validateSharedOperation(kind, proposal.operations[i])) return false;
    return true;
}

std::string_view sharedPluginKindName(const SharedPluginKind kind) noexcept
{
    switch (kind) {
    case SharedPluginKind::refHub: return "RefHub"; case SharedPluginKind::refTap: return "RefTap";
    case SharedPluginKind::refEQ: return "RefEQ"; case SharedPluginKind::refComp: return "RefComp";
    case SharedPluginKind::refTime: return "RefTime"; case SharedPluginKind::refColor: return "RefColor";
    case SharedPluginKind::refTransient: return "RefTransient"; case SharedPluginKind::unknown: break;
    }
    return "Unknown";
}
} // namespace ehvota::communication
