# Third-party dependency and notice register

Status: JUCE source is pinned for development; no third-party binary or model is distributed yet.

## JUCE

- Selected: JUCE 8.0.9.
- Pinned tag commit: `f72bad64d29715216226685810c5196bd0d79d77`.
- Repository location: `third_party/JUCE` Git submodule.
- Source: <https://github.com/juce-framework/JUCE/releases>
- Licence: JUCE 8 EULA with Starter, Indie, Pro, and Educational tiers.
- EULA: <https://juce.com/legal/juce-8-licence/>
- Pricing/licence overview: <https://juce.com/get-juce/>
- Licence selection: JUCE 8 Starter.
- Acceptance: user explicitly accepted the JUCE 8 EULA on 2026-07-23.
- Integration rule: the Git submodule pins the exact reviewed release. Do not update it without a new licence, maintenance, compatibility, diff, and test review.

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
