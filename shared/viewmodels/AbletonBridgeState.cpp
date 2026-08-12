#include "AbletonBridgeState.h"

#include <algorithm>
#include <unordered_set>

namespace ehvota::preview {
namespace {

using enum AbletonBridgeHealth;
using enum ExternalControlPolicy;

const std::vector<AbletonBridgeState> states {
    {
        "ableton-bridge-healthy",
        "Ableton Bridge connected",
        "Live Set state is current. Ehvota audio analysis remains local and independent of MCP.",
        healthy,
        proposeOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        92,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-drums-01", "DRUM BUS", "DRUM BUS", "Drums", true, true, false },
            { "track-bass-01", "BASS BUS", "BASS BUS", "Bass", true, true, false },
            { "track-vocal-01", "VOCAL BUS", "VOCAL BUS", "Vocals", true, true, false },
            { "track-music-01", "MUSIC BUS", "MUSIC BUS", "Music", true, true, false }
        },
        { "Bridge heartbeat current", "RefHub proposals allowed", "External writes still require approval" }
    },
    {
        "ableton-bridge-unavailable",
        "Ableton Bridge unavailable",
        "Max for Live integration is not currently available. Core VST3 processing and RefTap analysis remain usable.",
        unavailable,
        disabled,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "Not connected",
        0,
        false,
        false,
        true,
        false,
        false,
        {},
        { "Install or load Ehvota Bridge to discover Live Set state", "Do not block audio processing", "Manual RefHub mappings remain available" }
    },
    {
        "ableton-bridge-stale",
        "Ableton Bridge data is stale",
        "The last valid Live Set snapshot is preserved but cannot be used as current evidence until heartbeat health returns.",
        stale,
        inspectOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        32800,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-drums-01", "DRUM BUS", "DRUM BUS", "Drums", true, false, false },
            { "track-bass-01", "BASS BUS", "BASS BUS", "Bass", true, false, false }
        },
        { "Last valid snapshot remains visible", "Proposal creation paused", "Audio path is unaffected" }
    },
    {
        "ableton-track-renamed",
        "Mapped Ableton track was renamed",
        "The mapping keeps its stable bridge key and shows both the saved name and current Live Set name before confirmation.",
        healthy,
        proposeOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        74,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-drums-01", "DRUMS MASTER", "DRUM BUS", "Drums", true, true, true }
        },
        { "Saved mapping name: DRUM BUS", "Current name: DRUMS MASTER", "Do not create a duplicate mapping silently" }
    },
    {
        "ableton-track-removed",
        "Mapped Ableton track was removed",
        "The mapping is preserved as disconnected so history and reference assignments are not silently redirected.",
        healthy,
        inspectOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        81,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-music-01", "", "MUSIC BUS", "Music", false, false, false }
        },
        { "Saved mapping preserved", "Automatic remap disabled", "Relink requires explicit user choice" }
    },
    {
        "ableton-duplicate-role",
        "Two Live tracks share the Drums role",
        "The bridge reports both candidates and RefHub must require an explicit mapping or composite decision.",
        healthy,
        inspectOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        68,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-drums-01", "DRUM BUS A", "", "Drums", true, true, false },
            { "track-drums-02", "DRUM BUS B", "", "Drums", true, true, false }
        },
        { "No automatic winner", "Allow explicit single-source mapping or composite", "Proposal generation waits for resolved mapping" }
    },
    {
        "ableton-bridge-incomplete",
        "Ableton Bridge setup is incomplete",
        "Max for Live is present but the Ehvota Bridge cannot expose the required read-only Live Set capabilities yet.",
        incompatible,
        disabled,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        0,
        true,
        false,
        true,
        false,
        false,
        {},
        { "Core plugins remain usable", "Show exact missing capability", "Do not fall back to GUI automation" }
    },
    {
        "mcp-unavailable",
        "Codex/MCP is unavailable",
        "The Ableton Bridge and Ehvota plugins remain fully usable. MCP is an optional external consumer, not a runtime dependency.",
        healthy,
        proposeOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        63,
        true,
        false,
        true,
        false,
        false,
        {
            { "track-drums-01", "DRUM BUS", "DRUM BUS", "Drums", true, true, false }
        },
        { "Bridge healthy", "MCP disconnected", "RefHub manual and automatic proposal workflows remain available" }
    },
    {
        "external-proposal-pending",
        "External proposal awaiting approval",
        "An external client may create a bounded proposal, but RefHub remains the approval surface before processing changes are applied.",
        healthy,
        proposeOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        58,
        true,
        true,
        true,
        true,
        false,
        {
            { "track-drums-01", "DRUM BUS", "DRUM BUS", "Drums", true, true, false }
        },
        { "Proposal source: external control boundary", "Affected processors must be listed", "Apply requires explicit approval", "Undo transaction required" }
    },
    {
        "external-write-denied",
        "External write denied",
        "The requested action exceeds the current permission policy. No processing state changes were made.",
        healthy,
        inspectOnly,
        "Ehvota Mix Session",
        "Live 12 Suite",
        "0.1-dev",
        61,
        true,
        true,
        true,
        false,
        true,
        {
            { "track-bass-01", "BASS BUS", "BASS BUS", "Bass", true, true, false }
        },
        { "Read access remains available", "No direct parameter write occurred", "User may elevate policy only through explicit controls" }
    }
};

} // namespace

const std::vector<AbletonBridgeState>& allAbletonBridgeStates()
{
    return states;
}

const AbletonBridgeState* findAbletonBridgeState(const std::string_view id)
{
    const auto iterator = std::find_if(states.begin(), states.end(), [id](const auto& state) { return state.id == id; });
    return iterator == states.end() ? nullptr : &*iterator;
}

bool bridgeCanInspect(const AbletonBridgeState& state) noexcept
{
    if (state.health != AbletonBridgeHealth::healthy)
        return false;

    return state.controlPolicy == ExternalControlPolicy::inspectOnly
        || state.controlPolicy == ExternalControlPolicy::proposeOnly
        || state.controlPolicy == ExternalControlPolicy::approvedWrites;
}

bool bridgeCanCreateProposal(const AbletonBridgeState& state) noexcept
{
    if (state.health != AbletonBridgeHealth::healthy)
        return false;

    return state.controlPolicy == ExternalControlPolicy::proposeOnly
        || state.controlPolicy == ExternalControlPolicy::approvedWrites;
}

bool bridgeCanApplyApprovedWrite(const AbletonBridgeState& state) noexcept
{
    return state.health == AbletonBridgeHealth::healthy
        && state.controlPolicy == ExternalControlPolicy::approvedWrites
        && !state.externalWriteDenied;
}

std::vector<std::string> validateAbletonBridgeStates()
{
    std::vector<std::string> errors;
    std::unordered_set<std::string> ids;

    constexpr std::string_view requiredIds[] {
        "ableton-bridge-healthy",
        "ableton-bridge-unavailable",
        "ableton-bridge-stale",
        "ableton-track-renamed",
        "ableton-track-removed",
        "ableton-duplicate-role",
        "ableton-bridge-incomplete",
        "mcp-unavailable",
        "external-proposal-pending",
        "external-write-denied"
    };

    for (const auto& state : states) {
        if (state.id.empty())
            errors.emplace_back("Ableton bridge state has an empty id.");
        else if (!ids.insert(state.id).second)
            errors.emplace_back("Duplicate Ableton bridge state id: " + state.id);

        if (state.title.empty())
            errors.emplace_back("Ableton bridge state " + state.id + " has an empty title.");
        if (state.summary.empty())
            errors.emplace_back("Ableton bridge state " + state.id + " has an empty summary.");
        if (!state.audioProcessingSafe)
            errors.emplace_back("Fixture violates the architecture invariant that bridge failure must not make audio processing unsafe: " + state.id);
    }

    for (const auto requiredId : requiredIds) {
        if (!ids.contains(std::string(requiredId)))
            errors.emplace_back("Missing required Ableton bridge fixture: " + std::string(requiredId));
    }

    const auto* stale = findAbletonBridgeState("ableton-bridge-stale");
    if (stale != nullptr && bridgeCanCreateProposal(*stale))
        errors.emplace_back("Stale bridge state must not be eligible to create proposals.");

    const auto* denied = findAbletonBridgeState("external-write-denied");
    if (denied != nullptr && bridgeCanApplyApprovedWrite(*denied))
        errors.emplace_back("Denied external write fixture must never be eligible for apply.");

    const auto* pending = findAbletonBridgeState("external-proposal-pending");
    if (pending != nullptr && !bridgeCanCreateProposal(*pending))
        errors.emplace_back("Pending external proposal fixture must be proposal-capable.");

    return errors;
}

} // namespace ehvota::preview
