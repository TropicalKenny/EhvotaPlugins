# Ehvota's Bundle — Living Execution Plan

Last updated: 2026-07-23
Plan status: Phase 0 in progress
Authoritative product specification: the initial "EHVOTA'S BUNDLE — MASTER CODEX GOAL" supplied by the user.

## Product goal

Build **Ehvota's Bundle**, a Windows 10/11, 64-bit VST3 audio-plugin and utility suite for unbridged 64-bit use in FL Studio. The suite will combine reference playback and comparison, full-mix and stem-to-bus analysis, section-aware tonal/dynamic/spatial analysis, connected EQ and compressor control, tempo-synchronised timing, clean and optional analogue-coloured processing, and an offline local-AI stem-preparation application. It will ship through one signed Windows installer.

The product must be stable, predictable, offline, explainable, recoverable, maintainable, and safe. It must never upload audio, require an account or subscription, silently master audio, silently change remote parameters, or imply unsupported certainty. Core analysis and processing must work without AI; only Ehvota Stem Prep may use a local model.

## Confirmed constraints

- [x] Product name: Ehvota's Bundle.
- [x] Vendor: Ehvota.
- [x] Internal identifier: `EhvotasBundle`.
- [x] Package namespace: `com.ehvota.bundle`.
- [x] Target: Windows 10 and Windows 11, x64 only.
- [x] Plugin format: VST3; initial host configuration is unbridged 64-bit.
- [x] Primary host: FL Studio.
- [x] Language/build tools: C++20, CMake, MSVC through Visual Studio or Build Tools.
- [x] Framework: a current stable JUCE release after licence verification.
- [x] Installer default: Inno Setup, subject to documented research.
- [x] Local/offline operation; no cloud, account, subscription, required API, or audio upload.
- [x] No arbitrary third-party plugin control and no real-time stem inference inside a VST.
- [x] Route A development signing uses a manually trusted self-signed Ehvota certificate.
- [x] No signing private key, password, secret, user audio, or generated build output may be committed.
- [x] Production DSP may not begin until the JUCE UI Preview is reviewed and the user explicitly approves the UI direction.

## Current repository and host state

Read-only audit performed on 2026-07-23 before any repository mutation.

- Working directory: `/home/kenny/EhvotaPlugins`.
- The directory contained only empty `.git`, `.agents`, and `.codex` directories.
- The `.git` directory was an empty environment placeholder; Git reported that the working directory was not a repository. There was therefore no Git status, tracked-file list, branch, or history to inspect.
- No source, CMake, JUCE, VST3, installer, UI, test, documentation, legal, licence, generated, or signing-material files existed.
- No existing or duplicate implementations were found.
- Current execution host: Arch Linux x86_64, kernel `7.0.12-arch1-1`.
- Available here: CMake 4.3.4, Ninja 1.13.2, Git 2.54.0, Python 3.14.5, GCC/G++ 16.1.1.
- Not found here: MSVC `cl`, MSBuild, Visual Studio/`devenv`, Inno Setup `iscc`, PowerShell, Wine, Steinberg VST3 validator, or Clang.
- `/usr/bin/signtool` is a Linux/NSS utility, not the Windows SDK Authenticode signing tool, and is unsuitable for Route A signing.
- This host can support source authoring, documentation, architecture checks, selected portable tests, and possibly a non-product Linux UI preview used only as a development aid. It cannot provide authoritative evidence for the Windows-only build, Windows DPI behaviour, Authenticode signing, Inno Setup, VST3 validation on Windows, or FL Studio integration.
- The intended execution environment is a Windows computer. Before any Windows-specific gate is marked complete, the repository must be moved to or opened on that machine and the exact Windows toolchain must be audited there.

## Assumptions requiring validation

- [ ] Confirm the exact Windows edition/build, CPU, RAM, GPU, display resolutions, and scaling levels.
- [ ] Confirm installed Visual Studio/Build Tools workloads and Windows SDK version.
- [ ] Confirm installed FL Studio version/edition and its VST3 scanning paths.
- [x] Confirm whether JUCE licensing terms are acceptable for this personal project and any future distribution. User selected JUCE 8 Starter and accepted the JUCE 8 EULA on 2026-07-23.
- [x] Confirm whether JUCE will be fetched reproducibly or vendored after licence/maintenance review. JUCE 8.0.9 is pinned as a Git submodule at `f72bad64d29715216226685810c5196bd0d79d77`.
- [ ] Confirm the exact Inno Setup version and whether it meets repair/modify requirements without a different packaging layer.
- [ ] Confirm the Windows audio-device and GPU environment for Stem Prep benchmarks.
- [ ] Confirm whether the reference library must support non-ASCII paths and Windows long paths in Version 1.
- [ ] Confirm whether the UI preview may use a temporary system font before the bundled typeface is selected.
- [ ] Confirm plugin permanent VST3 class IDs before the first usable release.

## Research questions

- [x] What is the current stable JUCE release, supported CMake/MSVC baseline, VST3 support status, licence, and redistribution model? Initial decision recorded in `docs/DECISIONS.md` and `docs/THIRD_PARTY_NOTICES.md`; Windows compatibility remains to be executed.
- [ ] Which official FL Studio behaviours and limitations affect tempo, time signature, Smart Disable, offline rendering, scaling, plugin delay compensation, and VST3 state?
- [ ] Which analyser and reference workflows are most successful in current professional tools, without copying their visual identity?
- [ ] Which accessible Windows typeface best satisfies tabular-numeral, DPI, licence, and redistribution requirements?
- [ ] Which Windows accessibility APIs and JUCE facilities cover keyboard focus, screen readers, contrast, and reduced animation?
- [ ] What deterministic screenshot and UI-state test strategy is reliable for JUCE on Windows?
- [ ] Which local four-stem models and runtimes have verified redistribution licences and acceptable CPU fallback?
- [ ] Which Authenticode development-signing and timestamp options work safely with a self-signed local certificate?
- [ ] Does Inno Setup adequately support Complete, Plugins Only, Custom, upgrade, repair, modify, preserved personal data, and signed payloads?

## Proposed repository architecture

This architecture is provisional until the Windows toolchain and JUCE dependency are verified:

```text
/
├── AGENTS.md
├── plan.md
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
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
├── shared/
│   ├── analysis/
│   ├── audio/
│   ├── communication/
│   ├── timing/
│   ├── ui/
│   ├── presets/
│   ├── state/
│   ├── safety/
│   ├── logging/
│   └── utilities/
├── tests/
│   ├── unit/
│   ├── dsp/
│   ├── integration/
│   ├── ui/
│   ├── host/
│   └── fixtures/
├── benchmarks/
├── tools/
├── installer/
├── docs/
├── licenses/
├── presets/
├── resources/
└── generated/       # ignored review/build artefacts
```

Boundaries:

- `shared/ui` owns reusable production UI components, tokens, layout primitives, accessibility behaviours, and view contracts used by the preview and plugins.
- `apps/ui-preview` is the authoritative interactive Phase 1 prototype using deterministic mock/replay data; it must not become a disposable fork.
- DSP, analysis, communication, timing, safety, state, presets, logging, and UI remain distinct modules.
- Audio-thread code may communicate only through explicitly real-time-safe interfaces.
- Generated screenshots, recordings, build trees, caches, and packages stay outside source directories.

## UI/UX-first strategy

1. Research current official platform/framework guidance and a focused set of workflow competitors.
2. Document users, contexts, information architecture, and all eight required journeys.
3. Compare at most three coherent visual directions.
4. Recommend one direction based on readability, workflow, originality, and feasible JUCE implementation.
5. Define production tokens, typography, semantics, layout/breakpoints, interaction standards, accessibility, help, performance, and all required states.
6. Build Ehvota UI Preview from the same `shared/ui` production components and deterministic view-model fixtures intended for plugins.
7. Exercise every primary journey; verify continuous resizing, Windows DPI levels, keyboard interaction, state coverage, and UI performance on Windows.
8. Capture review screenshots in an ignored/generated-artifact location.
9. Complete self-review, documentation, and a coherent Git commit.
10. Stop and request explicit user approval. Do not implement production DSP before approval.

## Phases and granular tasks

### Phase 0 — Audit, research, and planning

- [x] Read the complete master goal.
- [x] Audit the initial directory without mutation.
- [x] Audit the current host toolchain without exposing secrets.
- [x] Identify the empty/non-repository state.
- [x] Create `plan.md` as the first repository mutation.
- [x] Initialize a valid Git repository after the required first mutation.
- [x] Add concise `AGENTS.md`.
- [x] Add `.agent/PLANS.md`.
- [x] Verify fresh Codex instruction loading. An ephemeral read-only GPT session reproduced the first root rule without tool or file access on 2026-07-23.
- [x] Add an appropriate `.gitignore` before builds or screenshots.
- [x] Create only the minimum active documentation, populated with evidence.
- [x] Research official JUCE, CMake/MSVC, VST3, Windows accessibility/DPI, FL Studio, and Inno Setup sources for the initial architecture.
- [x] Record initial dependency and licence findings.
- [x] Establish Windows environment bootstrap/audit instructions and `tools/check-windows-dev.ps1`.
- [x] Establish the initial risk register and decision log.

Acceptance criteria:

- Repository guidance and a restartable plan exist.
- Current state, constraints, risks, unresolved questions, and evidence are explicit.
- A valid Git repository exists without generated or secret files.
- No production dependency is accepted without licence and maintenance review.
- The intended Windows toolchain is either verified on Windows or clearly marked pending.

### Phase 1 — UI/UX foundation and approval

- [x] Define personas and studio usage contexts.
- [x] Document the eight required primary journeys and their error/recovery paths.
- [x] Define RefHub information architecture and individual plugin/app shells.
- [x] Produce low-fidelity wireframes.
- [x] Compare no more than Precision Studio, Warm Technical, and Hybrid Signal.
- [x] Select and justify one recommended direction.
- [x] Define design tokens, typography, colour semantics, spacing, sizing, graph styles, and icon rules.
- [x] Define keyboard map, exact-entry behaviour, help/tooltips, undo/redo, A/B, and focus behaviour.
- [x] Define empty/loading/offline/error/degraded/render/unsupported states.
- [x] Define compact/standard/wide layouts and minimum/default sizes.
- [x] Define 100%, 125%, 150%, 175%, and 200% Windows scaling validation.
- [x] Define UI performance budgets and reduced-motion behaviour.
- [x] Pin accepted JUCE 8.0.9 source at official tag commit `f72bad64d29715216226685810c5196bd0d79d77`.
- [ ] Establish JUCE and the Windows MSVC/CMake build.
- [ ] Compile and review the shared production UI component library on Windows. Source exists; JUCE/Windows build is unverified.
- [x] Run a non-product compile-only C++20 syntax/warning check against pinned JUCE headers. All UI/preview translation units passed with `-Wall -Wextra -Wpedantic`; no link, render, or Windows claim is implied.
- [x] Create deterministic mock/replay view models for all eight journeys and required low-confidence/render-safe/missing-model states.
- [ ] Build Ehvota UI Preview using those shared components. Target/source exists; no JUCE/Windows build evidence yet.
- [ ] Demonstrate every required screen and journey.
- [x] Add initial meaningful model/timing/token tests and replay states. Portable suite passed on 2026-07-23.
- [ ] Run Windows builds and tests.
- [ ] Verify continuous resize, DPI, keyboard, long labels, accessibility basics, and major states.
- [ ] Capture required screenshots and an interaction recording where practical.
- [ ] Complete the UI checklist and update `docs/UI_CONTRACT.md`.
- [ ] Review the complete diff and commit the UI phase.
- [ ] Present evidence and request explicit user approval.
- [ ] Record the user's approval before Phase 2.

Acceptance criteria:

- The authoritative prototype is a standalone Windows JUCE app built from production components.
- All eight journeys and mandatory states are demonstrable with deterministic data.
- Windows resize/DPI/keyboard evidence and screenshots exist.
- Documentation describes actual implemented behaviour.
- User explicitly approves the UI direction.

### Phase 2 — Core engineering skeleton (blocked by UI approval)

- [ ] Establish production CMake workspace and Windows presets.
- [ ] Implement shared state, presets, logging, safety, and test harness.
- [ ] Create VST3 shells and permanent class-ID process.
- [ ] Prove audio-thread contracts.
- [ ] Prove RefLink same-process registration/metrics path.
- [ ] Prove host timing and intentional-delay/technical-latency separation.
- [ ] Prove render-safe monitoring separation.

### Phase 3 — RefHub and RefTap foundation

- [ ] Implement reference library, playback, validation, relinking, mappings, and metadata.
- [ ] Implement RefTap bit-transparent path and compact metrics.
- [ ] Implement Live Monitor and Capture Analysis foundations.
- [ ] Implement sections, bands, loudness, dynamics, space, components, history, and confidence.
- [ ] Validate state recall, failure paths, real-time safety, and FL Studio integration.

### Phase 4 — Stem Prep

- [ ] Benchmark candidate models on the controlled corpus.
- [ ] Verify checkpoint/runtime licences before distribution.
- [ ] Implement offline queue, CPU fallback, optional proven GPU path, safe cancellation, temporary output, atomic completion, metadata, hashes, and validation.
- [ ] Integrate completed folders with RefHub.

### Phase 5 — RefEQ, RefComp, and RefTime

- [ ] Implement validated production DSP and shared analysis feeds.
- [ ] Implement pre/post comparison and bounded manual proposals.
- [ ] Implement RefTime conversions, copying, output modes, and click-free tempo transitions without misreporting musical delay as latency.
- [ ] Complete targeted FL Studio validation.

### Phase 6 — RefColor and dynamic tools

- [ ] Prototype the simplest measured analogue architecture that meets quality targets.
- [ ] Validate oversampling, aliasing, IMD, DC, smoothing, bypass, compensation, latency, and sample-rate stability.
- [ ] Implement RefTransient, RefClip, RefLimit, and cumulative-colour analysis only after proofs.

### Phase 7 — Advanced comparison

- [ ] Add section suggestions, multi-reference ranges, suitability, masking, blind A/B/X, deterministic Learn Mode, advanced history, and RefWidth.

### Phase 8 — Time-based effects

- [ ] Add RefDelay, RefVerb, and RefGate only after quality proofs.

### Phase 9 — Installer and release candidate

- [ ] Build Complete, Plugins Only, and Custom installer modes.
- [ ] Implement safe upgrade/repair/modify/uninstall preserving personal content.
- [ ] Implement Route A signing without repository secrets.
- [ ] Verify signatures, VST3 packages, installer contents, clean install/uninstall, and Windows/FL Studio acceptance.

## Dependencies

- Windows 10/11 x64 development machine.
- Visual Studio or Build Tools with MSVC C++ workload and Windows SDK.
- CMake and optionally Ninja compatible with the verified JUCE release.
- Current stable JUCE after licence and maintenance review.
- Steinberg VST3 validation tooling where distributable/available.
- FL Studio x64 for authoritative host validation.
- Inno Setup unless research changes the choice.
- Windows SDK `signtool.exe` and an external encrypted Route A certificate/private key.
- A redistribution-cleared offline model/runtime for Stem Prep, selected only after benchmarks.
- Properly licensed bundled font/icons, or documented system-font strategy.

## Risk register

### Legal and licence risks

- JUCE licence suitability and obligations are not yet verified.
- VST3 SDK/licensing and validator distribution constraints require verification.
- Stem-model checkpoints and inference runtimes may have incompatible or ambiguous redistribution terms.
- Fonts, icons, fixtures, reference audio, and competitor screenshots require explicit rights.
- Self-signed binaries must not imply public trust.

### Audio-thread risks

- Cross-instance communication, analyser capture, UI telemetry, and state changes can cause allocation, locking, unbounded work, or lifetime races.
- Host shutdown, duplicate instances, transport jumps, block-size/sample-rate changes, and offline rendering need explicit testable contracts.
- Intentional RefTime delay must remain separate from reported technical latency.
- Monitoring/reference paths must be provably excluded from renders.

### FL Studio integration risks

- Current host cannot verify FL Studio behaviour.
- Smart Disable, tempo/meter context, project reload, scaling, automation, offline render, PDC, bridged instances, and duplicate-registration behaviour require real Windows/FL Studio evidence.
- Initial RefLink is unbridged/same-process only; unsupported bridge cases must be visible.

### Stem-model risks

- Model quality varies by genre and source.
- Licence, runtime size, CPU speed, GPU support, RAM, reconstruction, leakage, artefacts, deterministic recovery, and output alignment are unproven.
- GPU claims must be limited to exact tested hardware/runtime combinations.

### Signing and installer risks

- No Windows signing or installer tool exists in the current host.
- Private-key handling, timestamp behaviour, signing internal VST3 binaries, upgrade identity, repair/modify semantics, and preserved user data need Windows validation.
- The installer must never silently trust a certificate.

### UI risks

- Linux rendering is not authoritative for Windows fonts, scaling, ClearType, input, accessibility, or JUCE per-monitor DPI behaviour.
- Dense analysis views may conflict with laptop readability and performance.
- Graph motion must remain decoupled from DSP and reduced-motion compatible.

## Performance budgets to establish

Initial budgets are research tasks, not yet validated promises:

- [ ] Audio callback time/CPU per plugin at target sample rates and buffer sizes.
- [ ] Zero allocations, blocking locks, file I/O, logging, UI calls, model inference, or unbounded loops in audio callbacks.
- [ ] RefTap CPU and memory budget.
- [ ] RefLink message rate, queue capacity, drop policy, and capture limits.
- [ ] UI frame-rate/refresh budgets for idle, live graphs, and reduced-motion modes.
- [ ] UI startup, resize, state-switch, and screenshot-test tolerances.
- [ ] Plugin memory budget and analysis-cache limits.
- [ ] Stem Prep CPU throughput, RAM, model footprint, disk-space, cancellation, and recovery budgets.
- [ ] Installer size and install/repair/uninstall duration budgets.

## Testing strategy

- Level 0: formatting, static file checks, CMake syntax/config checks, licence/secret/generated-file scans.
- Level 1: focused unit tests for view models, timing maths, state, safety policies, and individual DSP units.
- Level 2: component and integration tests for RefLink, library validation, proposals, render safety, and deterministic UI replay states.
- Level 3: Windows JUCE UI screenshot/interaction, DPI, keyboard, accessibility, performance, VST3 validation, and DAW harness tests.
- Level 4: FL Studio workflow matrix, objective DSP measurements, listening evaluations, clean-machine installer/signature/upgrade/repair/uninstall validation.

Tests are evidence only for the requirements they actually cover. Windows-only gates cannot be marked complete from Linux-only results.

## Decision log

### D-001 — Windows is the sole product target

Date: 2026-07-23
Status: Confirmed
Decision: Author and package only the Windows 10/11 x64 VST3 product. Linux builds, if temporarily used for portable logic or early UI development, are non-product development aids and never release evidence.
Reason: Explicit user constraint reinforced during the initial audit.

### D-002 — UI approval blocks production DSP

Date: 2026-07-23
Status: Confirmed
Decision: Build and validate the reusable JUCE production-component preview, then stop for explicit approval before production DSP.
Reason: Prevent interaction architecture from being forced by premature DSP implementation.

### D-003 — Do not infer a valid Git repository from the `.git` placeholder

Date: 2026-07-23
Status: Confirmed
Decision: Record the empty placeholder accurately and initialize Git only after `plan.md`, because `plan.md` must be the first repository mutation.
Reason: `git status`, `git log`, and `git ls-files` all confirmed no valid repository.

## Progress log

### 2026-07-23

- Read the complete 2,582-line master specification.
- Confirmed Windows targeting as a hard constraint.
- Completed the required initial read-only directory and toolchain audit.
- Found an empty project and an Arch Linux execution host without the required Windows build, installer, host, or signing tools.
- Created this file as the first repository mutation.
- Initialized Git on `main`, added repository guidance and ignore rules.
- Added populated Phase 0/1 documentation, authoritative-source research, user flows, information architecture, wireframes, proposed Hybrid Signal design system, and provisional UI contract.
- Added a repeatable Windows development audit script.
- Added the CMake workspace, Windows-only JUCE UI Preview source, shared UI source, deterministic scenario catalogue, and RefTime calculation core.
- Verified `cmake --preset portable-tests`, `cmake --build --preset portable-tests`, and `ctest --preset portable-tests`: 1/1 test passed.
- Verified a fresh ephemeral OpenAI GPT Codex session loaded root `AGENTS.md`; it reproduced the first repository rule while explicitly prohibited from reading files. The global Stop hook then reported a failure, with no repository mutation.
- Reviewed and committed the initial Phase 0/1 foundation as `d816aab` (`chore: establish UI-first project foundation`).
- Recorded the user's JUCE 8 Starter/EULA acceptance and pinned official JUCE 8.0.9 commit `f72bad64d29715216226685810c5196bd0d79d77`.
- Expanded deterministic fixtures to cover every mandatory UI validation state and added tested compact/standard/wide breakpoint policy.
- The initial tag-as-branch submodule command failed after cloning an empty worktree. Changed method: fetched the verified tag directly, checked out its exact commit, registered the gitlink, and confirmed `git submodule status` reports JUCE 8.0.9.
- Fixed JUCE string-boundary compile errors found by the first compile-only check; the second check and a warnings-enabled repeat passed.
- Windows/JUCE compilation, rendered UI, DPI/accessibility, FL Studio, installer, and signing remain unverified.

## Unresolved questions

- What exact Windows machine/toolchain will be used for the mandatory Windows evidence?
- Is the current Linux workspace a temporary staging location that will later be opened by Codex on Windows?
- Which current JUCE licence path is acceptable for personal use and possible later distribution?
- Which UI typeface and icon set meet licence and legibility requirements?
- Which final visual direction will the user approve after the interactive preview?
- Which model/runtime combination will pass both licensing and controlled Stem Prep benchmarks?

## Next three concrete actions

1. On the intended Windows computer, run `tools/check-windows-dev.ps1` from Developer PowerShell and record the redacted result.
2. Run `git submodule update --init --recursive`, configure `windows-msvc`, and compile/test `EhvotaUIDemo` with MSVC.
3. Exercise every scenario, fix compile/render/layout/accessibility defects, then capture the required Windows size/DPI screenshots and update the UI acceptance evidence.

## Restart instructions

1. Read `AGENTS.md`, then read this entire plan.
2. Inspect `git status --short --branch` and do not overwrite unrelated work.
3. Read `docs/CURRENT_STATE.md`, `docs/DECISIONS.md`, and relevant active specifications.
4. Locate the first incomplete task in the current phase and verify its prerequisites.
5. If work is running on Linux, do not claim Windows, FL Studio, installer, VST3-validator, DPI, or signing evidence.
6. Update this plan with evidence after each milestone or changed decision.
7. Stop after three failed correction iterations on substantially the same approach; record evidence and change the method.
8. Do not begin Phase 2 production DSP until Phase 1 is complete and the user's explicit UI approval is recorded.
