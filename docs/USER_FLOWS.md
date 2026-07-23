# User flows

Status: Phase 1 prototype contracts. Mocked data must make every success, warning, and recovery state replayable.

## Shared interaction rules

- Every workflow begins with explicit source, comparison, connection, and render-safety context.
- Long-running work exposes progress, safe cancellation, and recoverable state.
- Destructive or processing changes require confirmation or an explicit proposal acceptance.
- Every automatic result names its confidence and reasons.
- Keyboard operation follows the same logical sequence as the visual flow.
- Success never hides missing sources, stale measurements, or degraded analysis.

## J1 — Add a new reference

Entry: RefHub → Library → Add Reference.

1. Show the active References folder and an explicit **Choose folder** or **Scan** action.
2. Display discovered direct child folders as Pending, Validating, Ready, Needs mapping, or Invalid.
3. Open “Ayo Nights” with `full_mix.wav` plus four stems.
4. Show sample rate, duration, channel/alignment checks, role assignments, and separation-report confidence.
5. Require the user to resolve an uncertain filename role rather than guessing.
6. Enable **Load as primary reference** only when full-mix validation succeeds.
7. On load, update the persistent strip without starting playback.

Recovery states: missing full mix, unequal stem length, moved library, unreadable file, uncertain role, low stem confidence.

## J2 — Compare full mixes

Entry: RefHub → Compare with a primary reference loaded.

1. Select “Chorus 2” in the mix section lane and “Main Chorus” in the reference lane.
2. Enable loudness match and show its audition-only status.
3. Switch Mix/Reference from the persistent strip.
4. Present tone, dynamics, and width as separate synchronized panels.
5. Focus the Low-mid band and hear only the clearly labelled comparison mode.
6. Start blind A/B; hide identity while preserving a safe exit and trial counter.
7. Reveal the result and restore explicit identities.

Recovery states: uncaptured mix section, reference relink required, reference audition unavailable during render, insufficient active duration.

## J3 — Map stems to buses

Entry: RefHub → Mapping.

1. Select `drums.wav` from reference sources.
2. Map it to connected “Drum Bus · post” and show both identities.
3. Map bass, vocals, and music.
4. Build “Rhythm” from drums + bass, showing member sources.
5. Save the mapping with a distinct name.
6. Disconnect “Music Bus” in the deterministic fixture.
7. Preserve the mapping but mark the live endpoint disconnected and proposals unavailable.

Recovery states: duplicate role, disconnected target, stale RefTap, ambiguous pre/post placement, missing reference stem.

## J4 — Exclude components

Entry: Compare → Components.

1. Choose Instrumental Comparison.
2. Exclude vocals on live and reference sides with a paired, visible rule.
3. Switch to Drums Only.
4. Switch to No Drums.
5. For every mode, show a complete sentence describing included and excluded sources.
6. Show confidence degradation if required stems are low quality or missing.

Recovery states: one-sided exclusion, missing source, invalid composite, low reconstruction confidence.

## J5 — Capture a chorus

Entry: Compare/Sections → selected matched chorus pair.

1. Confirm live “Chorus 2” and reference “Main Chorus.”
2. Start **Capture analysis**.
3. Show armed, capturing, usable duration, progress, and cancel states.
4. Complete with a stable capture timestamp and measurement confidence.
5. Explain confidence reasons.
6. Save to History with mapping/reference/section identity.

Recovery states: transport never reaches region, loop discontinuity, too little active audio, source disconnect, cancellation, capture failure.

## J6 — Review and apply a proposal

Entry: RefHub → Proposals → Drum Bus.

1. Show measured tonal and dynamic differences from a named capture.
2. Generate a bounded RefEQ + RefComp proposal.
3. List every instance and changed parameter with current/proposed values.
4. Show confidence, reasons, expected benefit, trade-offs, and safety limits.
5. Preview Current/Proposed with audition-only loudness matching.
6. Run blind compare if desired.
7. Accept or reject explicitly.
8. On acceptance, record one undoable transaction; then demonstrate Undo.

Recovery states: plugin disconnects, remote proposal permission disabled, stale capture, low confidence (advice only), parameter outside protected range.

## J7 — RefTime before a third-party reverb

Entry: RefTime inserted before a third-party reverb on a send.

1. Show Host tempo 120 BPM, 4/4, and sample rate 48 kHz.
2. Select 1/32 dotted.
3. Prominently show 93.750 ms exact, 94 ms rounded, and 4,500 samples.
4. Copy Exact MS and confirm exactly what entered the clipboard.
5. Select Delayed Only and explain it as the external pre-delay workflow.
6. Change mocked host tempo to 128 BPM.
7. Show the old/new values and a click-free transition state.
8. Keep reported technical latency separate and at zero for the intentional delay in this mock.

Recovery states: host tempo unavailable, invalid manual BPM, sample rate change, offline render, third-party effect still has internal pre-delay.

## J8 — Prepare stems offline

Entry: Ehvota Stem Prep standalone app.

1. Drop `Ayo Nights.wav` into the queue.
2. Preview a new reference folder without modifying the source.
3. Select Balanced 4-Stem and Automatic device.
4. Show disk-space and model-ready checks.
5. Start the queue through Preparing, Separating, Validating, and Finalizing.
6. Cancel safely during a deterministic second job; retain no completed-looking partial output.
7. Resume/start another job.
8. Complete and show hashes, equal duration/alignment, reconstruction diagnostics, and confidence wording.
9. Open the folder.
10. Switch to RefHub Library fixture and show the new Ready reference.

Recovery states: missing model, licence error, GPU fallback to CPU, insufficient disk, temporary-file cleanup, overwrite confirmation, invalid output.

## Prototype navigation

The preview includes a scenario selector separate from the production UI. It can jump to any required state deterministically for review and screenshot capture. This selector must not leak into plugin interfaces or product state.

Each of the eight primary scenario fixtures also exposes a guided journey strip. **Back** and **Next** replay the documented steps without audio processing or external state changes; `Ctrl+Alt+Left` and `Ctrl+Alt+Right` provide the keyboard equivalent. Step labels and instructions are part of a dependency-free, validated view model rather than UI-only text.

For deterministic launch and capture, the preview accepts a one-based `--step=<n>` option after a primary journey scenario is selected. Required-state fixtures intentionally reject `--step` because they are evidence states, not primary journeys.

The journey strip is a review aid, not proof that downstream audio, file, host, or model behaviour exists. Windows interaction review must still verify that every step is readable and operable before the journeys are marked demonstrated.
