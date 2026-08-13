# Execution status

Last updated: 2026-08-13

## Current owner instruction

Development is to continue without routine stop points until Ehvota's Bundle is complete or a task is genuinely blocked by owner input, unavailable credentials/hardware, or evidence that can only be produced in the owner's local Ableton/Windows environment.

This supersedes the earlier workflow instruction in `plan.md` that required development to halt after the UI approval screens. UI review remains a checkpoint, not an idle point.

## Current implementation state

Work now includes:

- Ableton Live 12 Suite / Windows-first architecture;
- Windows/MSVC CI and coded UI review artifacts;
- approved RefHub Precision Studio visual foundation plus Sources/Mapping and RefTime review surfaces;
- production VST3 targets for RefHub, RefTap, RefEQ, RefComp, RefTime, RefColor and RefTransient;
- reference-folder discovery with one authoritative full mix plus optional diagnostic drums/bass/vocals/music stems;
- explicit analysis scope for full mix, drums, bass, vocals and music;
- section models and section-pair validation;
- capture buffers and post-capture spectrum/loudness/dynamics/spatial analysis;
- gated BS.1770-style loudness and monitoring-only automatic Level Match foundations;
- RefHub reference audition with Render Safe behaviour;
- RefTap transparent peak/RMS/correlation measurement;
- RefEQ six-band parametric EQ with HP/LP, smoothing and exact neutral path;
- RefComp Stereo Linked, Dual Mono and Mid/Side modes with optional host-tempo-synchronised attack/release;
- RefColor 4x oversampled analogue-character foundation with calibrated gain and DC protection;
- RefTransient stereo-linked transient shaping;
- proposal validation/explicit-approval policy;
- read-only Ableton Max for Live bridge source and bridge-state fixtures;
- deterministic portable regression tests across timing, analysis, mapping, safety and DSP primitives.

These processors are engineering foundations, not release-certified mastering claims. Objective measurements, Windows/Ableton host evidence and listening tests are still required.

## RefLink architecture correction

The original `RefLinkRegistry` singleton is retained only as an **in-process simulation/test backend**. A static singleton compiled into each VST3 DLL is not a valid product transport because RefHub, RefTap, RefEQ, RefComp and the other plugins are separate modules and therefore receive separate copies of static-library state.

The product RefLink boundary is now:

- a fixed-layout, versioned shared protocol with no pointers;
- Windows named shared memory in the current user session (`Local\\EhvotaRefLink_v1`);
- a named synchronization object used only from non-realtime code;
- fixed-size instance records, generations, heartbeats and stale-slot recovery;
- fixed-size proposal mailboxes with bounded parameter identifiers and values;
- no sockets, JSON, file I/O, mutex waits or OS IPC from an audio callback;
- audio callbacks publish only to local atomics; processor-owned message-thread timers copy snapshots to RefLink;
- target plugins poll approved proposal mailboxes off the audio thread and apply only validated Ehvota parameter operations.

If shared memory is unavailable or version-incompatible, Ehvota audio processing continues normally and the link is reported unavailable/degraded.

## Non-negotiable safety rules

- No bridge, Max for Live device, MCP client, shared-memory lock, socket, file I/O or network operation may run in the real-time audio thread.
- Loss of Ableton Bridge or RefLink must never stop or corrupt audio processing.
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
- cross-DLL RefLink lifecycle under real host load/unload/duplicate scenarios;
- Windows DPI/accessibility review on real displays;
- clean-machine installer/repair/uninstall;
- trusted development certificate installation/signing;
- subjective listening and artefact evaluation of colour/dynamics/time-based DSP.

Until those tests are run, CI success is build evidence, not a claim that the product is release-certified.
