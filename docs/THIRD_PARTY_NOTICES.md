# Third-party dependency and notice register

Status: research only; no third-party production dependency has been downloaded, vendored, linked, or distributed.

## JUCE

- Candidate: JUCE 8.0.9, based on the official release page observed on 2026-07-23.
- Source: <https://github.com/juce-framework/JUCE/releases>
- Licence: JUCE 8 EULA with Starter, Indie, Pro, and Educational tiers.
- EULA: <https://juce.com/legal/juce-8-licence/>
- Pricing/licence overview: <https://juce.com/get-juce/>
- Status: **not accepted or configured**.
- Required decision: user must select and accept a licence appropriate to personal development, revenue/funding, ownership, team size, and any distribution.
- Integration rule after acceptance: pin an exact reviewed revision and record it here; do not track an unbounded branch.

## VST3 SDK/API

- Source and documentation: <https://steinbergmedia.github.io/vst3_dev_portal/pages/>
- Licensing FAQ: <https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Licensing.html>
- Status: expected indirectly through JUCE for plugin implementation; direct SDK tools/validator may be used separately after licence/file-scope review.
- Notice requirement: follow applicable VST trademark and usage guidance in product documentation and About surfaces.

## Inno Setup

- Source/documentation: <https://jrsoftware.org/isinfo.php>
- Status: planned release tool, not a product runtime dependency and not installed on the current host.
- Required review: exact version, licence text, signing configuration, 64-bit mode, update/repair behavior, and any bundled third-party compression components.

## Fonts and icons

- Initial prototype plan uses Windows system fonts (`Segoe UI Variable`, `Segoe UI`) without bundling font files.
- No icon package is selected.
- Any bundled font or icon asset requires its exact licence and attribution here before entry into `resources/`.

## Stem-separation model and runtime

- No model or runtime is selected.
- No checkpoint or model file may enter the repository.
- Distribution is blocked until true model name/version, checkpoint licence, code/runtime licence, attribution, model size, hardware compatibility, and benchmark evidence are recorded.
- Final releases require `THIRD_PARTY_NOTICES.txt` and `MODEL_LICENSES\` generated from verified records.

## Test fixtures

- No third-party or user audio may be committed without explicit rights and provenance.
- Synthetic fixtures are preferred for automated DSP validation.
- Listening/genre benchmarks must use user-provided or properly licensed local material excluded from Git.
