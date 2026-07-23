# Information architecture

Status: Proposed for Phase 1 prototype.

## Suite model

```text
Ehvota's Bundle
├── RefHub
│   ├── persistent comparison strip
│   ├── primary workspace
│   └── contextual inspector
├── RefTap
├── RefEQ
├── RefComp
├── RefTime
├── RefColor
└── Stem Prep
```

Every surface uses the shared shell, terminology, semantic states, and help model. RefHub has the deepest navigation; processor plugins favor one primary workspace plus contextual/advanced controls.

## RefHub

### Persistent comparison strip

Always visible when relevant:

- primary reference identity and validation state;
- Mix/Reference segmented selector;
- loudness match with “audition only” semantics;
- play/audition state;
- loop and selected live/reference sections;
- Capture;
- Blind compare;
- undo/redo;
- Render Safe status;
- link health;
- highest-priority warning.

Reference playback must use a distinct transport identity and cannot resemble project playback.

### Primary navigation

Order follows frequency of use and workflow:

1. Overview
2. Compare
3. Library
4. Mapping
5. Sections
6. Bands
7. Dynamics
8. Space
9. Components
10. Links
11. Proposals
12. History
13. Timing
14. Settings

At compact width, navigation becomes a labelled drawer. Current view remains visible as text; essential views are not represented by unexplained icons.

### Workspace and inspector

- Workspace owns the task's principal visualization or list.
- Inspector appears for the selected source, band, section, capture, linked instance, or proposal.
- At wide width, the inspector is a right column.
- At standard width, it is a collapsible right column.
- At compact width, it is a labelled bottom sheet/drawer that never obscures critical confirmation controls.

### Overview priorities

Overview contains:

1. current comparison and section;
2. capture readiness/status;
3. mapping completeness;
4. three prioritized difference cards: Tone, Dynamics, Space;
5. confidence summary and one next action.

It does not duplicate every detailed meter.

## RefTap

Compact single-surface editor:

- identity/name;
- role;
- placement (pre/post description);
- link/capture status;
- minimal input activity;
- diagnostics/help.

No processing controls.

## RefEQ

- Shared shell.
- Large direct-manipulation EQ/analyser workspace.
- Selection-driven band controls.
- Bottom inspector for exact values and dynamics.
- Advanced drawer for phase, sidechain, analyser, and quality.
- Output/match meters remain visible.

## RefComp

- Shared shell.
- Immediate compression controls and gain-reduction history.
- Context panel for detector/sidechain/timing.
- Advanced drawer for topology, Mid/Side, lookahead, oversampling, safety.
- Current/proposed and matched bypass share suite semantics.

## RefTime

- Shared shell in a smaller editor.
- Exact time readout is dominant.
- Timing source/division controls are immediate.
- Copy actions and output mode are explicit.
- Timing table and transition mode use progressive disclosure.
- Host value, manual override, samples, and technical latency are visually distinct.

## RefColor

- Shared shell.
- Clean/character mode and Drive/Character/Mix/Output are immediate.
- Calibration, oversampling, bias, tone, dynamics, and channel behavior are contextual/advanced.
- Clean mode is first-class, not a hidden bypass surrogate.

## Stem Prep

Standalone layout:

- top: References folder, model/device status, privacy/offline badge;
- left/center: queue and per-job progress;
- right inspector: input/output, device, model, validation, diagnostics;
- bottom: disk/model readiness, Start/Pause/Cancel, open-folder action.

## Responsive classes

- Compact: narrower than 760 logical px; one primary pane, drawer navigation/inspector.
- Standard: 760–1099 logical px; primary pane plus optional inspector.
- Wide: 1100 logical px and above; persistent navigation, workspace, inspector where useful.

These are initial prototype thresholds. Windows evidence may refine them. Logical sizes must reflow; do not scale a fixed bitmap.

## Low-fidelity wireframes

### RefHub wide

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│ EHVOTA / REFHUB  [Reference: Ayo Nights ▾] [MIX|REFERENCE] [MATCH] [CAPTURE]│
│ Section: Chorus 2 ↔ Main Chorus    [BLIND] [UNDO][REDO]  Links ●  Render Safe│
├──────────────┬───────────────────────────────────────────┬──────────────────┤
│ Overview     │                                           │ Context          │
│ Compare      │             active workspace              │ selected source  │
│ Library      │                                           │ confidence       │
│ Mapping      │                                           │ controls         │
│ Sections     │                                           │ explanation      │
│ …            │                                           │                  │
├──────────────┴───────────────────────────────────────────┴──────────────────┤
│ Status: Live Monitor · Reference ready · Audition path excluded from render │
└─────────────────────────────────────────────────────────────────────────────┘
```

### RefHub compact

```text
┌──────────────────────────────────────────┐
│ ☰  REFHUB   Ayo Nights ▾      Links ●   │
│ [ MIX | REFERENCE ] [MATCH] [CAPTURE]   │
│ Chorus 2 ↔ Main Chorus        Render Safe│
├──────────────────────────────────────────┤
│                                          │
│            active workspace              │
│                                          │
├──────────────────────────────────────────┤
│ Compare                         Context ↑│
└──────────────────────────────────────────┘
```

### RefTime

```text
┌──────────────────────────────────────────────────┐
│ EHVOTA / REFTIME     Host 120 BPM · 4/4 · 48 kHz│
├──────────────────────────────────────────────────┤
│ Source [HOST]   Division [1/32] [DOTTED]         │
│                                                  │
│                  93.750 ms                       │
│             rounded 94 ms · 4,500 samples        │
│                                                  │
│ [COPY EXACT MS] [COPY NUMBER] [COPY SAMPLES]     │
│ Output [DELAYED ONLY ▾]  Transition [CROSSFADE ▾]│
├──────────────────────────────────────────────────┤
│ Intentional delay 93.750 ms · Reported latency 0 │
└──────────────────────────────────────────────────┘
```

### Stem Prep

```text
┌──────────────────────────────────────────────────────────────────┐
│ EHVOTA / STEM PREP   References: …\Ehvota's Bundle\References   │
│ Offline ●   Model ready ●   Device Automatic (CPU fallback)      │
├────────────────────────────────────────────┬─────────────────────┤
│ QUEUE                                      │ JOB INSPECTOR       │
│ 1  Ayo Nights     Separating  62%          │ input / destination │
│ 2  River Glass    Waiting                  │ model / device      │
│ 3  Live Room      Needs attention          │ output policy       │
│                                            │ validation          │
├────────────────────────────────────────────┴─────────────────────┤
│ Disk ready ●    [ADD FILES] [START] [CANCEL SAFELY] [OPEN FOLDER]│
└──────────────────────────────────────────────────────────────────┘
```
