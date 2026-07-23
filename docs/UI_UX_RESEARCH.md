# UI/UX research

Last researched: 2026-07-23
Scope: Phase 1 workflow and interaction benchmarks. This is comparative research, not permission to copy another product's visual identity.

## Research method

The initial pass used current first-party product manuals, product pages, and platform documentation. It focused on interaction patterns that reduce listening bias, make dense metering legible, expose precise parameter control, support linked instances, and survive Windows resizing/DPI conditions.

No competitor assets will be bundled. Screenshots used later for private research review must remain generated/reference artefacts and retain source attribution.

## User and context

### Primary persona — independent mix engineer/producer

- Works primarily in FL Studio on Windows.
- Mixes full songs and buses in long sessions on a laptop or desktop.
- Uses commercial references but needs faster section, component, dynamics, and spatial comparison.
- Values exact numerical control and direct listening more than decorative visualization.
- Wants proposals explained and bounded, never silently applied.
- May have only one display and limited editor space.

### Secondary persona — returning project maintainer

- Reopens an older FL Studio project after plugin, preset, reference-path, or machine changes.
- Needs stable identity, relinking, clear degraded states, and recoverable history.
- Must understand what is live, cached, missing, disconnected, proposed, or actually processing.

### Usage contexts

- Fast live monitoring while balancing a mix.
- Deliberate capture of corresponding song sections.
- Bus/stem mapping and diagnosis.
- Precision processor adjustment.
- Tempo-to-time calculation on a send.
- Offline reference-stem preparation.
- Export, where monitoring aids must be visibly and technically excluded.

## Benchmark findings

### Reference comparison

ADPTR Metric AB emphasizes immediate A/B switching, keyboard operation, multiple references, loudness matching, synchronized playback, filters, and resizable UI. The durable lesson is that audition state and loudness matching must remain persistent and unmistakable; they should not be buried inside an analysis tab.

Mastering The Mix REFERENCE exposes a reference library, loops, level matching, track alignment, meter views, stereo-width and over-compression information, resizing, and a sender utility. The lesson is to connect listening, selection, and measurement while avoiding an unexplained universal score as the main decision.

Ehvota consequence:

- Keep Mix/Reference, loudness match, section/loop, capture, and blind comparison in RefHub's persistent strip.
- Explain what is being compared and whether the result is live or captured.
- Present several named measurement dimensions and confidence reasons, not a single “professional” percentage.

### Direct manipulation and progressive disclosure

FabFilter Pro-Q's official manual documents a large interactive frequency display, selection-driven floating band controls, exact keyboard entry, wheel/fine adjustment, previous/next band navigation, and advanced panels that appear when relevant. It also labels masking/collision display as an indication rather than exact science.

Ehvota consequence:

- RefEQ should create/select bands directly in the graph.
- Selected-band controls appear contextually, with exact values and stable automation identity.
- Advanced dynamics/phase options live in an inspector.
- Analytical warnings must describe uncertainty and encourage listening.
- This interaction benchmark does not justify copying FabFilter's colors, shapes, layout proportions, gestures wholesale, or brand.

### Dense metering and linked sources

iZotope Insight uses distinct meter modules for loudness, levels, sound field, and spectrogram, and Relay provides data from other tracks. Youlean Loudness Meter supports resizable/scalable and mini views, separate graphs and numerical readouts, A/B states, and exposes targets with both numbers and state colors.

Ehvota consequence:

- RefHub views should be task-specific modules, not an all-at-once dashboard.
- RefTap/RefLink health and source identity must be explicit.
- Compact layouts retain critical numeric readouts while hiding secondary graphs.
- Color accompanies labels, icons, line patterns, and numbers.
- Saved UI scale and layout must recover safely when monitor topology changes.

### Windows scaling and accessibility

Microsoft recommends per-monitor DPI awareness for desktop applications and explicitly calls for testing startup on different displays, live scale changes, mixed-DPI movement, and primary-display changes. Its accessibility guidance calls for keyboard-only journeys, logical Tab/arrow navigation, exposed names/roles/states, Narrator testing, UI Automation checks, and scaling/high-contrast validation.

Ehvota consequence:

- Continuous reflow and readable physical sizing are requirements, not a global bitmap zoom.
- Test 100%, 125%, 150%, 175%, and 200% on Windows.
- Make keyboard focus persistent and visible.
- Expose semantic control information through JUCE accessibility handlers where supported.
- Test with Narrator and Windows accessibility tools, not just visual inspection.

### Host behavior

FL Studio's manual documents Smart Disable as a live-playback CPU feature and warns that it can interfere with time-based or long-decay behavior; it is disabled during rendering. Its wrapper also distinguishes manual and plugin-reported latency.

Ehvota consequence:

- RefLink health, metering continuity, capture, RefTime transitions, and decay processors need explicit Smart Disable tests.
- The UI must distinguish stale/disconnected measurements from silence.
- Intentional musical delay must never be reported as technical latency.
- Render-safe state is an explicit shell state even though Smart Disable behavior differs during rendering.

## Visual direction comparison

### A — Precision Studio

Strengths:

- Highest information density and easiest neutral graph comparison.
- Lowest rendering complexity.
- Strong fit for technical trust.

Risks:

- Can feel generic or clinical.
- Reference, live mix, and proposed processing may become too visually similar.

### B — Warm Technical

Strengths:

- Premium, calm, and musical.
- Warm metal accents can give Ehvota a recognizable signature without fake hardware.

Risks:

- Warm accent overuse weakens semantic distinction.
- Subtle material depth can reduce clarity or become expensive to render.
- Could imply “analogue” even in clean analysis tools.

### C — Hybrid Signal

Strengths:

- Best semantic fit: cool colors represent observation/reference analysis, while restrained warmth represents proposed or active processing.
- Supports clear pre/post, Mix/Reference, Mid/Side, and confidence distinctions.
- More distinctive than a purely neutral analyser while remaining feasible with JUCE primitives.

Risks:

- Two accent families can become neon or noisy without strict token rules.
- Must not equate “warm” with always beneficial processing.

## Recommendation

Recommend **Hybrid Signal** for the interactive prototype.

Use deep charcoal/navy surfaces, a cool cyan-blue analytical family, and a restrained amber-copper processing family. Keep saturation low, reserve bright values for active focus or warnings, and use neutral typography for most content. Stable semantic tokens—not arbitrary screen styling—govern every product.

This is a proposal pending the mandatory interactive prototype review and user approval.

## Sources

- [ADPTR Metric AB](https://www.plugin-alliance.com/products/metric-ab)
- [Mastering The Mix REFERENCE manual](https://www.masteringthemix.com/pages/reference-manual)
- [FabFilter Pro-Q 4 band controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols)
- [FabFilter Pro-Q 4 spectrum analyser](https://www.fabfilter.com/help/pro-q/using/analyzer)
- [iZotope Insight and Relay](https://www.izotope.com/en/products/insight/features/relay)
- [Youlean Loudness Meter](https://youlean.co/youlean-loudness-meter/)
- [Windows high-DPI desktop guidance](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows)
- [Windows accessibility testing](https://learn.microsoft.com/en-us/windows/apps/design/accessibility/accessibility-testing)
- [FL Studio Plugin Wrapper](https://www.image-line.com/fl-studio-learning-content/fl-studio-online-manual/html/plugins/wrapper.htm)
