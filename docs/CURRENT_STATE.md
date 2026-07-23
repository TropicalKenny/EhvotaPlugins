# Current state

Last verified: 2026-07-23

## Summary

Ehvota's Bundle is in Phase 0/1 foundation work. The workspace began empty. It now has a CMake workspace, portable timing/view-model libraries, deterministic tests, and source for the Windows-only JUCE `EhvotaUIDemo` target. The UI target has not been compiled because JUCE is intentionally unconfigured pending licence acceptance and the current host is not Windows.

There is no production DSP, VST3, installer, stem model, signing workflow, or release artefact.

The repository currently contains the living plan, repository guidance, active planning/research documentation, deterministic Phase 1 models/tests, preview source, and a pinned JUCE dependency. Any UI, DSP, host, installer, or signing behaviour not explicitly backed by later evidence remains unimplemented.

## Host boundary

The current Codex host is Arch Linux x86_64. It has CMake 4.3.4, Ninja 1.13.2, Git 2.54.0, Python 3.14.5, and GCC/G++ 16.1.1.

It does not have MSVC, MSBuild, Visual Studio, the Windows SDK Authenticode `signtool.exe`, Inno Setup, PowerShell, FL Studio, or Steinberg's VST3 validator. A Linux program named `signtool` is present but is an unrelated NSS utility.

Consequences:

- Source, architecture, deterministic portable logic, and selected non-product development checks may be prepared here.
- Windows 10/11 compatibility, MSVC compilation, Windows DPI/accessibility, VST3 packaging/validation, FL Studio behavior, Inno Setup, Authenticode signing, and clean-machine behavior cannot be claimed from this host.
- The authoritative JUCE UI Preview and every Windows-specific gate must ultimately run on the intended Windows computer.
- Linux builds, if added, are development aids only and are not a supported product.

## Completed evidence

- The complete master product specification was read.
- A read-only initial audit found no project files or signing material.
- `plan.md` was the first repository mutation.
- Root `AGENTS.md`, `.agent/PLANS.md`, and `.gitignore` now define durable working rules.
- A fresh ephemeral Codex 0.145.0 session using OpenAI `gpt-5.6-terra`, read-only sandboxing, and no tool/file access reproduced the first repository rule from its injected context, verifying root `AGENTS.md` discovery. The unrelated global Stop hook reported a failure after the response; no repository change occurred in that check.
- Git was initialized and the initial branch was renamed to `main`.
- Phase 1 research, journeys, information architecture, low-fidelity wireframes, Hybrid Signal recommendation, design tokens, and provisional UI contract are documented.
- Source exists for shared JUCE UI components and a responsive preview shell covering RefHub, RefTime, and Stem Prep scenarios.
- Deterministic fixtures cover all eight primary journeys plus low-confidence, Render Safe, and missing-model states.
- `cmake --preset portable-tests`, `cmake --build --preset portable-tests`, and `ctest --preset portable-tests` passed on 2026-07-23.
- The tests cover scenario-catalogue completeness, exact RefTime calculations, invalid BPM rejection, and candidate token contrast thresholds.
- Initial foundation commit: `d816aab` (`chore: establish UI-first project foundation`).
- The user accepted JUCE 8 Starter and the JUCE 8 EULA on 2026-07-23.
- JUCE 8.0.9 is pinned as a submodule at `f72bad64d29715216226685810c5196bd0d79d77`.
- The deterministic scenario catalogue now covers all mandatory UI validation states, including empty/missing/invalid reference states, capture failure, proposal applied, unsupported bridge, high CPU, model licence failure, safe cancellation, and installer repair.
- Responsive breakpoints are implemented in a dependency-free layout policy with boundary tests.
- A compile-only C++20 check against the pinned JUCE headers passed for all shared UI and preview translation units with `-Wall -Wextra -Wpedantic`. This proves source/API syntax on the current compiler only; it is not a linked executable or Windows evidence.

## Active work

- Verify current first-party JUCE, VST3, Windows, FL Studio, and Inno Setup requirements.
- Resolve the JUCE licence decision before fetching or building the framework.
- Run the Windows audit script and record the intended toolchain.
- Configure and compile the pinned JUCE UI target with MSVC on Windows.
- Exercise and refine Ehvota UI Preview on Windows before screenshots or approval.

## Release blockers

- The intended Windows toolchain has not been audited.
- UI Preview source exists but no Windows/JUCE build or rendered evidence exists.
- The UI direction has not passed the mandatory user approval gate.
- Production DSP is intentionally blocked until that approval.
