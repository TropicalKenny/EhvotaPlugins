# Design system

Status: Hybrid Signal proposal for prototype; token values require rendered review and Windows contrast validation.

## Principles

1. Observation and intervention are visually distinct.
2. Listening state is clearer than graph decoration.
3. Exact values remain readable at ordinary laptop sizes.
4. Dense information appears progressively.
5. Color reinforces labels, shapes, patterns, and position; it never carries meaning alone.
6. Motion communicates changing data or short state transitions, never ambience.

## Typography

Initial Windows system stack:

```text
Segoe UI Variable
Segoe UI
Arial
sans-serif
```

No font is downloaded or redistributed. Windows 10/11 testing must confirm glyph coverage, metrics, decimal clarity, and distinctions between `0/O`, `1/I/l`. Meter and timing styles request tabular numerals through the JUCE font-feature API when supported; visual tests must confirm equal-width digits.

| Token | Logical size | Weight | Use |
|---|---:|---:|---|
| `type.display` | 40 | 600 | RefTime exact value only |
| `type.metricLg` | 24 | 600 | primary meter values |
| `type.title` | 18 | 600 | view/plugin title |
| `type.heading` | 15 | 600 | panel heading |
| `type.body` | 14 | 400 | ordinary labels and text |
| `type.control` | 13 | 600 | buttons and values |
| `type.caption` | 12 | 400 | secondary metadata |

Do not use text below 11 logical px. Uppercase is limited to short product identifiers or compact state tags, never paragraphs or the primary labelling system.

## Spacing and sizing

Base spacing unit: 4 logical px.

```text
space.1 = 4
space.2 = 8
space.3 = 12
space.4 = 16
space.5 = 20
space.6 = 24
space.8 = 32
space.10 = 40
```

- Minimum ordinary interactive target: 32 × 32 logical px.
- Primary actions: minimum 36 logical px height.
- Compact icon-only actions are permitted only with an accessible name and tooltip.
- Panel radius: 8; small control radius: 5; focus ring: 2.
- Primary panel gap: 12–16; dense table row: 34; ordinary row: 40.

## Core palette

Base colors:

| Token | Hex | Purpose |
|---|---|---|
| `surface.canvas` | `#0B1018` | deepest background |
| `surface.base` | `#111925` | primary surface |
| `surface.raised` | `#172231` | panels |
| `surface.overlay` | `#202D3E` | popovers/selected panels |
| `border.subtle` | `#2A394B` | dividers |
| `border.strong` | `#52677D` | strong boundary |
| `text.primary` | `#F1F5F8` | primary text |
| `text.secondary` | `#AAB8C6` | secondary text |
| `text.muted` | `#78899A` | noncritical metadata |
| `focus` | `#F5D27A` | keyboard focus |

Semantic signal families:

| Token | Hex | Secondary cue |
|---|---|---|
| `signal.liveMix` | `#56C7D9` | solid line, `MIX` label |
| `signal.reference` | `#7596FF` | dashed line, `REF` label |
| `signal.proposed` | `#E4A45D` | dotted/ghost curve, `PROPOSED` label |
| `signal.processing` | `#F0BE72` | solid warm control fill, `ACTIVE` label |
| `signal.bypassed` | `#78899A` | reduced contrast + bypass icon |
| `signal.positive` | `#68C49A` | check icon |
| `signal.caution` | `#E7BE5B` | triangle icon |
| `signal.error` | `#F07A7A` | error icon and message |
| `signal.lowConfidence` | `#C89BCF` | hollow diamond and reason label |
| `signal.mid` | `#65D3B0` | `M` badge, solid line |
| `signal.side` | `#B38AE2` | `S` badge, dashed line |
| `signal.pre` | `#8E9CAB` | thin line, `PRE` label |
| `signal.post` | `#EBC06F` | thicker line, `POST` label |

These values are candidates. Automated contrast checks and Windows screenshots must verify text pairings; graph lines must be tested for color-vision deficiencies and never be identified by hue alone.

## Graph language

- Live values use solid strokes.
- Reference values use dashed strokes.
- Proposed values use dotted or translucent strokes and explicit labels.
- Captured/stable data uses a small lock/capture marker and timestamp.
- Stale data freezes visibly, desaturates, and gains a `STALE` label; it must not mimic silence.
- Grid lines remain subordinate to data.
- Filled glows are avoided except a subtle selected-band region.
- Graph animation refresh is capped by a user setting and pauses/reduces when not visible.

## Layout

- Compact: one principal pane with labelled navigation and inspector drawers.
- Standard: navigation plus workspace; inspector overlays/collapses.
- Wide: navigation, workspace, and persistent inspector when useful.
- Layout reflows at logical-width thresholds; a global scale setting may adjust density but cannot replace reflow.
- Critical action/status controls never disappear solely because width decreases.

## Controls

### Buttons

- Use verbs: **Capture analysis**, **Load reference**, **Apply proposal**.
- Destructive/cancelling actions use explicit language and confirmation proportional to loss.
- Toggle states include text or an adjacent stable label.

### Segmented controls

Use for mutually exclusive modes such as Mix/Reference or Delayed Only/Dry + Delayed/Dry Only. Keyboard arrows move selection; the selected segment exposes state to accessibility APIs.

### Knobs and numeric values

- Drag adjusts; Shift-drag is fine adjustment.
- Mouse wheel adjusts only while hovered/focused and uses a predictable step.
- Double-click or Enter opens exact entry.
- Escape cancels entry; Enter commits.
- Double-click with the documented modifier resets to default; the tooltip names the shortcut.
- Units are always visible.

### Tables and lists

- Selection, status, and primary action remain keyboard reachable.
- Sorting/filter state is textual and accessible.
- Long reference names elide in the middle or end as appropriate, with a tooltip and accessible full name.
- Row color is never the only validity indicator.

## Keyboard map

| Command | Shortcut |
|---|---|
| Move focus | Tab / Shift+Tab |
| Move within tabs/segments/lists | Arrow keys |
| Activate focused action | Enter or Space |
| Dismiss menu/dialog/drawer | Escape |
| Undo / Redo | Ctrl+Z / Ctrl+Shift+Z |
| Open preset browser | Ctrl+P |
| Save preset | Ctrl+S |
| Exact entry for focused numeric control | Enter |
| Fine adjustment | Shift + drag/wheel |
| Reset focused control | Ctrl+Backspace |
| Toggle contextual help | F1 |
| Previous/next deterministic journey step (preview only) | Ctrl+Alt+Left / Ctrl+Alt+Right |
| Previous/next selected band or division | Alt+Left / Alt+Right |

Global host conflicts must be tested in FL Studio. A shortcut that the host intercepts must have an accessible in-UI equivalent.

## Motion

- Default decorative transition duration: 120–180 ms.
- No flashing, looping decoration, parallax, or spring overshoot.
- Reduced motion removes decorative transitions and crossfades positional UI changes.
- Analyzer motion remains data-driven but may reduce refresh rate.
- Audio transition timing, such as RefTime crossfade, is not a decorative UI animation.
- Analyzer refresh defaults to 30 Hz, may offer a 60 Hz High mode, and drops to 15 Hz for reduced-motion/high-CPU protection.
- Idle, hidden, minimized, and fully obscured graphs do not repaint continuously.

## Shared shell

Every plugin shell reserves consistent locations for identity, presets, undo/redo, A/B, bypass, help, scaling, output meter where relevant, link status where relevant, and warnings. Smaller plugins may collapse labels but must preserve semantics and keyboard access.
