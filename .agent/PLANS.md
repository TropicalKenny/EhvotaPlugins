# Executable plan standard

Use an executable plan for multi-hour work, cross-module features, architectural changes, migrations, release work, or any task whose safe completion cannot be held in one short checklist. `plan.md` is the living project plan; a focused plan may live under `docs/plans/` when a milestone needs more implementation detail.

## Required qualities

Every executable plan must be:

- **Self-contained:** state the goal, constraints, affected boundaries, terminology, and prerequisites needed by a developer with no chat history.
- **Restartable:** name authoritative files, current state, completed evidence, unresolved risks, and the exact next actions.
- **Evidence-based:** give each task an observable acceptance condition and identify the command, test, inspection, screenshot, host exercise, or user approval that proves it.
- **Continuously updated:** record discoveries, decisions, deviations, failed approaches, and verification as they happen.
- **Honest:** distinguish implemented, tested, Windows-verified, FL-Studio-verified, user-approved, approximate, and unverified claims.
- **Scoped:** preserve the product goal without bundling unrelated cleanup or speculative abstractions.

## Required sections

Include:

1. Purpose and user-visible outcome.
2. Current state and evidence.
3. Constraints and non-goals.
4. Architecture and ownership boundaries.
5. Milestones with granular checkboxes.
6. Dependencies and licence implications.
7. Risks, failure modes, and recovery.
8. Acceptance criteria and verification matrix.
9. Decision log.
10. Progress log.
11. Unresolved questions.
12. Next three concrete actions.
13. Restart instructions.

## Maintenance rules

- Read the plan before implementation.
- Add significant work to the plan before or while starting it, not after completion.
- Mark a checkbox complete only beside evidence that covers its full scope.
- Record actual commands and relevant result summaries; do not paste large logs into the plan.
- If implementation changes the architecture, update the architecture and decision log in the same milestone.
- If behaviour changes, update the relevant specification and user documentation.
- If a prerequisite fails, isolate and resolve or document it before stacking dependent work on top.
- After three substantially identical failed attempts, stop, document the attempts and missing capability, make a smaller proof, and change the method.
- Keep secrets, signing material, user audio, generated binaries, and machine-specific private paths out of plans.
- A Linux build may support portable development but cannot satisfy Windows, MSVC, FL Studio, Windows DPI/accessibility, Inno Setup, VST3 validation, Authenticode, or clean-machine acceptance gates.

## Completion record

At milestone completion, record:

- files changed;
- tests and commands run;
- relevant outputs or artefact locations;
- requirements covered;
- failures and remaining uncertainty;
- documentation updated;
- diff/review outcome;
- commit identifier when committed;
- the next gated milestone.

For the Phase 1 UI gate, completion additionally requires a production-component JUCE preview, deterministic workflow states, Windows resizing/DPI/keyboard evidence, screenshots, self-review, an updated `docs/UI_CONTRACT.md`, a coherent commit, and explicit user approval before production DSP.
