# Coded UI review notes

Status: living review log for the UI approval gate. This file records visual evidence and corrections so the project does not repeat already-identified UI mistakes.

## 2026-08-13 — first Windows JUCE approval artifact

Evidence source: GitHub Actions `ehvota-coded-ui-approval` artifact generated from the Ableton-first feature branch.

### RefHub Active Comparison

What worked:
- Compare-first hierarchy is materially better than the rejected 14-item dashboard.
- Dual Mix/Reference timeline, selected Chorus region, large analyser, contextual range finding and collapsible inspector are present.
- Cyan/Violet/Amber semantics are aligned with the approved Precision Studio direction.
- Level Match state is implemented as monitoring-only view state with Auto/Section, Whole Track and Off modes.

Problems observed:
- Windows source encoding produced mojibake for en-dashes, arrows and separator characters. Root CMake now forces MSVC `/utf-8`; regenerated Windows screenshots must verify the fix before approval.
- The coded visual is still flatter and more utilitarian than the approved visual concept. It must remain an implementation baseline, not be mistaken for final polish.
- At 860×600 the analyser callout competes with the Insight region. Compact layout must favour the analyser and hide/collapse secondary detail instead of allowing visual collisions.
- The large-screen inspector is useful, but contributor rows and actions need stronger spacing and hierarchy.
- The Insight area should remain compact and proposal-oriented; it must not regress into a dashboard of equal-weight cards.

### Sources / Mapping

First artifact problem:
- The initial implementation showed two source lists but did not visually express the semantic mapping relationship strongly enough.

Correction:
- Reworked into Reference Sources → Mapping → Ableton/Ehvota Sources.
- Added explicit mapping state, a disconnected Vocal Bus example, relink-required state, confirmed/disconnected summary and visible composite definition.
- Track names remain hints, not semantic truth.

### RefTime

What worked:
- Exact milliseconds are the dominant readout.
- Host BPM, meter and sample rate are visible.
- Exact, rounded, samples and frequency values are separated.
- Delayed Only remains the default external-effect mode.

Problems observed:
- Footer explanatory text competed with copy buttons.
- Timing selectors did not have strong enough visible labels.
- External-effect workflow copy was too dense.

Correction:
- Reserved footer description space so copy actions do not overlap it.
- Added NOTE DIVISION, MODIFIER and OUTPUT MODE labels.
- Simplified tempo-change and external-effect explanations.
- Kept the signal path explicit: Source → RefTime → Third-party effect.

## CI learning

- Do not run the same feature-branch commit once for `push` and again for `pull_request`. Feature work now runs through the PR event; `push` is reserved for `main`.
- Superseded UI CI runs are cancelled using workflow concurrency so rapid UI iteration does not waste Windows build minutes.
- Screenshot generation is part of the Windows CI gate. A build is not visually reviewable until the PNG artifact is produced successfully.

## Approval rule

Do not begin production DSP because these notes improve. The actual regenerated coded screens must be shown to the user and explicitly approved at the UI gate first.
