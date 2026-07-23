# UI contract

Status: **Provisional Phase 1 contract. Not user-approved.**
The implemented JUCE preview and Windows validation may refine this document. Production DSP is blocked until the user approves the resulting interface direction.

## Global invariants

- Reference playback and project audio are never visually confusable.
- Audition-only controls say they are audition-only and cannot enter export.
- Live, captured, stale, missing, and invalid data have distinct labelled states.
- Proposed processing is visually and behaviorally distinct from active processing.
- Automatic changes are never applied without explicit review and acceptance.
- Every confidence display includes a level and inspectable reasons.
- Every user action that changes state supports undo where technically meaningful.
- Critical actions are reachable by keyboard and expose accessible names, roles, values, and states.
- Resizing causes layout reflow, not illegible uniform shrink.

## Shared shell contract

Required:

- Ehvota identity and product name.
- Preset browser, previous/next, save.
- Undo/redo.
- A/B state comparison.
- Bypass for processors; RefTap uses analysis enable/status rather than implying audio bypass.
- Help and interface scaling.
- Output meter where relevant.
- Link status where relevant.
- Exact value entry.
- Consistent warning/status area.

The shell must show Render Safe state in RefHub and any processor where monitoring-only paths exist.

## Preview journey replay

The standalone Phase 1 preview exposes a non-production review strip for each of the eight primary journeys:

- journey and step identity are always visible;
- Back and Next have labelled buttons and `Ctrl+Alt+Left` / `Ctrl+Alt+Right` equivalents;
- boundary actions disable instead of wrapping silently;
- selecting a primary scenario resets its journey to Step 1;
- required-state scenarios hide the journey strip rather than implying that they are complete workflows;
- `--scenario=<id> --step=<n>` selects a deterministic review state at launch.

This strip validates the interaction contract and documented sequence. It does not claim that audio, filesystem, model, plugin-link, clipboard, or host side effects are implemented during Phase 1.

## State model

Every data-bearing module exposes:

- `empty`: no configured source/data;
- `loading`: bounded operation with progress where known;
- `readyLive`: current data, not stable capture evidence;
- `capturing`: deliberate capture with usable duration;
- `readyCaptured`: stable, timestamped capture;
- `stale`: formerly valid data whose source stopped updating;
- `degraded`: partially usable with an explanation;
- `offline`: required local component unavailable;
- `error`: operation failed with recovery action;
- `renderSafe`: monitoring paths disabled for export.

Warnings never overwrite the last known valid value without marking it stale.

## Confirmation policy

No confirmation:

- navigation;
- audition mode changes;
- starting non-destructive validation;
- copying a timing value;
- selecting/focusing controls.

Explicit action but no modal:

- starting Capture Analysis;
- generating a proposal;
- loading a validated reference;
- changing a mapping that can be undone.

Confirmation required:

- overwriting existing Stem Prep output;
- deleting history/presets/reference metadata;
- applying a proposal affecting multiple plugin instances;
- clearing a queue with active work;
- changing a path that invalidates active references.

## RefHub contract

- The persistent comparison strip remains visible throughout relevant views.
- Overview presents no more than the most important Tone, Dynamics, Space, capture, mapping, and confidence summaries.
- Library never silently assigns an uncertain role.
- Mapping preserves disconnected assignments and labels them unavailable.
- Compare keeps listening controls adjacent to the measurements they contextualize.
- Capture Analysis is the primary basis for recommendations; Live Monitor is labelled as fast feedback.
- Proposals list every instance/parameter, current/proposed value, confidence, benefit, trade-off, and limit.
- History records source identity, section pair, mappings, capture time, measurements, and linked settings.

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

Initial Phase 1 acceptance targets, to be measured on the recorded Windows baseline:

- No continuous repaint timer while the UI is idle and no data is changing.
- Default live analyzer refresh: 30 Hz; optional High: at most 60 Hz; reduced-motion/high-CPU fallback: 15 Hz.
- Hidden, minimized, or fully obscured graphs pause nonessential repaint work.
- UI snapshot ingestion is bounded; a slow UI consumer may drop visual frames but never block the audio thread.
- Paint/layout work at default RefHub size: p95 at or below 8 ms and no unexplained frame above 50 ms during the deterministic replay.
- Continuous resize: p95 input-to-updated-frame at or below 50 ms, with no stall above 150 ms.
- Cold launch to interactive UI: at or below 1.5 seconds; warm launch: at or below 750 ms.
- UI Preview working set: target below 96 MiB at startup and below 160 MiB while cycling every fixture.
- Changing visual refresh quality must not change captured measurement values, proposal inputs, or audio processing.

These are budgets, not current claims. Record hardware, Windows build, display scale, graphics backend, build type, scenario, sample count, and measurement method with results.

## Acceptance checklist

- [ ] All eight journeys are replayable from deterministic scenarios.
  The eight validated step catalogues, navigation source, and deterministic launch option exist; rendered Windows exercise remains pending.
- [ ] Empty, loading, offline, error, degraded, stale, render, bridged/unsupported, high-CPU, missing-model, licence-error, cancellation, and repair states exist where relevant.
  Deterministic source fixtures now cover this state set, including dedicated bounded-loading and stale-data examples; rendered Windows review remains pending.
- [ ] Minimum/default/wide layouts work without overlap or unreachable controls.
- [ ] 100%, 125%, 150%, 175%, and 200% Windows scaling evidence exists.
- [ ] Mixed-DPI movement and live scale change are tested where hardware permits.
- [ ] Keyboard-only completion succeeds for primary actions.
- [ ] Focus, exact entry, mouse wheel, fine adjustment, reset, undo/redo, A/B, help, and tooltips are verified.
- [ ] Long names and Windows paths do not destroy layout.
  A dedicated long-label fixture and accessible full comparison-context label exist; compact/default/wide Windows rendering remains pending.
- [ ] High contrast, color-blind cues, reduced motion, and Narrator basics are reviewed.
- [ ] Analyzer rendering meets the established UI budget without audio-thread impact.
- [ ] Screenshots and review metadata exist under generated artefacts.
- [ ] Self-review findings are resolved or recorded.
- [ ] User explicitly approves the direction.
