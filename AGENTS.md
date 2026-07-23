# Repository rules

- Read `plan.md` before substantial work and keep it authoritative.
- Update `plan.md` whenever a milestone, decision, risk, or next action changes.
- Use a self-contained execution plan as defined in `.agent/PLANS.md` for significant features and refactors.
- Treat Windows 10/11 x64, MSVC, VST3, and unbridged FL Studio x64 as the product target. Never treat a Linux-only result as Windows validation.
- Preserve audio-thread safety: no allocation, blocking locks, file I/O, logging, UI calls, model inference, waiting, unbounded work, or escaping exceptions in the callback.
- Keep reference audition, comparison gain, and monitoring simulations out of rendered audio.
- Update the smallest relevant documentation surface for user-visible behaviour.
- Run risk-appropriate tests and report failures or unverified platform gates honestly.
- Never read, modify, print, log, or commit signing secrets, certificate passwords, private keys, user audio, or credentials.
- Do not silently expand product scope or begin production DSP before the recorded UI approval gate.
- Do not add a production dependency without licence, redistribution, maintenance, security, and size review.
- Search before adding utilities, components, modules, scripts, tests, or documentation; do not duplicate sufficient existing work.
- Keep generated builds, caches, screenshots, recordings, packages, and temporary outputs separate from source.
- After three failed correction attempts on substantially the same approach, record the evidence, stop repeating it, and change the method.
