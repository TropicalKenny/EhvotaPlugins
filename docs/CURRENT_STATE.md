# Current state

Last verified: 2026-08-12

## Summary

Ehvota's Bundle is in **Phase 1 UI refinement**. The repository already contains the JUCE/CMake UI-first foundation, deterministic preview scenarios, timing logic, shared UI components, documentation and tests. Production DSP is still intentionally blocked until the revised UI is explicitly approved.

The product target remains Windows 10/11 x64 VST3, but the primary DAW has changed from FL Studio to **Ableton Live 12 Suite**. FL Studio is now a later compatibility target and must not drive Version 1 architecture.

The audio processors remain C++/JUCE VST3 plugins. Ableton-specific integration will be optional and will use a Max for Live **Ehvota Bridge** backed by the Live Object Model. RefTap remains necessary for actual audio measurements. The bridge supplies DAW/project state; Ehvota supplies audio truth.

There is no production DSP, final VST3 suite, production Max for Live bridge, Stem Prep inference integration, installer, signing release flow or MCP server yet.

## Primary architecture decision

```text
Ableton Live 12 Suite
   |                     \
Live Object Model         Ehvota VST3 processors
   |                         |
Ehvota Bridge.amxd           RefLink / analysis
   |                         |
   +----------- controlled local boundary -----------+
```

Future Codex/MCP integration will be a separate project/repository and may consume the bridge/control boundary. It must not directly access audio-thread memory and must not bypass RefHub proposal, permission, validation or undo rules.

Core Ehvota plugins must continue working when:

- Max for Live is unavailable;
- Ehvota Bridge is not loaded;
- MCP is not installed;
- Codex is not running;
- the machine is offline.

## UI state

The first UI preview is **not approved as final**. It successfully proved the reusable component foundation, but review identified these problems:

- too many permanent navigation destinations;
- monitoring/dashboard visual hierarchy instead of a musical workflow;
- waveform/analyser too small;
- setup/validation information dominating normal comparison;
- onboarding/debug UI mixed into production UI;
- permanent right panel wasting space;
- duplicated status information;
- ambiguous top-toolbar controls;
- weak analyser labelling/axes;
- insufficient song timeline emphasis;
- small/dim secondary typography;
- UTF-8/special-character rendering defect visible in the preview.

The active UI direction remains a refined **Precision Studio** approach using the existing graphite/cyan/reference colour foundation, not a wholesale visual reset.

The next approval package must contain three production-component screens:

1. RefHub Active Comparison.
2. RefHub Sources / Mapping.
3. RefTime.

Primary RefHub navigation is now fixed to:

- Compare
- Sources
- Process
- History
- Settings

## Existing completed foundation

- `plan.md`, `AGENTS.md`, `.agent/PLANS.md` and repository rules exist.
- CMake workspace and presets exist.
- JUCE 8.0.9 is pinned at `f72bad64d29715216226685810c5196bd0d79d77`.
- Shared UI components and `SuiteShell` exist.
- `EhvotaUIDemo` source exists.
- Deterministic scenario catalogue and journey replay exist.
- RefTime calculation core exists.
- Portable tests exist and have previously passed on Linux.
- Responsive policies and Windows screenshot/audit tooling exist.
- `docs/ABLETON_INTEGRATION.md` documents the new bridge boundary.
- `ableton/README.md` scaffolds the host-integration area.
- GitHub issue #1 tracks the Ableton-first migration/UI refinement.
- Draft PR #2 contains the migration work.

## Current branch

`feature/ableton-first-foundation`

The branch currently contains:

- Ableton-first architecture decisions;
- Ehvota Bridge integration documentation;
- external automation safety boundary;
- rewritten `plan.md` with Ableton as primary host;
- this current-state update.

## Host boundary

Linux may be used for source editing, documentation and portable tests, but remains non-product evidence.

Windows is required for authoritative evidence of:

- MSVC builds;
- JUCE Windows rendering;
- per-monitor DPI/scaling;
- Windows accessibility behaviour;
- VST3 validation;
- Ableton Live 12 Suite behaviour;
- Max for Live bridge behaviour;
- Authenticode signing;
- Inno Setup;
- clean-machine install/upgrade/repair/uninstall.

## Active work

1. Remove remaining active FL Studio-first assumptions from documentation and preview states.
2. Add deterministic Ableton bridge/mapping scenarios.
3. Refactor RefHub UI around the five-item navigation and dominant musical workspace.
4. Produce the required Active Comparison, Sources/Mapping and RefTime screens.
5. Run portable tests on Linux.
6. Build/run/capture the revised UI on Windows.
7. Stop for explicit UI approval.

## Release blockers

- Revised UI has not yet passed user approval.
- Ableton-specific bridge identity/state contracts are not yet proven.
- Windows/MSVC/Ableton/Max validation for the new architecture is incomplete.
- Production DSP is intentionally blocked.
- Stem Prep model/runtime and licences remain unselected.
- Route A signing is designed but not implemented as a release flow.

## Immediate next step

Add deterministic UI/model states for the Ableton Bridge and mapping lifecycle before changing production DSP or host control code.