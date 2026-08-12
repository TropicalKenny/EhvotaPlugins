# Decision log

## D-001 — Windows is the sole product target

Date: 2026-07-23
Status: Accepted

The product targets Windows 10/11 x64, VST3, MSVC, and unbridged 64-bit hosts. Linux may host source authoring and portable development checks only. It is never release evidence or a supported product build.

## D-002 — Production DSP is blocked by UI approval

Date: 2026-07-23
Status: Accepted

Phase 1 must deliver a reusable JUCE UI Preview with deterministic scenarios, Windows validation evidence, screenshots, and self-review. The user must explicitly approve the UI direction before Phase 2 production DSP.

## D-003 — Ehvota UI Preview is production-component code

Date: 2026-07-23
Status: Accepted

The preview will use the shared components, tokens, layouts, view models, and interaction contracts intended for the product. HTML and design images may inform exploration but are not authoritative.

## D-004 — Recommended exploration direction is Hybrid Signal

Date: 2026-07-23
Status: Proposed; superseded by later UI review

Use a charcoal/navy base, a cool analytical identity for live/reference comparison, and a restrained warm identity for proposed/active processing. Later UI review retained the dark analytical foundation but moved the recommended production direction toward a calmer Precision Studio hierarchy with fewer permanent destinations and a much larger musical/analyser workspace.

See `docs/UI_UX_RESEARCH.md` and `docs/DESIGN_SYSTEM.md`.

## D-005 — Use JUCE 8 Starter with JUCE 8.0.9 pinned

Date: 2026-07-23
Status: Accepted

The user explicitly accepted the JUCE 8 EULA and selected JUCE 8 Starter on 2026-07-23.

Pin the official JUCE 8.0.9 tag at commit `f72bad64d29715216226685810c5196bd0d79d77` as `third_party/JUCE`. The tag and commit were verified directly against the official GitHub remote. CMake uses the submodule by default and permits an explicit `JUCE_ROOT` override. It never downloads JUCE or accepts a licence during configuration.

## D-006 — Inno Setup remains the installer default

Date: 2026-07-23
Status: Provisional

Inno Setup's documented types/components, 64-bit installation, stable `AppId`, uninstall logging, and signing hooks appear capable of the requested installer shape. Repair/modify UX and preserved personal-data behavior still require a proof on Windows before this decision becomes final.

## D-007 — Development VST3 staging uses the per-user standard location

Date: 2026-07-23
Status: Accepted

Development scripts should stage VST3 bundles under:

```text
%LOCALAPPDATA%\Programs\Common\VST3\Ehvota\
```

This avoids requiring administrator access during routine development. The final installer targets:

```text
C:\Program Files\Common Files\VST3\Ehvota\
```

Both are standard VST3 locations. Host scanning must still be verified explicitly.

## D-008 — Ableton Live 12 Suite becomes the primary DAW

Date: 2026-08-12
Status: Accepted

Ehvota's Bundle is now designed primarily around **Ableton Live 12 Suite on Windows x64** instead of FL Studio. The audio processors remain standard VST3 plugins so that future secondary-host compatibility remains possible.

The reason is architectural: the long-term product includes a controlled Codex/DAW bridge. Ableton's Max for Live and Live Object Model provide a substantially richer and more explicit project-state/control surface than the FL Studio scripting path originally assumed. The user is already highly proficient with Ableton, so there is no workflow-learning penalty that would justify retaining FL Studio as the architecture driver.

Consequences:

- All new host-specific research, validation, examples, UI states, tooling, and acceptance criteria must be Ableton-first.
- Existing FL Studio-specific assumptions are considered legacy until either removed or explicitly retained as secondary compatibility notes.
- RefHub, RefTap, RefEQ, RefComp, RefTime, RefColor, Stem Prep, reference folders, mapping, analysis, Learn Mode, proposals, signing, and installer architecture remain valid.
- RefTap remains necessary because the Live Object Model describes project/device state but does not replace detailed real-time audio measurement.
- FL Studio may be revisited later as a secondary host after Ableton support is stable.

## D-009 — Max for Live is the DAW integration bridge; Ehvota DSP stays VST3

Date: 2026-08-12
Status: Accepted

Do not convert Ehvota's processing suite into Max for Live devices. Keep production audio effects in C++/JUCE VST3.

Add a dedicated Ableton integration layer, provisionally named `Ehvota Bridge`, implemented as a Max for Live device plus a local companion bridge where required. Its responsibilities are project-state discovery and safe local communication, not audio processing.

Target architecture:

```text
Codex
  |
  | MCP
  v
Ehvota DAW MCP (separate repository later)
  |
  | localhost IPC
  v
Ehvota Bridge / Ableton adapter
  |                 \
  | Live Object       \ RefLink
  | Model              \
  v                     v
Ableton Live        Ehvota VST3 plugins
                    |  RefHub
                    |  RefTap
                    |  RefEQ
                    |  RefComp
                    \  other Ehvota processors
```

The Ehvota repository must preserve an **external automation boundary**: a future local bridge may read structured state and request safe actions, but must never directly poke audio-thread memory. All externally writable Ehvota actions must pass through the same proposal, validation, permission, and undo model used by RefHub.