#include "JourneyReplay.h"

#include "PreviewScenario.h"

#include <algorithm>
#include <array>
#include <unordered_set>

namespace ehvota::preview {
namespace {

const std::vector<JourneyDefinition> journeys {
    {
        1,
        "add-reference",
        "Add a new reference",
        "reference-ready",
        {
            { "Open Reference Library", "Confirm the local References folder before scanning." },
            { "Scan direct child folders", "Discovered folders stay local and begin in a visible validation state." },
            { "Select Ayo Nights", "Inspect the authoritative full mix and four diagnostic stems." },
            { "Validate files", "Review sample rate, duration, channels, alignment, and role assignments." },
            { "Review separation confidence", "Keep uncertainty and reconstruction caveats visible." },
            { "Resolve uncertain roles", "Never silently guess an ambiguous filename or source role." },
            { "Load primary reference", "Update the comparison strip without starting playback." }
        }
    },
    {
        2,
        "compare-full-mixes",
        "Compare full mixes",
        "compare-chorus",
        {
            { "Choose matched sections", "Select Chorus 2 and the reference Main Chorus." },
            { "Enable loudness match", "Mark the gain as audition-only and excluded from render." },
            { "Switch Mix and Reference", "Keep source identity explicit during normal audition." },
            { "Inspect measurements", "Review tone, dynamics, and width as separate synchronized evidence." },
            { "Focus Low-mid", "Limit the comparison to the clearly labelled selected band." },
            { "Run blind A/B", "Hide identity while preserving a safe exit and trial count." },
            { "Reveal result", "Restore explicit identities and retain the comparison outcome." }
        }
    },
    {
        3,
        "map-stems",
        "Map stems to buses",
        "mapping-disconnected",
        {
            { "Select drums.wav", "Choose the confirmed reference source role." },
            { "Map Drum Bus", "Connect to Drum Bus post-processing and show both identities." },
            { "Map remaining roles", "Connect bass, vocals, and music without silent guesses." },
            { "Build Rhythm composite", "Combine drums and bass with visible member sources." },
            { "Save mapping", "Give the reusable mapping an explicit name." },
            { "Detect disconnect", "Preserve the Music Bus assignment while marking its endpoint unavailable." },
            { "Choose recovery", "Relink or reconnect before remote proposals become available." }
        }
    },
    {
        4,
        "exclude-components",
        "Exclude components",
        "components-no-drums",
        {
            { "Choose Instrumental", "Use the same comparison intent on Mix and Reference." },
            { "Exclude vocals", "Show the paired exclusion rule on both sides." },
            { "Select Drums Only", "State exactly which source is included." },
            { "Select No Drums", "Include bass and music while excluding vocals and drums." },
            { "Verify analysis labels", "Describe included and excluded sources in a complete sentence." },
            { "Review confidence", "Explain degradation caused by missing or leaky diagnostic stems." }
        }
    },
    {
        5,
        "capture-chorus",
        "Capture a chorus",
        "capture-running",
        {
            { "Confirm section pair", "Use live Chorus 2 and reference Main Chorus." },
            { "Arm Capture Analysis", "Do not replace stable history with provisional live data." },
            { "Collect active audio", "Show usable duration, source health, progress, and safe cancellation." },
            { "Complete capture", "Publish stable results only after the evidence target is met." },
            { "Explain confidence", "List duration, stability, source, and reconstruction reasons." },
            { "Save to History", "Store mapping, reference, section pair, timestamp, and measurements." }
        }
    },
    {
        6,
        "review-proposal",
        "Review and apply a proposal",
        "proposal-review",
        {
            { "Select Drum Bus", "Anchor the proposal to a named stable capture." },
            { "Generate bounded proposal", "Limit changes to selected RefEQ and RefComp instances." },
            { "Inspect every change", "Show current and proposed values, limits, confidence, and reasons." },
            { "Review trade-offs", "State the expected benefit and possible loss before preview." },
            { "Preview Current / Proposed", "Use audition-only output matching." },
            { "Run blind comparison", "Keep identity hidden only for the trial." },
            { "Accept or reject", "Require an explicit decision; never apply silently." },
            { "Undo transaction", "Revert both affected instances as one scoped operation." }
        }
    },
    {
        7,
        "reftime-valhalla",
        "RefTime before a third-party reverb",
        "reftime-120",
        {
            { "Read host timing", "Confirm 120 BPM, 4/4, and 48 kHz." },
            { "Select 1/32 dotted", "Use the musical division as the timing source." },
            { "Read exact values", "Show 93.750 ms, 94 ms rounded, and 4,500 samples independently." },
            { "Copy Exact MS", "Confirm exactly what entered the clipboard." },
            { "Choose Delayed Only", "Keep the third-party reverb fully wet with internal pre-delay at zero." },
            { "Change host tempo", "Move the deterministic host value from 120 to 128 BPM." },
            { "Crossfade timing", "Show old and new delay values during a click-free transition." },
            { "Verify latency wording", "Intentional musical delay remains separate from technical latency." }
        }
    },
    {
        8,
        "prepare-stems",
        "Prepare stems offline",
        "stem-prep-queue",
        {
            { "Add Ayo Nights.wav", "Queue the local source without changing the original." },
            { "Preview reference folder", "Show the proposed output before creating it." },
            { "Choose Balanced 4-Stem", "Keep the true model identity available in technical information." },
            { "Choose Automatic device", "Verify model, licence, disk, and output policy while retaining CPU fallback." },
            { "Run local queue", "Advance through Preparing, Separating, Validating, and Finalizing." },
            { "Cancel the second job safely", "Leave no partial output that looks complete." },
            { "Resume another task", "Start or resume a valid queued job without restoring cancelled output." },
            { "Validate completed output", "Check hashes, alignment, duration, format, and reconstruction diagnostics." },
            { "Open completed folder", "Expose the validated local files and separation report." },
            { "Refresh RefHub Library", "Show the new Ready reference without starting playback." }
        }
    }
};

} // namespace

const std::vector<JourneyDefinition>& allJourneys()
{
    return journeys;
}

const JourneyDefinition* findJourneyForScenario(const std::string_view scenarioId)
{
    const auto match = std::find_if(
        journeys.begin(),
        journeys.end(),
        [scenarioId](const JourneyDefinition& journey) {
            return journey.primaryScenarioId == scenarioId;
        });
    return match == journeys.end() ? nullptr : &*match;
}

std::vector<std::string> validateJourneyCatalogue()
{
    std::vector<std::string> errors;
    std::unordered_set<std::string> ids;
    std::unordered_set<std::string> scenarioIds;
    std::array<bool, 8> numbers {};
    constexpr std::array<std::size_t, 8> expectedStepCounts { 7, 7, 7, 6, 6, 8, 8, 10 };

    for (const auto& journey : journeys) {
        if (journey.number < 1 || journey.number > static_cast<int>(numbers.size())) {
            errors.emplace_back("Journey has an invalid number: " + std::to_string(journey.number));
        }
        else if (numbers[static_cast<std::size_t>(journey.number - 1)]) {
            errors.emplace_back("Duplicate journey number: " + std::to_string(journey.number));
        }
        else {
            numbers[static_cast<std::size_t>(journey.number - 1)] = true;
        }

        if (journey.id.empty())
            errors.emplace_back("Journey has an empty id.");
        else if (!ids.insert(journey.id).second)
            errors.emplace_back("Duplicate journey id: " + journey.id);

        if (journey.title.empty())
            errors.emplace_back("Journey " + journey.id + " has an empty title.");
        if (journey.steps.empty())
            errors.emplace_back("Journey " + journey.id + " has no replay steps.");
        else if (journey.number >= 1 && journey.number <= static_cast<int>(expectedStepCounts.size())
            && journey.steps.size()
                != expectedStepCounts[static_cast<std::size_t>(journey.number - 1)]) {
            errors.emplace_back(
                "Journey " + journey.id + " has "
                + std::to_string(journey.steps.size()) + " steps; expected "
                + std::to_string(
                    expectedStepCounts[static_cast<std::size_t>(journey.number - 1)])
                + ".");
        }

        if (findScenario(journey.primaryScenarioId) == nullptr) {
            errors.emplace_back(
                "Journey " + journey.id + " references a missing scenario: "
                + journey.primaryScenarioId);
        }
        else if (!scenarioIds.insert(journey.primaryScenarioId).second) {
            errors.emplace_back(
                "Multiple journeys use primary scenario: " + journey.primaryScenarioId);
        }

        for (std::size_t index = 0; index < journey.steps.size(); ++index) {
            if (journey.steps[index].title.empty() || journey.steps[index].instruction.empty()) {
                errors.emplace_back(
                    "Journey " + journey.id + " has incomplete step "
                    + std::to_string(index + 1));
            }
        }
    }

    for (std::size_t index = 0; index < numbers.size(); ++index) {
        if (!numbers[index])
            errors.emplace_back("Missing journey number: " + std::to_string(index + 1));
    }

    return errors;
}

bool JourneyReplay::selectForScenario(const std::string_view scenarioId)
{
    journey_ = findJourneyForScenario(scenarioId);
    stepIndex_ = 0;
    return journey_ != nullptr;
}

bool JourneyReplay::setStepNumber(const std::size_t oneBasedStep)
{
    if (journey_ == nullptr || oneBasedStep == 0 || oneBasedStep > journey_->steps.size())
        return false;

    stepIndex_ = oneBasedStep - 1;
    return true;
}

bool JourneyReplay::next()
{
    if (!canGoNext())
        return false;

    ++stepIndex_;
    return true;
}

bool JourneyReplay::previous()
{
    if (!canGoPrevious())
        return false;

    --stepIndex_;
    return true;
}

const JourneyDefinition* JourneyReplay::journey() const noexcept
{
    return journey_;
}

const JourneyStep* JourneyReplay::currentStep() const noexcept
{
    if (journey_ == nullptr || stepIndex_ >= journey_->steps.size())
        return nullptr;
    return &journey_->steps[stepIndex_];
}

std::size_t JourneyReplay::currentStepNumber() const noexcept
{
    return journey_ != nullptr ? stepIndex_ + 1 : 0;
}

std::size_t JourneyReplay::totalSteps() const noexcept
{
    return journey_ != nullptr ? journey_->steps.size() : 0;
}

bool JourneyReplay::canGoNext() const noexcept
{
    return journey_ != nullptr && stepIndex_ + 1 < journey_->steps.size();
}

bool JourneyReplay::canGoPrevious() const noexcept
{
    return journey_ != nullptr && stepIndex_ > 0;
}

double JourneyReplay::progress() const noexcept
{
    return journey_ != nullptr && !journey_->steps.empty()
        ? static_cast<double>(stepIndex_ + 1) / static_cast<double>(journey_->steps.size())
        : 0.0;
}

} // namespace ehvota::preview
