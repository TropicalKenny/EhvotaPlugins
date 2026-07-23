# Architecture

Status: Phase 1 boundaries proposed; implementation pending.

## Product boundary

Ehvota's Bundle is one Windows-only product family, not a collection of unrelated plugins. Shared modules own the visual language, state, presets, timing, analysis, communication, safety, and diagnostic conventions. Each executable or VST3 owns only its product-specific orchestration and processing.

## Layering

```text
Apps and VST3 editors
        │
        ├── shared/ui + feature view models
        ├── shared/state + shared/presets
        └── feature application services
                 │
                 ├── analysis
                 ├── timing
                 ├── communication
                 ├── safety
                 └── audio/DSP
```

Rules:

- UI components render immutable or message-thread-owned view data; they never reach directly into an audio callback.
- View models expose semantic states rather than graph-specific ad hoc booleans.
- Audio processors own real-time state and exchange bounded snapshots through lock-free or equivalently real-time-safe structures.
- File I/O, library scanning, hashing, model inference, logging, proposal generation, and persistence run off the audio thread.
- Monitoring/reference audition and actual processing are separate signal-path concepts. Render Safe Mode can disable the former without bypassing the latter.
- Cross-instance RefLink begins same-process and unbridged. No network or cloud transport is permitted.

## Phase 1 production-component architecture

```text
apps/ui-preview/
  Main.cpp
  PreviewApplication.*
  PreviewWindow.*
  PreviewScenarioController.*

shared/ui/
  tokens/
  components/
  layout/
  accessibility/
  graphs/
  shell/

shared/viewmodels/
  CommonViewModels.*
  RefHubViewModel.*
  StemPrepViewModel.*
  ProcessorViewModels.*
  PreviewScenarios.*
```

The preview must consume the same `shared/ui` components and contracts intended for plugin editors. Mock data belongs in scenario fixtures, never embedded in drawing code.

## Planned internal modules

- `RefLink`: instance identity, registration, health, compact metrics, deliberate capture transport.
- `RefTimeCore`: tempo/meter/sample conversions and click-free timing-transition policy.
- `RefAnalysisCore`: live/capture measurements, activity gating, confidence inputs, section and component models.
- `RefAnalogCore`: optional measured nonlinear processing; not active during Phase 1.
- `RefPresetCore`: suite-consistent presets, schema/versioning, A/B, migration.
- `RefSafetyCore`: proposal limits, render-safety policy, state validation, finite-value guards.
- `RefUI`: tokens, components, shells, layouts, graph rendering, accessibility.
- `RefState`: serializable project/application state with explicit migration.
- `RefLogging`: non-real-time diagnostics with redaction and bounded storage.

## Ownership constraints

- No generic catch-all `utilities` API without a narrow owner and documented consumer.
- No plugin editor may define a second version of a shared shell control or semantic color.
- No business or safety rule may exist independently in UI and processor code.
- VST3 class IDs and serialized parameter IDs become compatibility contracts once released.
- External dependencies enter through isolated adapters and require a documented licence/maintenance decision.

## Windows architecture constraints

- Product binaries are x64 only and built with MSVC.
- Standalone apps and VST3 editors must respond correctly to Windows per-monitor DPI behavior.
- VST3 packages use the standard bundle layout and are staged to the user-level VST3 location during development; the installer uses the all-users location.
- Windows-specific integrations must have a portable core where reasonable, but portability must not distort the Windows product.
