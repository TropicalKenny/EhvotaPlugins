#include "PreviewScenario.h"

#include <algorithm>
#include <array>
#include <unordered_set>

namespace ehvota::preview {
namespace {

using enum ProductArea;
using enum Tone;

const std::vector<Scenario> scenarios {
    {
        "reference-ready",
        "Journey 1 · Add a new reference",
        "Ayo Nights is ready to compare",
        "The full mix is authoritative. Four diagnostic stems passed alignment checks; separation uncertainty remains visible.",
        refHub,
        "Library",
        "Ayo Nights",
        "No section selected",
        { { "READY", positive }, { "4 STEMS", reference }, { "CONFIDENCE MEDIUM", lowConfidence } },
        {
            { "Reference files", "5 of 5 expected files found",
              { { "Full mix", "Ready", "48 kHz · 03:31 · Stereo", positive },
                { "Stem alignment", "Passed", "Equal start and duration", positive },
                { "Role mapping", "Confirmed", "No silent guesses", reference } } },
            { "Separation report", "Diagnostic stems are usable with caveats",
              { { "Reconstruction", "Medium", "Kick energy shared with bass", lowConfidence },
                { "Vocal ambience", "Detected", "Present in music stem", caution } } }
        },
        "Load reference",
        { "Primary source: full_mix.wav", "Hashes recorded", "Load does not start playback", "Action: Load as primary reference" }
    },
    {
        "compare-chorus",
        "Journey 2 · Compare full mixes",
        "Chorus tone, dynamics, and width",
        "Loudness-matched audition is enabled. Captured metrics remain separate from the live monitor.",
        refHub,
        "Compare",
        "Ayo Nights",
        "Chorus 2 ↔ Main Chorus",
        { { "CAPTURED", positive }, { "MATCH −1.8 dB", reference }, { "AUDITION ONLY", caution } },
        {
            { "Tone", "Your mix is denser in the low-mid range",
              { { "Low-mid", "+1.7 dB", "200–500 Hz · captured average", liveMix },
                { "High", "−0.9 dB", "6–20 kHz · within target range", reference } } },
            { "Dynamics", "Reference has more short-term movement",
              { { "Integrated", "−10.8 LUFS", "Matched for audition", neutral },
                { "Crest factor", "8.6 dB", "Reference 9.8 dB", caution } } },
            { "Space", "Width is similar; low sides need attention",
              { { "Side / Mid", "−9.2 dB", "Reference −8.8 dB", positive },
                { "Low Side", "Elevated", "Below 110 Hz", caution } } }
        },
        "Band focus · Low-mid",
        { "Solid line: Mix", "Dashed line: Reference", "Active audio: 18.4 s", "Blind comparison available" }
    },
    {
        "mapping-disconnected",
        "Journey 3 · Map stems to buses",
        "One saved mapping is disconnected",
        "Reference roles remain mapped while the missing live endpoint is clearly unavailable.",
        refHub,
        "Mapping",
        "Ayo Nights",
        "Full arrangement",
        { { "3 / 4 CONNECTED", caution }, { "RHYTHM COMPOSITE", proposed } },
        {
            { "Reference sources", "Four confirmed diagnostic roles",
              { { "Drums", "Drum Bus · post", "Connected", positive },
                { "Bass", "Bass Bus · post", "Connected", positive },
                { "Vocals", "Vocal Bus · post", "Connected", positive },
                { "Music", "Music Bus · post", "Disconnected; mapping preserved", error } } },
            { "Composite", "Rhythm = Drums + Bass",
              { { "Members", "2", "Drums, Bass", proposed },
                { "Status", "Ready", "Both endpoints connected", positive } } }
        },
        "Music Bus · post",
        { "Last update: 14.2 s ago", "Endpoint unavailable", "Proposals disabled", "Action: relink or reconnect" }
    },
    {
        "components-no-drums",
        "Journey 4 · Exclude components",
        "Instrumental comparison · No drums",
        "Analysis includes bass and music on both sides. Vocals and drums are excluded explicitly.",
        refHub,
        "Components",
        "Ayo Nights",
        "Chorus 2 ↔ Main Chorus",
        { { "NO DRUMS", proposed }, { "VOCALS EXCLUDED", neutral }, { "CONFIDENCE MEDIUM", lowConfidence } },
        {
            { "Included", "Same rule applied to Mix and Reference",
              { { "Bass", "Included", "Connected and active", positive },
                { "Music", "Included", "Reference ambience leakage noted", caution } } },
            { "Excluded", "Not part of this measurement",
              { { "Drums", "Excluded", "Both sides", neutral },
                { "Vocals", "Excluded", "Both sides", neutral } } }
        },
        "Why confidence is Medium",
        { "Music stem contains vocal ambience", "Usable active duration: 16.8 s", "Advice remains available", "Automatic apply unavailable" }
    },
    {
        "capture-running",
        "Journey 5 · Capture a chorus",
        "Capturing Chorus 2",
        "Capture Analysis is collecting stable section evidence. Live feedback continues but does not replace the capture.",
        refHub,
        "Compare",
        "Ayo Nights",
        "Chorus 2 ↔ Main Chorus",
        { { "CAPTURING 62%", proposed }, { "12.4 s USABLE", liveMix }, { "CANCEL SAFE", neutral } },
        {
            { "Progress", "Waiting for the loop to complete",
              { { "Captured", "19.1 s", "Target 30.8 s", liveMix },
                { "Active audio", "12.4 s", "Silence excluded", positive },
                { "Sources", "5 / 5", "All timestamps current", positive } } },
            { "Stability", "Provisional until capture completes",
              { { "Tone variance", "Falling", "Measurement converging", positive },
                { "Dynamics", "Pending", "Needs one more loop", caution } } }
        },
        "Capture controls",
        { "Cancel keeps prior capture", "No recommendation generated yet", "Transport discontinuity protection active" }
    },
    {
        "proposal-review",
        "Journey 6 · Review and apply a proposal",
        "Drum Bus · bounded tone and dynamics proposal",
        "Every affected parameter is visible. Preview and loudness match are audition-only; acceptance creates one undo transaction.",
        refHub,
        "Proposals",
        "Ayo Nights",
        "Chorus 2 ↔ Main Chorus",
        { { "REVIEW REQUIRED", proposed }, { "CONFIDENCE HIGH", positive }, { "2 INSTANCES", caution } },
        {
            { "RefEQ · Drum Bus", "Reduce low-mid buildup within bus safety limits",
              { { "Band 3 gain", "−1.8 dB", "Current 0.0 dB · limit ±3.0 dB", proposed },
                { "Band 3 Q", "0.82", "Current 0.70", proposed } } },
            { "RefComp · Drum Bus", "Recover slightly more between transients",
              { { "Release", "94 ms", "Current 120 ms", proposed },
                { "Max reduction", "3.0 dB", "Protected limit unchanged", caution } } },
            { "Trade-off", "May reduce perceived weight",
              { { "Expected benefit", "Clearer kick/body separation", "Preview before accepting", positive } } }
        },
        "Proposal transaction",
        { "Current / Proposed available", "Loudness match: −0.6 dB audition only", "Apply", "Reject", "Undo after apply" }
    },
    {
        "reftime-120",
        "Journey 7 · RefTime before Valhalla",
        "1/32 dotted at 120 BPM",
        "Exact musical delay is ready to copy into a third-party effect workflow. Intentional delay is not reported as processing latency.",
        refTime,
        "Timing",
        "Host timing",
        "120 BPM · 4/4 · 48 kHz",
        { { "HOST SYNC", positive }, { "DELAYED ONLY", proposed }, { "TECH LATENCY 0", neutral } },
        {
            { "Exact timing", "93.750 ms",
              { { "Rounded", "94 ms", "Nearest whole millisecond", neutral },
                { "Samples", "4,500", "48,000 Hz", liveMix },
                { "Division", "1/32 dotted", "3/64 note", reference } } },
            { "Tempo transition", "Clean crossfade",
              { { "New tempo", "128 BPM", "87.891 ms after change", proposed },
                { "Third-party repeat", "Unchanged", "RefTime offsets effect onset only", caution } } }
        },
        "Copy and output",
        { "Copy Exact MS", "Copy Number Only", "Copy Samples", "Internal pre-delay in reverb should be zero" }
    },
    {
        "stem-prep-queue",
        "Journey 8 · Prepare stems offline",
        "Ayo Nights · separating locally",
        "The original is preserved. Temporary output is not visible as complete until validation and final rename succeed.",
        stemPrep,
        "Queue",
        "References folder",
        "Balanced 4-Stem · Automatic device",
        { { "OFFLINE", positive }, { "62%", proposed }, { "CPU FALLBACK READY", neutral } },
        {
            { "Queue", "1 active · 2 waiting",
              { { "Ayo Nights", "Separating 62%", "GPU active", proposed },
                { "River Glass", "Waiting", "Disk/model checks passed", neutral },
                { "Live Room", "Needs attention", "Existing output requires confirmation", caution } } },
            { "Output policy", "Safe temporary-write workflow",
              { { "Original", "Preserved", "Never normalized or overwritten", positive },
                { "Format", "Stereo float WAV", "Equal start and duration", reference },
                { "Validation", "Pending", "Hashes and reconstruction after separation", caution } } }
        },
        "Ayo Nights",
        { "Preparing → Separating → Validating → Finalizing", "Cancel safely", "Open folder after completion", "RefHub refresh after finalization" }
    },
    {
        "low-confidence",
        "Required state · Low confidence",
        "Advice only · insufficient stable evidence",
        "The result remains inspectable, but proposal application is unavailable until the evidence improves.",
        refHub,
        "Overview",
        "Lossy Demo",
        "Verse ↔ Verse",
        { { "LOW CONFIDENCE", lowConfidence }, { "ADVICE ONLY", caution } },
        {
            { "Evidence", "Several independent weaknesses",
              { { "Active duration", "4.1 s", "Minimum target not met", error },
                { "Stem leakage", "High", "Cymbals in vocals", caution },
                { "Encoding", "Lossy", "Reference source", caution } } }
        },
        "How to improve",
        { "Capture a longer active region", "Use the original lossless full mix", "Review stem role mapping", "No automatic apply" }
    },
    {
        "render-safe",
        "Required state · Render Safe",
        "Export detected · monitoring paths disabled",
        "Reference audition, A/B, translation, and loudness-match gain are excluded. Enabled processors continue normally.",
        refHub,
        "Overview",
        "Ayo Nights",
        "Export",
        { { "RENDER SAFE", positive }, { "REFERENCE MUTED", neutral }, { "ANALYSIS PAUSED", neutral } },
        {
            { "Excluded from render", "Monitoring-only paths are inactive",
              { { "Reference audio", "Excluded", "Release-blocking invariant", positive },
                { "Translation", "Excluded", "Phone/small-speaker simulations", positive },
                { "Match gain", "Excluded", "Audition only", positive } } },
            { "Still active", "Production audio path",
              { { "Processors", "Enabled", "Normal configured processing", proposed } } }
        },
        "Render protection",
        { "Audition controls disabled", "No reference transport", "State clears after export", "Automated path test required" }
    },
    {
        "model-missing",
        "Required state · Missing model",
        "Stem model is unavailable",
        "Stem Prep remains usable for library inspection while separation actions are disabled with local repair guidance.",
        stemPrep,
        "Queue",
        "References folder",
        "Model unavailable",
        { { "OFFLINE", positive }, { "MODEL MISSING", error } },
        {
            { "Local model", "No compatible installed model found",
              { { "Expected location", "ProgramData", "Exact path available in diagnostics", neutral },
                { "Queue", "Paused", "No job data lost", caution } } }
        },
        "Repair",
        { "Run installer Repair", "Verify model licence record", "No internet or account required", "CPU/GPU selection unavailable" }
    }
};

} // namespace

const std::vector<Scenario>& allScenarios()
{
    return scenarios;
}

const Scenario* findScenario(const std::string_view id)
{
    const auto iterator = std::find_if(
        scenarios.begin(),
        scenarios.end(),
        [id](const Scenario& scenario) { return scenario.id == id; });
    return iterator == scenarios.end() ? nullptr : &*iterator;
}

std::vector<std::string> validateScenarioCatalogue()
{
    std::vector<std::string> errors;
    std::unordered_set<std::string> ids;
    std::array<bool, 8> journeys {};

    for (const auto& scenario : scenarios) {
        if (scenario.id.empty())
            errors.emplace_back("Scenario has an empty id.");
        else if (!ids.insert(scenario.id).second)
            errors.emplace_back("Duplicate scenario id: " + scenario.id);

        if (scenario.title.empty())
            errors.emplace_back("Scenario " + scenario.id + " has an empty title.");
        if (scenario.panels.empty())
            errors.emplace_back("Scenario " + scenario.id + " has no panels.");
        if (scenario.inspectorLines.empty())
            errors.emplace_back("Scenario " + scenario.id + " has no inspector content.");

        constexpr std::string_view prefix { "Journey " };
        if (scenario.journey.starts_with(prefix) && scenario.journey.size() > prefix.size()) {
            const auto journeyCharacter = scenario.journey[prefix.size()];
            if (journeyCharacter >= '1' && journeyCharacter <= '8')
                journeys[static_cast<std::size_t>(journeyCharacter - '1')] = true;
        }
    }

    for (std::size_t index = 0; index < journeys.size(); ++index) {
        if (!journeys[index])
            errors.emplace_back("Missing Journey " + std::to_string(index + 1) + " scenario.");
    }

    return errors;
}

} // namespace ehvota::preview
