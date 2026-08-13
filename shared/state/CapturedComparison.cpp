#include "CapturedComparison.h"

namespace ehvota::state {

bool CapturedComparison::hasStablePair() const noexcept
{
    if (!mix || !reference)
        return false;
    if (mix->sectionId != reference->sectionId)
        return false;
    return mix->evidence == CaptureEvidence::capturedStable
        && reference->evidence == CaptureEvidence::capturedStable
        && mix->loudness.valid
        && reference->loudness.valid;
}

ehvota::analysis::LevelMatchDecision CapturedComparison::levelMatch() const noexcept
{
    if (!hasStablePair())
        return {};
    return ehvota::analysis::calculateReferenceMonitorTrim(mix->loudness, reference->loudness);
}

} // namespace ehvota::state
