# Ableton integration

This directory is reserved for the **Ehvota Bridge** and Ableton-specific integration assets.

Current status: architecture only. Production integration code is intentionally blocked until the UI refinement/approval gate is complete.

Planned contents may include:

```text
ableton/
├── bridge-device/        # Max for Live device source/assets
├── protocol/             # versioned bridge message schema
├── fixtures/             # deterministic Ableton state fixtures
├── tests/                # protocol/reconnect/identity tests
└── README.md
```

Rules:

- Keep Ehvota DSP in C++/JUCE VST3.
- Do not move RefHub, RefTap, RefEQ, RefComp, RefTime, RefColor, or other processors into Max for Live.
- The Max for Live bridge is a project-state and control bridge, not an audio processor.
- Ableton integration must be optional at runtime; Ehvota plugins must continue functioning if the bridge is absent.
- No network/cloud dependency.
- Bind future IPC to the local machine only by default.
- Do not expose audio-thread memory.
- Externally requested processing changes must pass through Ehvota proposal/validation/permission/undo systems.
- FL Studio compatibility, if revisited, is secondary and must not constrain the Ableton-first architecture.

See `docs/ABLETON_INTEGRATION.md` and `docs/DECISIONS.md`.