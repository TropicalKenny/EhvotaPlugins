# Execution status

Last updated: 2026-08-13

## Current owner instruction

Development is to continue without routine stop points until Ehvota's Bundle is complete or a task is genuinely blocked by owner input, unavailable credentials/hardware, or evidence that can only be produced in the owner's local Ableton/Windows environment.

This supersedes the earlier workflow instruction in `plan.md` that required development to halt after the UI approval screens.

The UI approval gate still has an important meaning:

- coded RefHub, Sources/Mapping and RefTime screens must be shown to the owner;
- owner feedback must be incorporated before the visual system is considered final;
- visual approval is not permission for unsafe or irreversible architecture changes;
- however, independent production engineering, deterministic tests, protocol/state foundations, plugin skeletons and reversible DSP work may continue while UI review is pending.

## Current implementation state

Work now includes:

- Ableton-first product architecture;
- Windows/MSVC CI and coded screenshot artifacts;
- RefHub Precision Studio UI foundation;
- Sources/Mapping and RefTime approval surfaces;
- automatic Level Match state model;
- Ableton bridge health/state fixtures;
- portable bridge protocol and capability negotiation model;
- proposal validation/explicit-approval safety model;
- reference package/stem/mapping state model;
- production VST3 targets for RefHub, RefTap, RefEQ, RefComp and RefTime;
- initial RefTap transparent metering;
- initial RefHub transparent live measurement path;
- initial RefEQ neutral parametric processing;
- initial RefComp stereo-linked compression;
- initial RefTime host-synchronised intentional-delay processing.

These production processors are foundations, not yet quality-certified releases. Every DSP block still requires dedicated numerical/audio validation before release status.

## Non-negotiable safety rules

- No bridge, Max for Live device, MCP client or network operation may run in the real-time audio thread.
- Loss of Ableton bridge state must never stop or corrupt audio processing.
- Reference Level Match is monitoring-only and must never alter exported project audio.
- RefTap must remain bit-transparent.
- Externally initiated processing changes must become reviewable proposals and cannot bypass explicit approval, freshness, bounds and undo checks.
- Diagnostic stems never replace the authoritative full reference mix.
- No user audio or secrets are committed to the repository.

## Evidence still requiring the owner's Windows/Ableton machine

CI can compile and render Windows targets, but final release evidence still requires the real product environment for:

- Ableton Live 12 Suite VST3 scanning and recall;
- Max for Live Ehvota Bridge lifecycle inside an actual Live Set;
- tempo/meter automation behaviour;
- DAW offline export/render-safe proof;
- Windows DPI/accessibility review on real displays;
- clean-machine installer/repair/uninstall;
- trusted development certificate installation/signing;
- subjective listening and artefact evaluation of colour/dynamics/time-based DSP.

Until those tests are run, CI success is build evidence, not a claim that the product is release-certified.
