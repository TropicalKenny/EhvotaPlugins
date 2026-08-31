#include "shared/safety/ReferenceProposalGenerator.h"

#include <cassert>
#include <cmath>

namespace {

ehvota::analysis::SectionComparison baseComparison()
{
    ehvota::analysis::SectionComparison comparison;
    comparison.valid = true;
    const auto& definitions = ehvota::analysis::defaultComparisonBands();
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        comparison.bands[i].band = definitions[i];
        comparison.bands[i].mixPowerDb = -24.0;
        comparison.bands[i].referencePowerDb = -24.0;
        comparison.bands[i].differenceDb = 0.0;
    }
    return comparison;
}

ehvota::communication::SharedParameterSnapshot eqSnapshot()
{
    using namespace ehvota::communication;
    SharedParameterSnapshot snapshot;
    snapshot.revision = 4;
    const SharedParameterKey keys[] {
        SharedParameterKey::eqBand1Gain, SharedParameterKey::eqBand2Gain,
        SharedParameterKey::eqBand3Gain, SharedParameterKey::eqBand4Gain,
        SharedParameterKey::eqBand5Gain, SharedParameterKey::eqBand6Gain
    };
    for (const auto key : keys)
        snapshot.values[snapshot.count++] = { key, 0.0f };
    return snapshot;
}
}

int main()
{
    using namespace ehvota;
    const communication::SharedInstanceId target { 2, 9 };

    auto comparison = baseComparison();
    auto parameters = eqSnapshot();
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 20, "chorus-1"));

    comparison.bands[1].mixPowerDb = -21.0;
    comparison.bands[1].referencePowerDb = -24.0;
    comparison.bands[1].differenceDb = 3.0;
    parameters.values[1].value = 1.0f;
    const auto lowMid = safety::generateRefEqProposal(comparison, target, parameters, 20, "chorus-1");
    assert(lowMid.has_value());
    assert(lowMid->state == safety::ProposalState::readyForReview);
    assert(!lowMid->userExplicitlyApproved);
    assert(!safety::mayApply(*lowMid));
    assert(lowMid->changes.size() == 1);
    assert(lowMid->changes[0].parameterId == "b2Gain");
    assert(lowMid->changes[0].targetInstanceId == "2:9");
    assert(std::abs(lowMid->changes[0].currentValue - 1.0) < 1.0e-9);
    assert(std::abs(lowMid->changes[0].proposedValue + 0.5) < 1.0e-9);

    comparison = baseComparison();
    parameters = eqSnapshot();
    comparison.bands[4].mixPowerDb = -30.0;
    comparison.bands[4].referencePowerDb = -24.0;
    comparison.bands[4].differenceDb = -6.0;
    parameters.values[4].value = 17.5f;
    const auto bounded = safety::generateRefEqProposal(comparison, target, parameters, 20, "verse-2");
    assert(bounded.has_value());
    assert(bounded->changes[0].parameterId == "b5Gain");
    assert(std::abs(bounded->changes[0].proposedValue - 18.0) < 1.0e-9);

    comparison = baseComparison();
    comparison.bands[2].mixPowerDb = -23.2;
    comparison.bands[2].referencePowerDb = -24.0;
    comparison.bands[2].differenceDb = 0.8;
    parameters = eqSnapshot();
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 20, "verse-small"));

    comparison = baseComparison();
    comparison.bands[3].mixPowerDb = -20.0;
    comparison.bands[3].referencePowerDb = -24.0;
    comparison.bands[3].differenceDb = 4.0;
    parameters = eqSnapshot();
    parameters.count = 3; // selected b4 gain is no longer present.
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 20, "missing-target-state"));

    parameters = eqSnapshot();
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 6000, "stale-target"));
    parameters.revision = 0;
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 20, "unpublished-target"));

    comparison.valid = false;
    parameters = eqSnapshot();
    assert(!safety::generateRefEqProposal(comparison, target, parameters, 20, "invalid-analysis"));
    return 0;
}
