# Ehvota's Bundle — Living Execution Plan

Last updated: 2026-08-12
Plan status: Phase 1 UI refinement in progress; production DSP remains blocked
Primary host decision: Ableton Live 12 Suite on Windows x64
Secondary future compatibility target: FL Studio

## Product goal

Build **Ehvota's Bundle**, a Windows 10/11 x64 VST3 audio-plugin and utility suite centred on reference-driven mixing, deterministic audio analysis, linked Ehvota processors, precise musical timing, optional analogue colour, and offline stem preparation.

Ehvota remains a normal C++/JUCE VST3 suite. **Ableton Live 12 Suite** is now the primary DAW because the product's longer-term direction includes deep project-state awareness and controlled Codex/MCP integration. Ableton integration will use an optional Max for Live **Ehvota Bridge** backed by the Live Object Model. The Ehvota audio processors must continue to work without the bridge, without MCP, without Codex, without an account, and without a network connection.

The original full reference remains authoritative. AI-separated stems are diagnostic inputs only. Core analysis and processing do not require AI; only Ehvota Stem Prep may use a local model.

## Confirmed constraints

- [x] Product name: Ehvota's Bundle.
- [x] Vendor: Ehvota.
- [x] Internal identifier: `EhvotasBundle`.
- [x] Package namespace: `com.ehvota.bundle`.
- [x] Product target: Windows 10/11 x64.
- [x] Plugin format: VST3.
- [x] Primary host: **Ableton Live 12 Suite** on Windows.
- [x] Secondary future host: FL Studio; it must not drive Version 1 architecture.
- [x] DAW integration layer: optional Max for Live `Ehvota Bridge` using the Live Object Model.
- [x] Audio processing remains C++20/JUCE VST3, not Max for Live DSP.
- [x] Future Codex/MCP integration is separate from core plugin operation.
- [x] External writes must use the same proposal, validation, permission, and undo path as RefHub.
- [x] No direct external access to audio-thread memory.
- [x] Local/offline operation; no required cloud, account, subscription, API, or audio upload.
- [x] No real-time AI stem separation inside a VST.
- [x] Route A development signing uses a manually trusted self-signed Ehvota certificate.
- [x] No signing private key, password, user audio, model secret, or generated build output may be committed.
- [x] Production DSP is blocked until the revised JUCE UI Preview is reviewed and explicitly approved by the user.
- [x] Current UI direction is not approved; it needs a second refinement pass.

## Current repository state

The repository already contains a working Phase 0/1 foundation rather than being empty:

- CMake workspace and presets.
- JUCE 8.0.9 pinned as a submodule at `f72bad64d29715216226685810c5196bd0d79d77`.
- Shared JUCE UI components and `SuiteShell`.
- Standalone `EhvotaUIDemo` source.
- Deterministic preview scenarios and journey replay models.
- RefTime calculation core.
- Portable tests and Windows screenshot/audit tooling.
- Living documentation and repository instructions.
- No production DSP, final VST3 product set, Stem Prep inference integration, installer, signing release flow, or final Ableton bridge implementation yet.

A previous plan treated FL Studio as the primary host. That host-specific assumption is now superseded by decisions D-008/D-009 in `docs/DECISIONS.md` and `docs/ABLETON_INTEGRATION.md`.

## Current development-host boundary

Repository work can occur on Linux, but Linux remains a source-authoring and portable-test environment only. Windows is the sole product/release target.

Linux may provide evidence for:

- source architecture;
- portable deterministic logic;
- model/view-model tests;
- compile-only checks where explicitly labelled non-product;
- documentation and review.

Linux cannot be final evidence for:

- MSVC builds;
- Windows JUCE rendering;
- Windows DPI/accessibility;
- VST3 host behaviour;
- Ableton Live integration;
- Max for Live integration;
- Authenticode signing;
- Inno Setup;
- clean-machine installation.

## Architecture

```text
/
├── AGENTS.md
├── plan.md
├── CMakeLists.txt
├── CMakePresets.json
├── .agent/PLANS.md
├── apps/
│   ├── ui-preview/
│   └── stem-prep/
├── plugins/
│   ├── refhub/
│   ├── reftap/
│   ├── refeq/
│   ├── refcomp/
│   ├── reftime/
│   ├── refcolor/
│   └── future/
├── ableton/
│   ├── README.md
│   └── bridge/             # Max for Live bridge source/assets when implementation begins
├── shared/
│   ├── analysis/
│   ├── audio/
│   ├── communication/
│   ├── timing/
│   ├── ui/
│   ├── viewmodels/
│   ├── presets/
│   ├── state/
│   ├── safety/
│   ├── logging/
│   └── utilities/
├── tests/
├── benchmarks/
├── tools/
├── installer/
├── docs/
├── licenses/
├── presets/
├── resources/
└── generated/             # ignored screenshots/build/review artifacts
```

### Boundary: Ableton vs Ehvota

**Ableton Bridge / Live Object Model provides project structure and controllable DAW state:**

- Live Set identity/state;
- tracks and return tracks;
- names and stable-enough bridge identities;
- mixer state exposed by the Live API;
- devices and parameters exposed by the Live API;
- sends;
- tempo, meter and transport;
- selected track/device;
- future controlled actions.

**Ehvota plugins provide audio truth:**

- spectra;
- loudness;
- true peak;
- dynamics;
- transients;
- Mid/Side;
- correlation;
- component activity;
- captures;
- reference mappings;
- RefEQ state;
- RefComp state;
- proposal state.

The bridge is optional. RefTap remains necessary because DAW object state does not replace real-time audio measurement.

### External automation boundary

Future architecture:

```text
Codex
  |
 MCP
  |
Ehvota DAW MCP   (separate future repository)
  |
localhost IPC
  |
Ehvota Bridge / External Control Gateway
  |                         \
Live Object Model            RefLink-safe control boundary
  |                            |
Ableton Live                Ehvota plugins
```

Rules:

- MCP is not implemented in this repository during the UI phase.
- No raw audio-thread memory is exposed externally.
- External reads use immutable/snapshot-style state where practical.
- External writes are never arbitrary memory/parameter writes.
- All write-capable operations are validated and permissioned.
- Processing changes default to proposals requiring user approval.
- Undo scope is explicit.
- Loss of bridge/MCP must not stop audio processing.
- No network/cloud dependency is introduced for normal use.

## UI/UX-first strategy — current gate

The first UI preview proved the component foundation but is **not approved**. It looks too much like a monitoring/dashboard application, contains too many permanent navigation destinations, gives insufficient space to the musical timeline/analyser, mixes onboarding/debug UI with production UI, and duplicates status information.

The second refinement pass must keep the good visual foundation while changing information architecture.

### Required primary navigation

Exactly five primary destinations:

1. Compare
2. Sources
3. Process
4. History
5. Settings

Secondary navigation:

**Compare**
- Overview
- Tone
- Dynamics
- Space
- Components

**Sources**
- Library
- Mapping
- Sections
- Validation

**Process**
- Links
- Proposals
- Signal Chain
- Learn

Band Focus is contextual inside Compare. Timing is contextual and also has the standalone RefTime plugin.

### Mandatory second-pass screens

Before production DSP, implement and review:

- [ ] **Screen A — RefHub Active Comparison**: large timeline, large analyser, compact A/B + Level Match + Blind + Capture controls, one prioritised finding, contextual inspector collapsed by default.
- [ ] **Screen B — RefHub Sources / Mapping**: library source state, reference stems, live Ableton sources, mapping matrix, composite builder, disconnected/renamed source handling.
- [ ] **Screen C — RefTime**: host BPM/meter, note value, dotted/triplet, exact milliseconds as visual focus, samples, copy controls, Delayed Only/Dry+Delayed/Dry Only, signal-flow explanation.

### Required Ableton UI states

Add deterministic fixtures for:

- [ ] Ableton Bridge connected and healthy.
- [ ] Ableton Bridge not installed / unavailable.
- [ ] Bridge stale / heartbeat lost while audio remains safe.
- [ ] Track renamed after a saved mapping.
- [ ] Track removed after a saved mapping.
- [ ] Duplicate semantic roles, e.g. two tracks both labelled/assigned `Drums`.
- [ ] Bridge connected but Max for Live integration permission/state is incomplete.
- [ ] MCP unavailable while the bridge and plugins remain usable.
- [ ] External proposal pending approval.
- [ ] External write denied by permission policy.

### UI rules retained

- Dark graphite/charcoal foundation.
- Cyan = live mix.
- Blue/violet = reference.
- Warm accent = proposed/active intervention.
- Green = healthy/safe.
- Amber = caution.
- Red = blocking error.
- No gaming/neon aesthetic.
- No fake rack hardware or photorealistic screws.
- No permanent empty right panel.
- Contextual inspector only.
- Debug controls must be clearly debug-only and absent from release builds.
- Fix UTF-8 rendering and test `Ehvota’s`, arrows, ±, μ, Ω, LUFS, dBFS, Mid/Side.
- Audio/timeline/analyser is the dominant workspace.
- Do not rely on colour alone.
- Support compact/standard/wide layouts and Windows scaling 100/125/150/175/200%.

## Phases

### Phase 0 — Foundation and research

Completed foundation work includes repository bootstrap, durable instructions, JUCE pinning, deterministic UI state infrastructure, timing core, portable tests, and Windows review tooling.

Remaining host-migration tasks:

- [x] Accept Ableton Live 12 Suite as primary host.
- [x] Document Ableton bridge architecture.
- [x] Define future external automation boundary.
- [ ] Remove active FL Studio-first assumptions from current-state and validation documentation.
- [ ] Research/record Ableton Live 12 Suite + Max for Live version/tool requirements from first-party sources before implementation.

### Phase 1 — UI/UX refinement and approval — ACTIVE

- [x] Existing production-component UI preview foundation exists.
- [x] Existing deterministic scenario catalogue and journey replay exist.
- [x] Existing shell/layout/token infrastructure exists.
- [x] User reviewed the first preview and rejected it as final UI.
- [ ] Replace 14-item sidebar with five primary destinations.
- [ ] Implement contextual secondary navigation.
- [ ] Make timeline/analyser dominant in active comparison.
- [ ] Convert permanent right panel to collapsible contextual inspector.
- [ ] Replace permanent journey card with state-based onboarding.
- [ ] Consolidate duplicated reference/status information.
- [ ] Separate debug UI from release UI.
- [ ] Improve typography and secondary-text contrast.
- [ ] Fix and regression-test UTF-8/special characters.
- [ ] Add Ableton bridge deterministic states.
- [ ] Implement Screen A.
- [ ] Implement Screen B.
- [ ] Implement Screen C.
- [ ] Run portable model/layout tests on Linux.
- [ ] Run Windows MSVC build of EhvotaUIDemo.
- [ ] Validate 860×600, 1024×640, 1180×760, 1366×768, 1440×900, 1600×1000 RefHub layouts.
- [ ] Validate 620×420, 720×500, 900×620 RefTime layouts.
- [ ] Validate Windows scale factors 100/125/150/175/200%.
- [ ] Capture required review screenshots.
- [ ] Update `docs/UI_CONTRACT.md`, `docs/DESIGN_SYSTEM.md`, `docs/INFORMATION_ARCHITECTURE.md`, `docs/USER_FLOWS.md`.
- [ ] Stop and request explicit user approval.
- [ ] Record approval before Phase 2.

Acceptance criteria:

- Three required screens exist using shared production components.
- Ableton-specific source/mapping/bridge states are demonstrable.
- Normal comparison no longer resembles an enterprise dashboard.
- Audio timeline/analyser is the visual priority.
- Debug UI is absent from release-mode preview.
- Responsive and DPI evidence exists on Windows.
- User explicitly approves the revised UI.

### Phase 2 — Core engineering skeleton — BLOCKED

Only after UI approval:

- [ ] Production CMake/VST3 targets.
- [ ] Permanent VST3 IDs.
- [ ] Shared state/preset/logging/safety foundations.
- [ ] Real-time thread contracts.
- [ ] RefLink same-process proof.
- [ ] External-control gateway abstraction that does not expose audio-thread memory.
- [ ] Host timing proof in Ableton Live.
- [ ] Render-safe monitoring proof.

### Phase 3 — RefHub / RefTap foundation

- [ ] Reference library/playback/validation/relinking.
- [ ] Mapping Matrix.
- [ ] RefTap bit-transparent path and compact timestamped metrics.
- [ ] Live Monitor.
- [ ] Capture Analysis.
- [ ] Sections, Bands, Dynamics, Space, Components, History, Confidence.
- [ ] Ableton state recall and duplicate/renamed/removed track handling.

### Phase 4 — Ehvota Bridge and Stem Prep

Bridge:

- [ ] Implement minimal read-only Max for Live Ehvota Bridge first.
- [ ] Expose only verified Live Object Model state.
- [ ] Add heartbeat/version/capability negotiation.
- [ ] Handle project reload, track rename/removal, duplicate names and device changes.
- [ ] No processing writes until read-only behaviour is stable.

Stem Prep:

- [ ] Benchmark candidate four-stem models.
- [ ] Verify exact checkpoint/runtime licences before bundling.
- [ ] Offline CPU fallback required.
- [ ] Safe cancellation and atomic completion.
- [ ] Preserve alignment and relative stem levels; no individual normalisation.
- [ ] RefHub folder integration.

### Phase 5 — RefEQ, RefComp, RefTime

- [ ] Production RefEQ and dynamic EQ.
- [ ] Production RefComp.
- [ ] Embedded analysis feeds.
- [ ] Bounded RefHub proposals.
- [ ] RefTime exact host-synchronised calculations, copy readouts, output modes and click-free timing transitions.
- [ ] Validate Ableton tempo automation and meter changes.

### Phase 6 — RefColor and dynamic tools

- [ ] Measured analogue prototypes.
- [ ] Oversampling/aliasing/IMD/DC/smoothing/bypass/compensation validation.
- [ ] RefTransient.
- [ ] RefClip.
- [ ] RefLimit.
- [ ] Cumulative colour analysis.

### Phase 7 — Advanced comparison and controlled external actions

- [ ] Automatic section suggestions.
- [ ] Multi-reference ranges.
- [ ] Reference suitability.
- [ ] Internal masking analysis.
- [ ] Blind A/B/X.
- [ ] Deterministic Learn Mode.
- [ ] RefWidth.
- [ ] Add narrowly scoped Ableton write actions only after read-only bridge stability.
- [ ] All processing writes still route through proposal/permission/undo policy.

### Phase 8 — Time-based effects

- [ ] RefDelay.
- [ ] RefVerb.
- [ ] RefGate.

Only after quality proofs; do not add weak effects to increase plugin count.

### Phase 9 — Installer, Route A signing, release candidate

- [ ] Complete / Plugins Only / Custom installer modes.
- [ ] Include finished VST3 plugins, Stem Prep, offline model, presets and optional Ehvota Bridge assets/install guidance.
- [ ] Safe upgrade/repair/modify/uninstall.
- [ ] Preserve user references/presets/captures by default.
- [ ] Self-signed development certificate workflow.
- [ ] Sign internal binaries and final installer.
- [ ] Clean-machine Ableton Live 12 Suite validation.

## Risk register

### Ableton integration

- Live Object Model coverage may differ from desired high-level actions; expose only verified capabilities.
- Max for Live device lifecycle and Live Set reload/duplication need explicit tests.
- Track names are not sufficient stable identity; bridge mapping needs durable identity/recovery strategy.
- A bridge heartbeat may become stale while audio processing continues; UI must show this without implying audio failure.
- Third-party device parameters may be technically visible, but Version 1 must not grant unrestricted control.

### External automation

- MCP must not become a bypass around RefHub safety.
- Read access and write access require separate capabilities.
- External writes need validation, permission, transaction scope and undo.
- No remote caller may block the audio thread.

### Audio thread

- No heap allocation, blocking lock, file I/O, model inference, UI call, waiting, or unbounded work in audio callbacks.
- Cross-instance lifetime and host shutdown are explicit concerns.
- RefTime intentional delay is not technical latency.
- Reference/monitoring paths must be excluded from renders.

### Stem models

- Quality varies by genre/source.
- Exact model checkpoint licences can differ from framework licences.
- GPU claims require exact tested hardware/runtime evidence.

### UI

- Dense technical information can overwhelm laptop-sized plugin windows.
- The UI must remain musical and task-centred, not dashboard-centred.
- Linux rendering is not Windows DPI evidence.

### Signing/install

- Route A creates trust only on machines where the Ehvota public certificate is deliberately imported.
- Installer must never silently install a root certificate.
- Private key remains external and encrypted.

## Testing strategy

- **Level 0:** formatting, source scans, generated/secret/licence checks.
- **Level 1:** portable unit tests for view models, timing, state, mapping policies and DSP units.
- **Level 2:** integration tests for RefLink, bridge-state models, proposals, render safety and deterministic UI replay.
- **Level 3:** Windows JUCE build/UI, DPI, keyboard/accessibility, VST3 validation, Ableton host tests, Max for Live bridge tests.
- **Level 4:** objective DSP measurement, listening evaluations, clean-machine installer/signature/upgrade/repair/uninstall.

Evidence is scoped: Linux-only results cannot close Windows/Ableton-specific gates.

## Decision log summary

- D-001: Windows x64 is the sole product target.
- D-002: UI approval blocks production DSP.
- D-003: UI Preview uses production components.
- D-005: JUCE 8 Starter / JUCE 8.0.9 pin accepted.
- D-006: Inno Setup provisional default.
- D-007: development VST3 staging uses per-user standard location.
- D-008: Ableton Live 12 Suite is the primary DAW; FL Studio moves to later compatibility.
- D-009: Ehvota Bridge is optional Max for Live integration; external writes share RefHub safety/undo.

See `docs/DECISIONS.md` for full rationale.

## Progress log

### 2026-07-23

- Repository/UI-first foundation created.
- JUCE 8.0.9 pinned.
- Deterministic scenario catalogue, journey replay, responsive shell and Windows review tooling added.
- Portable tests and warnings-enabled compile-only checks passed on Linux.
- Initial UI preview later reached a Windows-rendered state but was not accepted as final design.

### 2026-08-12

- User chose Ableton Live 12 Suite as the primary host.
- Created `feature/ableton-first-foundation`.
- Added `docs/ABLETON_INTEGRATION.md` and `ableton/README.md`.
- Added Ableton-first architecture decisions and future external-automation boundary.
- Opened issue #1 and draft PR #2.
- Rewrote this living plan to remove FL Studio-first architecture and make the second UI refinement the active gate.

## Unresolved questions

- Exact installed Ableton Live 12 Suite build and Max version on the Windows machine.
- Exact Windows toolchain versions after moving/running the branch on Windows.
- Stable mapping identity strategy between saved Ehvota mappings and Live Set objects.
- Final approved typography/icon assets.
- Final four-stem model/runtime after benchmark and redistribution review.
- Exact first set of read-only bridge capabilities after a Live Object Model proof.

## Next three concrete actions

1. Update `docs/CURRENT_STATE.md` and active UI/host documentation to Ableton-first language.
2. Add deterministic Ableton bridge/mapping states to the preview model and tests.
3. Refactor the UI preview into the three mandatory high-fidelity screens, then stop for user approval before production DSP.

## Restart instructions

1. Read `AGENTS.md`, `.agent/PLANS.md`, and this entire plan.
2. Inspect branch/status and preserve unrelated changes.
3. Read `docs/CURRENT_STATE.md`, `docs/DECISIONS.md`, `docs/ABLETON_INTEGRATION.md`, `docs/UI_CONTRACT.md`, and active UI specifications.
4. Work from the first incomplete item in Phase 1.
5. Do not claim Windows/Ableton/Max evidence from Linux-only work.
6. Update plan/docs after each milestone with actual evidence.
7. Stop after three substantially similar failed approaches and change method.
8. Do not begin production DSP until revised UI approval is recorded.