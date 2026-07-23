# Decision log

## D-001 — Windows is the sole product target

Date: 2026-07-23
Status: Accepted

The product targets Windows 10/11 x64, VST3, MSVC, and unbridged FL Studio x64. Linux may host source authoring and portable development checks only. It is never release evidence or a supported product build.

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
Status: Proposed; pending prototype review

Use a charcoal/navy base, a cool analytical identity for live/reference comparison, and a restrained warm identity for proposed/active processing. This best clarifies the product's central distinction between observation and intervention while remaining feasible in JUCE. It must not become neon, game-like, or a blend of all three directions.

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

Both are standard locations documented by Steinberg. FL Studio scanning must still be verified explicitly.
