# UI contract

Status: **RefHub visual foundation approved by user on 2026-08-12.** Production DSP remains blocked until the coded JUCE preview reproduces the approved direction and passes the UI approval gate for the remaining required screens.

## Approved RefHub visual foundation

The approved RefHub direction is the Precision Studio concept reviewed on 2026-08-12. It is now the visual contract for implementation, not an exploratory mockup.

### Visual hierarchy

- Near-black/graphite studio canvas with minimal chrome and restrained borders.
- Cyan identifies **Your Mix**.
- Violet/purple identifies **Reference**.
- Warm amber/orange identifies an actionable difference, selected problem range, or proposed intervention.
- Green is reserved primarily for healthy/connected/success states.
- The musical timeline and analysis canvas dominate the interface.
- Findings appear in context on the analysis surface and in a compact Ehvota Insight area; the UI must not regress into a generic SaaS/monitoring dashboard.
- Secondary information uses progressive disclosure and a collapsible contextual inspector.

### Approved primary structure

Top header:
- Ehvota / RefHub identity.
- Reference selector.
- Section selector.
- MIX / REF A-B control.
- Ableton Link/bridge health.
- Blind.
- Capture.
- Undo / Redo.
- Settings and compact output/activity indication.

Left rail:
- Compare.
- Sources.
- Process.
- History.
- Settings may remain a compact utility control rather than another large navigation destination.

Compare canvas:
- dual waveform timeline for Your Mix and Reference;
- named musical sections such as Intro, Verse, Chorus, Outro;
- selected section range and duration;
- transport / A-B audition controls;
- loop/capture status;
- level-match state;
- Tone / Dynamics / Space / Components lenses over the same comparison workspace;
- large primary analyser;
- contextual selected-range finding;
- collapsible inspector;
- compact Ehvota Insight / suggested-focus / quick-action region.

The approved concept contains a selected low-mid example around 220–410 Hz with Your Mix, Reference, difference, confidence, contributors, listening actions, comparison action, and Create Proposal. These values are illustrative fixture data, not hard-coded DSP claims.

## Automatic reference level matching — REQUIRED

Automatic perceptual level matching is a core RefHub feature and must not be treated as optional polish.

### Behaviour

- Level matching changes **monitoring/audition gain only**. It never modifies the source reference file, project audio, exported audio, captured evidence, or underlying measurements.
- RefHub measures the relevant captured Mix and Reference material using perceptual loudness suitable for reference comparison rather than peak normalization.
- The UI must expose the measured loudness values and the monitoring compensation/trim used for the comparison.
- Switching MIX ↔ REF while Level Match is enabled must preserve a fair perceived-loudness comparison.
- Turning Level Match off must audition the uncompensated original relative levels.
- Blind Compare defaults to Level Match enabled so loudness does not trivially reveal or bias the source. A deliberate advanced override may exist, but the default blind workflow is matched.
- Band/range audition must use controlled audition gain so a filtered range is not judged merely as worse because filtering made it quieter. This audition compensation remains monitoring-only and must be clearly distinct from processing.

### Modes

RefHub must support these explicit modes:

1. **Auto / Section** — default and recommended. Match the currently paired captured Mix and Reference sections (for example Chorus 1 ↔ Chorus 1).
2. **Whole Track** — use the validated whole-track Mix/Reference loudness relationship where sufficient evidence exists.
3. **Off** — no monitoring compensation; preserve the actual loudness difference.

If the selected section lacks sufficient stable captured evidence, Auto / Section must not silently invent a value. It must fall back only according to a documented rule (for example Whole Track when valid) and label that fallback, or report that section matching is unavailable.

### UI requirements

The Level Match surface must make it possible to inspect:
- current mode;
- on/off state;
- Mix loudness;
- Reference loudness;
- applied monitoring trim/compensation;
- whether values are section-based, whole-track, live/temporary, captured/stable, or fallback;
- stale/degraded/unavailable state.

Example presentation (illustrative only):

`SECTION MATCH · ON`
`Mix -14.8 LUFS · Ref -10.9 LUFS · Ref monitor trim -3.9 dB`

Do not imply that equal LUFS guarantees identical perceived loudness. The feature is an objective perceptual-level aid and the final comparison remains auditory.

## Global invariants

- Reference playback and project audio are never visually confusable.
- Audition-only controls say they are audition-only and cannot enter export.
- Live, captured, stale, missing, and invalid data have distinct labelled states.
- Proposed processing is visually and behaviorally distinct from active processing.
- Automatic processing changes are never applied without explicit review and acceptance.
- Every confidence display includes a level and inspectable reasons.
- Every user action that changes state supports undo where technically meaningful.
- Critical actions are reachable by keyboard and expose accessible names, roles, values, and states.
- Resizing causes layout reflow, not illegible uniform shrink.
- Render Safe must disable/bypass monitoring-only audition paths that must not enter an export.

## Ableton-first integration boundary

Ableton Live 12 Suite on Windows is the primary host target. Ehvota processors remain VST3. Max for Live / Live Object Model integration is an external host-state bridge, not the DSP implementation.

RefHub may display Ableton connection health and use validated host context, but the visual identity remains Ehvota's own. Ableton integration should be informative and unobtrusive.

The future Codex/MCP bridge may inspect Ehvota and Ableton state, but externally initiated processing changes must pass through the same proposal, validation, permission, preview and undo boundaries used by RefHub. No external client may bypass those controls.

## Contextual inspector

The inspector is contextual and collapsible. It may show Range, Component, or Source details. When collapsed, the analyser expands into the reclaimed space. It must not reserve a large empty permanent column.

For a selected range it may show:
- exact frequency range;
- Mix value;
- Reference value;
- average difference;
- confidence;
- contributors such as Drums, Bass, Music, Vocals;
- Listen Range;
- Compare A/B;
- Create Proposal.

## Ehvota Insight and proposals

The Insight region interprets evidence without pretending that measurements are taste.

- Prefer one prioritised finding over a wall of equal-weight cards.
- Suggested processing remains a **proposal** until explicitly accepted.
- Do not use language or controls that imply an automatic fix has already been applied.
- A quick action may create a gentle proposal, but the production wording should prefer `Create Proposal` / `Preview Proposal` over an ambiguous `Apply Fix`.
- Proposal details must identify every affected Ehvota instance/parameter, current/proposed value, confidence, expected benefit, trade-offs and limits.

## Sources contract

Sources contains Library, Mapping, Sections and Validation. Detailed reference/stem validation belongs here rather than dominating the normal comparison view.

Mapping must support reference stems and Ableton/Ehvota live sources, composites, disconnected assignments, saved mappings, and explicit validation. Uncertain roles are never silently assigned.

## Process contract

Process contains Links, Proposals, Signal Chain and Learn. It exposes connected RefTap/RefEQ/RefComp/future Ehvota processors, health and roles. External/Codex proposals use the same review path as local RefHub proposals.

## RefTime contract

- Exact milliseconds are one of the editor's largest text elements.
- Host BPM/meter/sample rate and timing source are always visible.
- Exact, rounded, sample, and optional frequency values are labelled independently.
- Clipboard actions name their format and confirm copied content without a blocking modal.
- Delayed Only is the default external pre-delay mode.
- Intentional delay and reported technical latency appear as separate values.
- Tempo transitions identify crossfade, instant, or tape glide.

## Stem Prep contract

- It is a standalone offline application, never a VST.
- Original inputs are preserved.
- Queue state distinguishes preparing, separating, validating, finalizing, complete, cancelled, and failed.
- Cancellation never leaves temporary output looking complete.
- GPU failure exposes CPU fallback.
- Output validation states alignment, duration, sample rate, hashes, reconstruction diagnostics, and honest confidence wording.
- Missing model or licence problems explain the local repair action.

## Initial size contract

| Surface | Default | Minimum |
|---|---:|---:|
| RefHub | 1180 × 760 | 860 × 600 |
| RefEQ / RefComp | 920 × 620 | prototype to establish |
| RefTime | 720 × 500 | prototype to establish |
| RefTap | 440 × 240 | compact fixed minimum; resizing to verify |
| RefColor | 760 × 540 | prototype to establish |
| Stem Prep | 1040 × 720 | prototype to establish |

Values are logical pixels and may be refined from Windows evidence. All major plugin editors remain continuously resizable.

## UI performance budgets

- No continuous repaint timer while the UI is idle and no data is changing.
- Default live analyzer refresh: 30 Hz; optional High: at most 60 Hz; reduced-motion/high-CPU fallback: 15 Hz.
- Hidden, minimized, or fully obscured graphs pause nonessential repaint work.
- UI snapshot ingestion is bounded; a slow UI consumer may drop visual frames but never block the audio thread.
- Paint/layout work at default RefHub size: p95 at or below 8 ms and no unexplained frame above 50 ms during deterministic replay.
- Continuous resize: p95 input-to-updated-frame at or below 50 ms, with no stall above 150 ms.
- Cold launch to interactive UI: at or below 1.5 seconds; warm launch: at or below 750 ms.
- Changing visual refresh quality must not change captured measurement values, proposal inputs, or audio processing.

These are budgets, not current claims. Record hardware, Windows build, display scale, graphics backend, build type, scenario, sample count, and measurement method with results.

## Approval gate

Approved:
- [x] RefHub overall visual direction / Precision Studio foundation.
- [x] Ableton-first host direction.
- [x] Automatic reference Level Match as a core feature with Auto/Section, Whole Track and Off modes.

Still required before production DSP:
- [ ] Coded JUCE RefHub Active Comparison preview matches the approved visual contract closely enough for user review.
- [ ] RefHub Sources / Mapping approval screen.
- [ ] RefTime approval screen.
- [ ] Minimum/default/wide layouts without overlap or unreachable controls.
- [ ] 100%, 125%, 150%, 175%, and 200% Windows scaling evidence.
- [ ] Keyboard-only primary actions and basic accessibility review.
- [ ] UTF-8 fixture and long-name/path review.
- [ ] User explicitly approves the coded UI gate.
