# Ehvota’s Bundle — Project Source / Continuation Handoff

Last updated: 2026-08-31
Repository: `TropicalKenny/EhvotaPlugins`
Working branch: `feature/ableton-first-foundation`
Purpose: canonical continuation context for a new ChatGPT/Codex chat. Read this document, `plan.md`, `docs/UI_CONTRACT.md`, and `AGENTS.md` before changing code.

## 1. Product intent

Ehvota’s Bundle is a personal Windows audio-production toolset built primarily for Ableton Live. It is not being designed for sale or general distribution. The user has more than eight years of Ableton experience and chose Ableton as the primary DAW after initially considering FL Studio.

Primary goals:
- compare the user’s mix against one or more reference songs objectively without replacing musical judgement;
- analyse full mixes and optional reference stems;
- compare mapped Ableton buses/tracks against reference stems;
- provide transparent measurement plus reviewable Ehvota processing proposals;
- make timing, reference comparison, dynamics, spatial analysis and repeatable mixing decisions faster;
- keep the entire core usable without cloud AI;
- allow Codex/MCP integration later without giving an external agent unrestricted or realtime audio-thread control.

No third-party plugin control is required. Ehvota controls only its own processors and reads safe Ableton state through the bridge.

## 2. Agreed bundle

### RefHub
Master/reference hub placed on the master. It is the main comparison, analysis, mapping and proposal surface.

Required analysis:
- full mix vs reference;
- frequency/tone comparison;
- broad musical bands with solo/listen support: Low, Low-Mid, Mid, High-Mid, High and Air/highest region as implemented by the shared band definitions;
- dynamics/compression comparison;
- stereo/spatial comparison;
- Mid/Side information;
- component/stem diagnostics;
- named musical Section comparison;
- Blind comparison;
- automatic reference Level Match.

Reference folder workflow:
- one folder per reference song;
- authoritative full mix plus optional stems;
- recognised semantic components: Drums, Bass, Vocals, Music;
- stems are diagnostic inputs; the full mix remains authoritative;
- unknown files/roles must never be silently guessed;
- analysis scope explicitly supports excluding components, e.g. no Vocals, no Drums, Drums only, Music + Bass, etc.

### RefEQ
Individual-track/bus Ehvota EQ. Six-band implementation is underway. Required Stereo/Mid/Side processing and neutral/transparent state. RefHub may create bounded proposals for it, but never silently apply them.

### RefComp
Individual-track/bus compressor. Required Stereo Linked, Dual Mono and Mid/Side modes. Manual milliseconds remain available. Optional host-tempo-synchronised attack/release is supported. RefHub proposals must remain reviewable and stale-safe.

### RefTap
Transparent measurement/capture plugin for buses/tracks. It must not colour audio. It publishes measurement/state to RefHub and performs local bounded captures. Raw bus audio must not be sent through RefLink shared memory.

### RefTime
Timing utility designed to sit before reverbs/delays or other time-based processors. It converts host BPM/meter into musically useful exact millisecond values and can provide intentional timing/delay functionality. Exact ms readout is required so values can be copied into third-party effects such as Valhalla. Straight/dotted/triplet timing is required. It is not intended to automate third-party plugin parameters.

### RefColor
Dedicated analogue-character processor. Analogue colour must not be baked into transparent measurement processors. RefColor has an exact clean state and true bypass. Current foundation uses fixed 4× oversampled HQ processing, bounded saturation/asymmetry and DC management. Default character should be subtle and musical, but final quality must be validated by listening/tests before being called finished.

### RefTransient
Transient-shaping processor based on a shared transient detector so RefHub dynamics analysis and the processor do not use incompatible definitions of transient behaviour.

### Stem Prep
Standalone offline application, not a VST. It exists as a convenience/fallback stem separator for users without Codex even though the owner can use Codex separately for higher-quality preparation. It must work offline, preserve originals, expose honest validation/confidence, support cancellation safely and provide CPU fallback when GPU/model execution is unavailable. Do not claim source separation is non-AI if the chosen separation model is ML-based; document the actual model/runtime honestly when implemented.

## 3. AI policy

The core plugin does NOT require AI for automatic Level Match, EQ/dynamics/spatial analysis, spectrum comparison, Mid/Side analysis, timing calculations, reference comparison or proposal generation. Those should use deterministic DSP/measurement/rules.

Codex can be used outside the plugin for tasks such as preparing reference stems or reasoning over exported/captured information. Future MCP integration is optional and must use safe structured tools.

No external AI/client may bypass Ehvota’s proposal, validation, permission, preview, stale-value and undo boundaries.

## 4. Approved RefHub UI direction

`docs/UI_CONTRACT.md` is the detailed visual contract.

The approved foundation is the dark Precision Studio direction:
- near-black/graphite studio canvas;
- cyan = Your Mix;
- violet/purple = Reference;
- amber/orange = actionable difference / selected problem / proposed intervention;
- green mainly for healthy/connected/success states;
- large timeline and analyser dominate the interface;
- Compare-first workflow rather than a generic SaaS dashboard;
- collapsible contextual inspector;
- compact Ehvota Insight region;
- Compare / Sources / Process / History / Settings navigation;
- real measurements must replace fixture claims whenever evidence exists;
- proposed processing is visually distinct from active processing.

Do not restart UI ideation from scratch. Refine the approved direction and show coded JUCE previews for review.

## 5. Automatic Level Match contract

Level Match is a core RefHub feature.

Modes:
1. `Auto / Section` — default/recommended. Match the selected paired Mix/Reference Section.
2. `Whole Track` — use validated whole-track evidence.
3. `Off` — no monitoring compensation.

Rules:
- monitoring/audition gain only; never modify source/reference files, project audio, captured evidence or exports;
- use perceptual loudness/BS.1770-style measurement rather than peak normalisation;
- display Mix loudness, Reference loudness and applied monitor trim;
- Blind Compare defaults to matched evidence so loudness does not reveal/bias the source;
- filtered-band audition needs controlled audition compensation distinct from processing;
- if Section evidence is unavailable, fallback to Whole Track only when valid and label the fallback; never invent a value;
- equal LUFS is an objective aid, not a claim of identical perceived loudness.

## 6. Ableton integration architecture

Primary target: Ableton Live 12 Suite on Windows.

Ehvota processors remain VST3. Ableton/Max for Live integration is an external host-state bridge, not the DSP path.

The bridge is read-only first. Live API/Live Object Model work is main-thread/deferred and must never enter realtime DSP.

Important identity decision:
- do NOT persist JavaScript `LiveAPI.id` as durable mapping identity; it is runtime-dynamic;
- RefTap owns a persistent Ehvota UUID stored/restored in VST3 state;
- RefHub stores semantic role → RefTap persistent UUID mappings;
- Ableton track/bus name is a human-readable hint only;
- duplicate live RefTap UUIDs are ambiguous and must not auto-map.

## 7. RefLink architecture and safety

The original in-process/static registry idea was rejected because separate VST3 DLLs do not share the same C++ static storage.

Current production direction uses versioned Windows session-local shared memory for cross-DLL RefLink communication.

Rules:
- no sockets, JSON, OS shared-memory calls, locks or Ableton APIs in `processBlock()`;
- audio callback updates local atomics/preallocated buffers only;
- message-thread timers publish heartbeats/metrics/parameter snapshots and consume approved commands;
- fixed instance slots, generations, heartbeat freshness and stale-instance recovery;
- persistent Ehvota UUID is separate from runtime slot/generation identity;
- raw captured bus audio never travels through RefLink;
- RefTap analyses captured audio locally and publishes bounded summary measurements.

Cross-plugin writes are intentionally narrow:
- no generic unrestricted `setParameter` API;
- fixed known Ehvota parameter IDs and bounded values;
- proposal carries target generation;
- proposal carries expected current parameter value;
- target rejects stale proposal if generation/current value no longer matches;
- only an explicitly approved proposal may enter the mailbox;
- target applies APVTS changes on the message thread, not the audio thread.

## 8. Reference/component capture

Master and component capture must be sample-accurate enough for valid Section comparison.

Current architecture direction:
- capture storage is preallocated outside the audio callback;
- realtime thread only copies into fixed storage;
- analysis happens after capture completion off the audio thread;
- mapped RefTap buses are scheduled to one common future host-sample boundary for manual component capture;
- saved Sections require an exact absolute host-sample start, not a shifted `+1 second` substitute;
- if there is insufficient lead time for an exact Section component capture, reject it rather than move the Section;
- missed starts and transport discontinuities invalidate evidence rather than stitching unrelated audio;
- completed RefTap evidence must not be overwritten before the timer/worker publishes it.

Named Sections have independent Mix and Reference ranges. Editing a Section after capture must not silently reinterpret old evidence; evidence binds to the Section definition used when armed.

## 9. Whole-track measurement

Whole-track Level Match is being implemented as streaming BS.1770-style measurement rather than retaining an entire song in RAM.

Requirements:
- master input is measured unmodified;
- reference file is streamed in chunks on a worker thread;
- long references are not loaded wholesale into RAM;
- mono references are handled explicitly for loudness measurement;
- whole-track evidence has validity/staleness state separate from Section evidence;
- whole-track evidence is runtime measurement evidence, not blindly serialized as permanently valid project truth.

## 10. Current repository state at handoff

Branch source-of-truth commit at the last confirmed branch update in this work session:
- `a5c7e7507373393102172da80eb673a33bd883ee`
- message: `Add paired sections and streaming whole-track loudness`

Immediately before that, Windows CI had become fully green after fixing:
- missing JUCE `AudioTransportSource` include;
- shadowed `playHead` warning;
- JUCE VST2→VST3 replacement guard by explicitly setting `JUCE_VST3_CAN_REPLACE_VST2=0` for this unreleased VST3-only bundle.

At the time this handoff is written, the run for `a5c7e750...` had portable tests green and Windows full build in progress. The user then explicitly requested that unnecessary GitHub Actions stop.

### Important off-branch prepared work

These objects were intentionally prepared without moving the branch so CI would not be retriggered repeatedly. A future agent MUST verify they still exist and inspect/rebase them before use; do not blindly force them onto a changed branch.

- Commit `199d35d38e6103690f5f4707d3cca99931add9d0`
  - host-scheduled capture foundation candidate.
- Tree `633c05cde009849d3018f74ba2d669fa91be1f90`
  - hardened RefTap timing migration candidate, including completed-evidence failure cleanup.
- Commit `348fe895ded0206d83e80457653f7ceb67b7ca21`
  - frozen RefTap timing consolidation candidate built from the reviewed tree.
- Tree `ef8a7cfac6a17c40c02f26c7b4c378ee232ddb46`
  - rebased RefHub processor candidate.
- Commit `b10993d26658f7b610514df42defaa572067743a`
  - frozen RefHub processor orchestration candidate.

Earlier temporary tree/commit identifiers mentioned in chat may have been superseded by the frozen candidates above. Prefer the latest reviewed/frozen candidate and verify parentage/content before updating refs.

## 11. GitHub Actions policy — IMPORTANT

GitHub Actions ARE allowed, but only when they provide real value at an intentional push/integration gate.

Do:
- batch coherent changes locally/off-branch/in Git objects first;
- push/update the working branch when a meaningful implementation slice is ready;
- let Actions validate that intentional push;
- use Windows/MSVC CI for genuine integration gates because the product target is Windows/VST3/Ableton;
- investigate failures before the next intentional push.

Do NOT:
- trigger Actions for every tiny file edit;
- repeatedly push half-built intermediate states just to see whether they compile;
- cancel a useful Windows build by immediately pushing another trivial commit;
- rerun successful workflows without a reason;
- use Actions as a substitute for static review or portable tests that can be reasoned/tested before push;
- create unnecessary CI churn/cost.

Working rule: **prepare/batch first → intentional push → one useful CI gate → fix findings in a coherent batch → intentional next push.**

## 12. Installation/signing decision

Bundle name: **Ehvota’s Bundle**.

Windows only. Final delivery should be one installer `.exe` that installs the required VST3s/apps/resources.

Signing decision: Route A from the earlier planning discussion (personal/self-managed development signing path rather than buying a public commercial distribution certificate at this stage). The project is personal-use software. Do not turn code-signing into a commercial-distribution requirement unless the user changes the goal.

Installer/signing work is not yet considered finished. Final validation must happen on the user’s actual Windows machine.

## 13. Quality rules

- Transparent processors must have a true neutral/identity state.
- RefTap must remain bit-transparent.
- Measurement refresh quality must never alter captured measurement values or DSP.
- No realtime allocation/locking/file/network/bridge work in audio callbacks.
- Never describe unfinished DSP as mastering-quality merely because it compiles.
- Analogue enhancement belongs in deliberate processors such as RefColor, not hidden inside measurement tools.
- All automatic corrective behaviour is proposal-first and requires explicit approval.
- Undo must exist where technically meaningful.
- Stale/degraded/missing/invalid evidence must be labelled distinctly.
- Confidence must have inspectable reasons; do not present taste as measurement fact.
- Do not break already-working functionality while adding features.

## 14. Immediate continuation order

A new chat/agent should proceed in this order:

1. Read this file, `plan.md`, `docs/UI_CONTRACT.md`, `AGENTS.md`, and current branch head.
2. Check whether the last Actions run for `a5c7e750...` eventually succeeded/failed, but do not rerun it merely for curiosity.
3. Inspect parentage/diff of prepared commit `199d35d...` and the frozen candidates `348fe895...` and `b10993d...` against the current branch. Rebase/recreate only if branch movement requires it.
4. Land the shared host-scheduled capture foundation as one coherent integration slice.
5. Validate portable tests before pushing; then perform one intentional branch push and allow one useful Windows/MSVC Actions gate.
6. Land RefTap timing consolidation only after the scheduler layer is sound.
7. Land RefHub orchestration for saved Sections + Whole-Track evidence after those lower layers are green.
8. Wire the approved RefHub editor to real saved Section CRUD/selection and real Level Match state. Do not redesign the approved UI.
9. Ensure saved Section capture drives both master and mapped component buses at the correct absolute Section timing.
10. Continue deeper RefHub proposal generation from real evidence/live target snapshots, then finish remaining processor quality work.
11. Complete Stem Prep offline app and honest model/runtime/validation flow.
12. Build the single Windows installer, signing path and clean-machine validation.
13. Perform final tests in the user’s actual Ableton Live 12 Suite environment on Windows; this cannot be honestly certified from GitHub CI alone.

## 15. What requires user input later

Do not stop for ordinary engineering choices that are already covered by this source document.

User input is required for:
- visual approval when a materially changed coded UI preview needs judgement;
- subjective listening decisions/analogue character quality;
- testing the actual VST3s inside the user’s Ableton Live installation;
- Max for Live `.amxd` final host testing;
- Windows signing certificate installation/use where credentials are local to the user;
- final installer test on the user’s machine;
- any genuine product decision not already specified here.

Otherwise continue building without repeatedly asking for confirmation.

## 16. Continuation instruction for a new chat

Use this document as the source of truth for intent and constraints, but verify current code before acting. Do not restart planning from zero. Do not discard already-working architecture. Do not overuse GitHub Actions. Continue from the current branch and the prepared integration candidates, keep the learning loop/documentation current, and only claim features/tests that have actually been implemented and verified.
