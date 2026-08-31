#include "ReferenceProposalGenerator.h"

#include "LinkProposalAdapter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace ehvota::safety {
namespace {

constexpr std::array<ehvota::communication::SharedParameterKey, 6> gainKeys {
    ehvota::communication::SharedParameterKey::eqBand1Gain,
    ehvota::communication::SharedParameterKey::eqBand2Gain,
    ehvota::communication::SharedParameterKey::eqBand3Gain,
    ehvota::communication::SharedParameterKey::eqBand4Gain,
    ehvota::communication::SharedParameterKey::eqBand5Gain,
    ehvota::communication::SharedParameterKey::eqBand6Gain
};

constexpr std::array<const char*, 6> gainParameterIds {
    "b1Gain", "b2Gain", "b3Gain", "b4Gain", "b5Gain", "b6Gain"
};

bool validSettings(const ReferenceProposalSettings& settings) noexcept
{
    return std::isfinite(settings.minimumBandDifferenceDb)
        && std::isfinite(settings.correctionFraction)
        && std::isfinite(settings.maximumEqStepDb)
        && settings.minimumBandDifferenceDb > 0.0
        && settings.correctionFraction > 0.0
        && settings.correctionFraction <= 1.0
        && settings.maximumEqStepDb > 0.0
        && settings.maximumEqStepDb <= 6.0
        && settings.maximumTargetAgeMs > 0;
}

} // namespace

std::optional<Proposal> generateRefEqProposal(
    const ehvota::analysis::SectionComparison& comparison,
    const ehvota::communication::SharedInstanceId target,
    const ehvota::communication::SharedParameterSnapshot& targetParameters,
    const std::uint64_t targetHeartbeatAgeMs,
    std::string evidenceId,
    const ReferenceProposalSettings& settings)
{
    using namespace ehvota::communication;

    if (!comparison.valid || !target.valid() || evidenceId.empty() || !validSettings(settings))
        return std::nullopt;
    if (targetHeartbeatAgeMs > settings.maximumTargetAgeMs || targetParameters.revision == 0)
        return std::nullopt;
    if (!validateSharedParameterSnapshot(SharedPluginKind::refEQ, targetParameters))
        return std::nullopt;

    std::size_t strongest = comparison.bands.size();
    double strongestMagnitude = settings.minimumBandDifferenceDb;
    for (std::size_t i = 0; i < comparison.bands.size(); ++i) {
        const auto& band = comparison.bands[i];
        if (!std::isfinite(band.differenceDb)
            || !std::isfinite(band.mixPowerDb)
            || !std::isfinite(band.referencePowerDb))
            continue;
        // Avoid suggesting EQ from effectively silent/undefined spectral regions.
        if (band.mixPowerDb <= -120.0 || band.referencePowerDb <= -120.0)
            continue;
        const auto magnitude = std::abs(band.differenceDb);
        if (magnitude >= strongestMagnitude) {
            strongestMagnitude = magnitude;
            strongest = i;
        }
    }
    if (strongest >= comparison.bands.size())
        return std::nullopt;

    const auto current = sharedParameterValue(targetParameters, gainKeys[strongest]);
    if (!current)
        return std::nullopt;

    const auto rawCorrection = -comparison.bands[strongest].differenceDb * settings.correctionFraction;
    const auto boundedCorrection = std::clamp(rawCorrection, -settings.maximumEqStepDb, settings.maximumEqStepDb);
    const auto proposed = std::clamp(static_cast<double>(*current) + boundedCorrection, -18.0, 18.0);
    if (std::abs(proposed - static_cast<double>(*current)) < 0.1)
        return std::nullopt;

    Proposal proposal;
    proposal.id = "refeq:" + evidenceId + ":" + std::to_string(target.slot)
        + ":" + std::to_string(target.generation) + ":" + std::to_string(strongest);
    proposal.origin = ProposalOrigin::localRefHub;
    proposal.state = ProposalState::readyForReview;
    proposal.evidenceId = std::move(evidenceId);
    proposal.userExplicitlyApproved = false;
    proposal.bridgeFresh = true;
    proposal.targetStateFresh = true;
    // The exact current value is captured here, so an inverse Ehvota transaction can be formed.
    // Actual application still requires explicit approval and the target-side stale-value check.
    proposal.undoAvailable = true;
    proposal.changes.push_back({
        sharedInstanceTargetId(target),
        TargetKind::refEq,
        gainParameterIds[strongest],
        static_cast<double>(*current),
        proposed,
        -18.0,
        18.0
    });

    const auto validation = validateProposal(proposal);
    if (!validation.valid || mayApply(proposal))
        return std::nullopt;
    return proposal;
}

} // namespace ehvota::safety
