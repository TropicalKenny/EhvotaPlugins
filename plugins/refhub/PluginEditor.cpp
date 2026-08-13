#include "PluginEditor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ehvota::refhub {
namespace {

const auto canvas = juce::Colour(0xff090d12);
const auto panel = juce::Colour(0xff10161d);
const auto raised = juce::Colour(0xff151c24);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe8edf2);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto muted = juce::Colour(0xff66727e);
const auto cyan = juce::Colour(0xff62d8ff);
const auto violet = juce::Colour(0xff9278ff);
const auto amber = juce::Colour(0xffe8a85b);
const auto green = juce::Colour(0xff64d98b);

constexpr std::array<ehvota::state::ReferenceRole, 4> componentRoles {
    ehvota::state::ReferenceRole::drums,
    ehvota::state::ReferenceRole::bass,
    ehvota::state::ReferenceRole::vocals,
    ehvota::state::ReferenceRole::music
};

void drawPanel(juce::Graphics& g, const juce::Rectangle<int> area, const bool raisedPanel = false)
{
    g.setColour(raisedPanel ? raised : panel);
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void drawWave(juce::Graphics& g, const juce::Rectangle<float> area, const juce::Colour colour, const float phase)
{
    juce::Path path;
    for (int x = 0; x <= static_cast<int>(area.getWidth()); ++x) {
        const auto t = static_cast<float>(x) / juce::jmax(1.0f, area.getWidth());
        const auto y = area.getCentreY()
            - (std::sin(t * 90.0f + phase) + 0.4f * std::sin(t * 237.0f + phase * 0.5f))
                * area.getHeight() * 0.20f;
        const auto px = area.getX() + static_cast<float>(x);
        if (x == 0) path.startNewSubPath(px, y); else path.lineTo(px, y);
    }
    g.setColour(colour.withAlpha(0.9f));
    g.strokePath(path, juce::PathStrokeType(1.1f));
}

juce::String snapshotName(const ehvota::communication::SharedInstanceSnapshot& snapshot)
{
    auto result = juce::String::fromUTF8(snapshot.name.data());
    if (result.isEmpty())
        result = juce::String(ehvota::communication::sharedPluginKindName(snapshot.pluginKind).data());
    return result;
}

juce::String roleName(const ehvota::state::ReferenceRole role)
{
    return juce::String(ehvota::state::referenceRoleName(role).data());
}

juce::String componentBatchName(const ComponentCaptureBatchState state)
{
    switch (state) {
    case ComponentCaptureBatchState::idle: return "Idle";
    case ComponentCaptureBatchState::scheduled: return "Scheduled";
    case ComponentCaptureBatchState::capturing: return "Capturing";
    case ComponentCaptureBatchState::analysing: return "Analysing";
    case ComponentCaptureBatchState::complete: return "Complete";
    case ComponentCaptureBatchState::partial: return "Partial";
    case ComponentCaptureBatchState::error: return "Error";
    }
    return "Unknown";
}

bool sameId(const ehvota::communication::SharedInstanceId a,
    const ehvota::communication::SharedInstanceId b) noexcept
{
    return a.slot == b.slot && a.generation == b.generation;
}

float spectrumX(const double frequency, const juce::Rectangle<float> graph)
{
    const auto f = std::clamp(frequency, 20.0, 20000.0);
    const auto t = std::log10(f / 20.0) / std::log10(1000.0);
    return graph.getX() + static_cast<float>(t) * graph.getWidth();
}

float spectrumY(const double db, const juce::Rectangle<float> graph)
{
    const auto clamped = std::clamp(db, -100.0, 0.0);
    const auto t = static_cast<float>((clamped + 100.0) / 100.0);
    return graph.getBottom() - t * graph.getHeight();
}

void drawSpectrumPath(juce::Graphics& g,
    const ehvota::analysis::SpectrumSummary& spectrum,
    const juce::Rectangle<float> graph,
    const juce::Colour colour)
{
    juce::Path path;
    bool started = false;
    for (const auto& bin : spectrum.bins) {
        if (bin.frequencyHz < 20.0 || bin.frequencyHz > 20000.0)
            continue;
        const auto x = spectrumX(bin.frequencyHz, graph);
        const auto y = spectrumY(bin.powerDb, graph);
        if (!started) {
            path.startNewSubPath(x, y);
            started = true;
        } else {
            path.lineTo(x, y);
        }
    }
    if (started) {
        g.setColour(colour);
        g.strokePath(path, juce::PathStrokeType(1.5f));
    }
}

} // namespace

RefHubAudioProcessorEditor::RefHubAudioProcessorEditor(RefHubAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    for (auto* component : {
             static_cast<juce::Component*>(&navCompare_), static_cast<juce::Component*>(&navSources_),
             static_cast<juce::Component*>(&navProcess_), static_cast<juce::Component*>(&navHistory_),
             static_cast<juce::Component*>(&navSettings_), static_cast<juce::Component*>(&loadFolderButton_),
             static_cast<juce::Component*>(&referenceSelector_), static_cast<juce::Component*>(&sectionSelector_),
             static_cast<juce::Component*>(&levelMatchSelector_), static_cast<juce::Component*>(&mixButton_),
             static_cast<juce::Component*>(&referenceButton_), static_cast<juce::Component*>(&blindButton_),
             static_cast<juce::Component*>(&captureButton_), static_cast<juce::Component*>(&proposalButton_),
             static_cast<juce::Component*>(&liveMeterLabel_), static_cast<juce::Component*>(&componentCaptureButton_),
             static_cast<juce::Component*>(&componentStatusLabel_), static_cast<juce::Component*>(&createProposalButton_),
             static_cast<juce::Component*>(&approveProposalButton_), static_cast<juce::Component*>(&rejectProposalButton_),
             static_cast<juce::Component*>(&undoButton_), static_cast<juce::Component*>(&renderSafeToggle_),
             static_cast<juce::Component*>(&liveDurationEditor_), static_cast<juce::Component*>(&referenceStartEditor_),
             static_cast<juce::Component*>(&referenceDurationEditor_) })
        addAndMakeVisible(*component);

    for (auto& selector : mappingSelectors_) addAndMakeVisible(selector);
    for (auto& toggle : scopeToggles_) addAndMakeVisible(toggle);

    navCompare_.onClick = [this] { setPage(Page::compare); };
    navSources_.onClick = [this] { setPage(Page::sources); };
    navProcess_.onClick = [this] { setPage(Page::process); };
    navHistory_.onClick = [this] { setPage(Page::history); };
    navSettings_.onClick = [this] { setPage(Page::settings); };

    referenceSelector_.addItem("No reference loaded", 1);
    referenceSelector_.setSelectedId(1, juce::dontSendNotification);
    referenceSelector_.setEnabled(false);
    sectionSelector_.addItem("Manual Window", 1);
    sectionSelector_.setSelectedId(1, juce::dontSendNotification);

    levelMatchSelector_.addItem("AUTO / SECTION", 1);
    levelMatchSelector_.addItem("WHOLE TRACK", 2);
    levelMatchSelector_.addItem("OFF", 3);
    const auto initialMode = processor_.levelMatchMode();
    levelMatchSelector_.setSelectedId(initialMode == RefHubLevelMatchMode::autoSection ? 1
        : initialMode == RefHubLevelMatchMode::wholeTrack ? 2 : 3, juce::dontSendNotification);
    levelMatchSelector_.onChange = [this] {
        const auto id = levelMatchSelector_.getSelectedId();
        processor_.setLevelMatchMode(id == 1 ? RefHubLevelMatchMode::autoSection
            : id == 2 ? RefHubLevelMatchMode::wholeTrack : RefHubLevelMatchMode::off);
    };

    loadFolderButton_.setTooltip("Open one reference folder containing the authoritative full mix and optional drums, bass, vocals, and music stems.");
    loadFolderButton_.onClick = [this] { chooseReferenceFolder(); };

    mixButton_.setClickingTogglesState(true);
    referenceButton_.setClickingTogglesState(true);
    blindButton_.setClickingTogglesState(true);
    referenceButton_.setEnabled(false);
    mixButton_.setToggleState(true, juce::dontSendNotification);
    blindButton_.setToggleState(processor_.blindComparison(), juce::dontSendNotification);
    mixButton_.onClick = [this] {
        processor_.stopReferenceAudition();
        mixButton_.setToggleState(true, juce::dontSendNotification);
        referenceButton_.setToggleState(false, juce::dontSendNotification);
    };
    referenceButton_.onClick = [this] {
        processor_.startReferenceAudition();
        const auto active = processor_.referenceAuditionActive();
        referenceButton_.setToggleState(active, juce::dontSendNotification);
        mixButton_.setToggleState(!active, juce::dontSendNotification);
    };
    blindButton_.onClick = [this] { processor_.setBlindComparison(blindButton_.getToggleState()); };

    liveDurationEditor_.setText("8.0", false);
    referenceStartEditor_.setText("0.0", false);
    referenceDurationEditor_.setText("8.0", false);
    for (auto* editor : { &liveDurationEditor_, &referenceStartEditor_, &referenceDurationEditor_ }) {
        editor->setInputRestrictions(8, "0123456789.");
        editor->setJustification(juce::Justification::centred);
    }

    captureButton_.onClick = [this] {
        const auto duration = editorDouble(liveDurationEditor_).value_or(8.0);
        if (processor_.beginMixCapture(duration))
            captureButton_.setButtonText("CAPTURING");
    };
    proposalButton_.setTooltip("Analyse the captured mix against the manual reference window and stage a bounded RefEQ proposal when a RefEQ is linked. Nothing is applied automatically.");
    proposalButton_.onClick = [this] { beginMainAnalysisFromWindow(); };

    for (std::size_t i = 0; i < scopeToggles_.size(); ++i) {
        scopeToggles_[i].setButtonText(roleName(componentRoles[i]));
        scopeToggles_[i].setToggleState(true, juce::dontSendNotification);
        scopeToggles_[i].onClick = [this, i] {
            analysisScope_.setIncluded(componentRoles[i], scopeToggles_[i].getToggleState());
        };
        mappingSelectors_[i].onChange = [this, i] { configureMappingSelector(i); };
    }
    componentCaptureButton_.onClick = [this] { beginComponentCaptureFromWindow(); };
    componentStatusLabel_.setColour(juce::Label::textColourId, secondary);
    componentStatusLabel_.setJustificationType(juce::Justification::centredLeft);

    createProposalButton_.onClick = [this] {
        if (const auto target = firstLinked(ehvota::communication::SharedPluginKind::refEQ))
            (void) processor_.stageLatestRefEqProposal(*target);
    };
    approveProposalButton_.onClick = [this] { (void) processor_.approveAndSubmitStagedProposal(); };
    rejectProposalButton_.onClick = [this] { processor_.rejectStagedProposal(); };
    undoButton_.onClick = [this] { (void) processor_.undoLastAppliedProposal(); };

    renderSafeToggle_.setToggleState(processor_.renderSafe(), juce::dontSendNotification);
    renderSafeToggle_.onClick = [this] { processor_.setRenderSafe(renderSafeToggle_.getToggleState()); };

    liveMeterLabel_.setColour(juce::Label::textColourId, secondary);
    liveMeterLabel_.setJustificationType(juce::Justification::centredRight);

    setResizable(true, true);
    setResizeLimits(860, 600, 1800, 1200);
    setSize(1180, 760);
    refreshLinkedSources();
    updateVisibility();
    startTimerHz(15);
}

std::optional<double> RefHubAudioProcessorEditor::editorDouble(const juce::TextEditor& editor) const
{
    const auto value = editor.getText().getDoubleValue();
    if (!std::isfinite(value) || value < 0.0)
        return std::nullopt;
    return value;
}

std::optional<ehvota::communication::SharedInstanceId> RefHubAudioProcessorEditor::firstLinked(
    const ehvota::communication::SharedPluginKind kind) const
{
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linked {};
    const auto count = processor_.copyLinkedInstances(linked);
    for (std::size_t i = 0; i < count; ++i)
        if (linked[i].pluginKind == kind && linked[i].heartbeatAgeMs <= 5000)
            return linked[i].id;
    return std::nullopt;
}

void RefHubAudioProcessorEditor::setPage(const Page page)
{
    page_ = page;
    updateVisibility();
    resized();
    repaint();
}

void RefHubAudioProcessorEditor::updateVisibility()
{
    const auto compare = page_ == Page::compare;
    const auto sources = page_ == Page::sources;
    const auto process = page_ == Page::process;
    const auto settings = page_ == Page::settings;

    loadFolderButton_.setVisible(compare || sources);
    for (auto* c : { static_cast<juce::Component*>(&referenceSelector_), static_cast<juce::Component*>(&sectionSelector_),
             static_cast<juce::Component*>(&levelMatchSelector_), static_cast<juce::Component*>(&mixButton_),
             static_cast<juce::Component*>(&referenceButton_), static_cast<juce::Component*>(&blindButton_),
             static_cast<juce::Component*>(&captureButton_), static_cast<juce::Component*>(&proposalButton_),
             static_cast<juce::Component*>(&liveMeterLabel_) })
        c->setVisible(compare);

    for (auto& selector : mappingSelectors_) selector.setVisible(sources);
    for (auto& toggle : scopeToggles_) toggle.setVisible(sources);
    componentCaptureButton_.setVisible(sources);
    componentStatusLabel_.setVisible(sources);
    liveDurationEditor_.setVisible(sources);
    referenceStartEditor_.setVisible(sources);
    referenceDurationEditor_.setVisible(sources);

    createProposalButton_.setVisible(process);
    approveProposalButton_.setVisible(process);
    rejectProposalButton_.setVisible(process);
    undoButton_.setVisible(process);
    renderSafeToggle_.setVisible(settings);
}

void RefHubAudioProcessorEditor::chooseReferenceFolder()
{
    folderChooser_ = std::make_unique<juce::FileChooser>(
        "Choose an Ehvota reference folder",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory));
    folderChooser_->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
        [safe = juce::Component::SafePointer<RefHubAudioProcessorEditor>(this)](const juce::FileChooser& chooser) {
            if (safe == nullptr) return;
            const auto directory = chooser.getResult();
            if (directory != juce::File{}) safe->applyReferenceFolder(directory);
        });
}

void RefHubAudioProcessorEditor::applyReferenceFolder(const juce::File& directory)
{
    processor_.stopReferenceAudition();
    loadedReference_ = loadReferenceFolder(directory);
    juce::String validationText;
    for (const auto& issue : loadedReference_.validation.issues) {
        if (validationText.isNotEmpty()) validationText << "\n";
        validationText << (issue.blocking ? "Blocking: " : "Note: ") << issue.message;
    }

    if (!loadedReference_.validation.valid || !loadedReference_.authoritativeFullMixFile.existsAsFile()) {
        processor_.unloadReferenceFile();
        referenceSelector_.clear(juce::dontSendNotification);
        referenceSelector_.addItem("Invalid reference folder", 1);
        referenceSelector_.setSelectedId(1, juce::dontSendNotification);
        referenceSelector_.setTooltip(validationText);
        referenceSelector_.setEnabled(false);
        referenceButton_.setEnabled(false);
        mixButton_.triggerClick();
        repaint();
        return;
    }

    if (!processor_.loadReferenceFile(loadedReference_.authoritativeFullMixFile)) {
        referenceSelector_.clear(juce::dontSendNotification);
        referenceSelector_.addItem("Could not load full mix", 1);
        referenceSelector_.setSelectedId(1, juce::dontSendNotification);
        referenceSelector_.setEnabled(false);
        referenceButton_.setEnabled(false);
        return;
    }

    referenceSelector_.clear(juce::dontSendNotification);
    referenceSelector_.addItem(juce::String(loadedReference_.package.name), 1);
    referenceSelector_.setSelectedId(1, juce::dontSendNotification);
    referenceSelector_.setTooltip(validationText.isNotEmpty()
        ? validationText
        : "Reference package validated. The full mix is authoritative; separated stems remain diagnostic unless alignment is verified.");
    referenceSelector_.setEnabled(true);
    referenceButton_.setEnabled(true);
    repaint();
}

void RefHubAudioProcessorEditor::refreshLinkedSources()
{
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linked {};
    const auto count = processor_.copyLinkedInstances(linked);
    juce::String signature;
    for (std::size_t i = 0; i < count; ++i) {
        if (linked[i].pluginKind != ehvota::communication::SharedPluginKind::refTap) continue;
        signature << linked[i].id.slot << ':' << static_cast<int>(linked[i].id.generation)
                  << ':' << snapshotName(linked[i]) << ';';
    }
    if (signature == linkedSignature_)
        return;
    linkedSignature_ = signature;

    const auto mappings = processor_.copyComponentMappings();
    for (std::size_t role = 0; role < mappingSelectors_.size(); ++role) {
        auto& selector = mappingSelectors_[role];
        selector.onChange = nullptr;
        selector.clear(juce::dontSendNotification);
        selector.addItem("Unmapped", 1);
        mappingChoices_[role].clear();
        int selectedId = 1;
        for (std::size_t i = 0; i < count; ++i) {
            if (linked[i].pluginKind != ehvota::communication::SharedPluginKind::refTap
                || linked[i].heartbeatAgeMs > 5000)
                continue;
            mappingChoices_[role].push_back(linked[i].id);
            const auto itemId = 100 + static_cast<int>(mappingChoices_[role].size());
            auto label = snapshotName(linked[i]);
            label << "  ·  RefTap " << linked[i].id.slot << ':' << static_cast<int>(linked[i].id.generation);
            selector.addItem(label, itemId);
            if (mappings[role].enabled && sameId(mappings[role].refTap, linked[i].id))
                selectedId = itemId;
        }
        selector.setSelectedId(selectedId, juce::dontSendNotification);
        selector.onChange = [this, role] { configureMappingSelector(role); };
    }
}

void RefHubAudioProcessorEditor::configureMappingSelector(const std::size_t roleIndex)
{
    if (roleIndex >= mappingSelectors_.size()) return;
    const auto selected = mappingSelectors_[roleIndex].getSelectedId();
    if (selected <= 1) {
        processor_.clearComponentMapping(componentRoles[roleIndex]);
        return;
    }
    const auto choice = static_cast<std::size_t>(selected - 101);
    if (choice >= mappingChoices_[roleIndex].size()
        || !processor_.setComponentMapping(componentRoles[roleIndex], mappingChoices_[roleIndex][choice])) {
        componentStatusLabel_.setText("Mapping rejected: endpoint is no longer a fresh RefTap.", juce::dontSendNotification);
        linkedSignature_.clear();
        refreshLinkedSources();
    }
}

void RefHubAudioProcessorEditor::beginComponentCaptureFromWindow()
{
    const auto live = editorDouble(liveDurationEditor_);
    const auto start = editorDouble(referenceStartEditor_);
    const auto reference = editorDouble(referenceDurationEditor_);
    if (!live || !start || !reference || *live <= 0.0 || *reference <= 0.0) {
        componentStatusLabel_.setText("Enter valid positive section durations and a non-negative reference start.", juce::dontSendNotification);
        return;
    }
    if (processor_.beginComponentCapture(*live, *start, *reference, analysisScope_))
        componentStatusLabel_.setText("Mapped buses scheduled to one common host-sample window.", juce::dontSendNotification);
    else
        componentStatusLabel_.setText("Component capture could not start. Check mappings, reference stems, host timing and existing captures.", juce::dontSendNotification);
}

void RefHubAudioProcessorEditor::beginMainAnalysisFromWindow()
{
    const auto start = editorDouble(referenceStartEditor_);
    const auto duration = editorDouble(referenceDurationEditor_);
    if (!start || !duration || *duration <= 0.0)
        return;
    const auto target = firstLinked(ehvota::communication::SharedPluginKind::refEQ);
    if (target)
        (void) processor_.startSectionAnalysis(*target, *start, *duration, "manual-window");
    else
        (void) processor_.startSectionAnalysis(*start, *duration, "manual-window");
}

void RefHubAudioProcessorEditor::paintCommon(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto header = bounds.removeFromTop(62);
    g.setColour(text);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota", header.removeFromLeft(92), juce::Justification::centredLeft);
    g.setColour(cyan);
    g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    g.drawText("RefHub", header.removeFromLeft(70), juce::Justification::centredLeft);

    const char* pageName = page_ == Page::compare ? "COMPARE" : page_ == Page::sources ? "SOURCES / MAPPING"
        : page_ == Page::process ? "PROCESS" : page_ == Page::history ? "HISTORY" : "SETTINGS";
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
    g.drawText(pageName, header.removeFromLeft(150), juce::Justification::centredLeft);

    g.setColour(green);
    g.fillEllipse(static_cast<float>(header.getX() + 6), static_cast<float>(header.getCentreY() - 3), 6.0f, 6.0f);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("LIVE", header.getX() + 18, header.getY(), 52, header.getHeight(), juce::Justification::centredLeft);

    auto rail = bounds.removeFromLeft(82);
    drawPanel(g, rail);
    bounds.removeFromLeft(10);

    auto footer = bounds.removeFromBottom(24);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    juce::String footerText("RefLink ");
    footerText << (processor_.refLinkStatus() == ehvota::communication::SharedLinkOpenStatus::ready ? "Ready" : "Unavailable")
               << " · " << static_cast<int>(processor_.linkedInstanceCount()) << " linked Ehvota endpoints";
    g.drawText(footerText, footer, juce::Justification::centredLeft);
    bounds.removeFromBottom(8);
}

void RefHubAudioProcessorEditor::paintCompare(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto timeline = bounds.removeFromTop(168);
    drawPanel(g, timeline);
    auto tl = timeline.reduced(16, 10);
    auto labels = tl.removeFromTop(24);
    const std::array<const char*, 4> stages { "Current capture", "Reference window", "Level matched", "Review proposal" };
    for (const auto* label : stages) {
        auto cell = labels.removeFromLeft(labels.getWidth() / std::max(1, static_cast<int>(stages.size())));
        g.setColour(secondary);
        g.setFont(juce::FontOptions(9.5f));
        g.drawText(label, cell, juce::Justification::centred);
    }

    auto mixWave = tl.removeFromTop(42).toFloat().withTrimmedLeft(72.0f);
    auto refWave = tl.removeFromTop(42).toFloat().withTrimmedLeft(72.0f);
    g.setColour(cyan); g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("YOUR MIX", timeline.getX() + 12, static_cast<int>(mixWave.getY()), 66, 32, juce::Justification::centredLeft);
    g.setColour(violet);
    g.drawText("REFERENCE", timeline.getX() + 12, static_cast<int>(refWave.getY()), 66, 32, juce::Justification::centredLeft);
    drawWave(g, mixWave, cyan, 0.1f);
    drawWave(g, refWave, violet, 0.8f);

    bounds.removeFromTop(10);
    auto insight = bounds.removeFromBottom(126);
    bounds.removeFromBottom(10);
    drawPanel(g, insight);
    drawPanel(g, bounds);

    auto analyser = bounds.reduced(18, 12);
    auto tabs = analyser.removeFromTop(34);
    const std::array<const char*, 4> lenses { "TONE", "DYNAMICS", "SPACE", "COMPONENTS" };
    for (std::size_t i = 0; i < lenses.size(); ++i) {
        auto tab = tabs.removeFromLeft(118);
        g.setColour(i == 0 ? cyan : secondary);
        g.setFont(juce::FontOptions(10.5f).withStyle(i == 0 ? "Bold" : "Regular"));
        g.drawText(lenses[i], tab, juce::Justification::centred);
    }

    auto graph = analyser.reduced(0, 6).toFloat();
    g.setColour(border.withAlpha(0.7f));
    for (int i = 1; i < 10; ++i) {
        const auto x = graph.getX() + graph.getWidth() * static_cast<float>(i) / 10.0f;
        g.drawVerticalLine(static_cast<int>(x), graph.getY(), graph.getBottom());
    }
    for (int i = 1; i < 6; ++i) {
        const auto y = graph.getY() + graph.getHeight() * static_cast<float>(i) / 6.0f;
        g.drawHorizontalLine(static_cast<int>(y), graph.getX(), graph.getRight());
    }

    const auto snapshot = processor_.copyLatestSectionAnalysis();
    std::size_t strongest = 1;
    double strongestDifference = 0.0;
    bool haveAnalysis = snapshot && snapshot->valid && snapshot->comparison.valid;
    if (haveAnalysis) {
        drawSpectrumPath(g, snapshot->mix.spectrum, graph, cyan);
        drawSpectrumPath(g, snapshot->reference.spectrum, graph, violet);
        for (std::size_t i = 0; i < snapshot->comparison.bands.size(); ++i) {
            const auto magnitude = std::abs(snapshot->comparison.bands[i].differenceDb);
            if (magnitude > std::abs(strongestDifference)) {
                strongestDifference = snapshot->comparison.bands[i].differenceDb;
                strongest = i;
            }
        }
    } else {
        drawWave(g, graph.withHeight(graph.getHeight() * 0.75f), cyan, 0.1f);
        drawWave(g, graph.withTrimmedTop(graph.getHeight() * 0.12f).withHeight(graph.getHeight() * 0.70f), violet, 0.8f);
    }

    const auto& definitions = ehvota::analysis::defaultComparisonBands();
    const auto selected = definitions[std::min(strongest, definitions.size() - 1)];
    const auto selectedStart = spectrumX(selected.lowHz, graph);
    const auto selectedEnd = spectrumX(selected.highHz, graph);
    g.setColour(amber.withAlpha(haveAnalysis ? 0.10f : 0.04f));
    g.fillRect(juce::Rectangle<float>(selectedStart, graph.getY(), selectedEnd - selectedStart, graph.getHeight()));

    auto callout = juce::Rectangle<int>(static_cast<int>(selectedStart + 18), bounds.getY() + 72, 216, 112)
        .constrainedWithin(bounds.reduced(16));
    g.setColour(juce::Colour(0xee0b1016));
    g.fillRoundedRectangle(callout.toFloat(), 8.0f);
    g.setColour(haveAnalysis ? amber : border);
    g.drawRoundedRectangle(callout.toFloat().reduced(0.5f), 8.0f, 1.0f);
    auto co = callout.reduced(12, 8);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText(haveAnalysis ? juce::String(selected.name.data()) + " DIFFERENCE" : "WAITING FOR CAPTURE",
        co.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(text); g.setFont(juce::FontOptions(17.0f).withStyle("Bold"));
    g.drawText(juce::String(selected.lowHz, 0) + " – " + juce::String(selected.highHz, 0) + " Hz",
        co.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(haveAnalysis ? amber : secondary);
    const auto prefix = strongestDifference >= 0.0 ? "+" : "";
    g.drawText(haveAnalysis ? prefix + juce::String(strongestDifference, 1) + " dB" : "Capture → Analyse",
        co.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(secondary); g.setFont(juce::FontOptions(9.5f));
    g.drawText(haveAnalysis ? "Measured paired section" : "No measurement shown as fact", co, juce::Justification::centredLeft);

    auto ins = insight.reduced(18, 12);
    g.setColour(haveAnalysis ? amber : secondary);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("EHVOTA INSIGHT", ins.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(text); g.setFont(juce::FontOptions(17.0f).withStyle("Bold"));
    juce::String insightText = haveAnalysis
        ? juce::String(selected.name.data()) + (strongestDifference >= 0.0 ? " is above reference" : " is below reference")
        : "Capture a representative mix section to begin";
    g.drawText(insightText, ins.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(secondary); g.setFont(juce::FontOptions(10.5f));
    if (snapshot && snapshot->levelMatch.valid) {
        g.drawFittedText("Section Level Match: Mix " + juce::String(snapshot->levelMatch.mixLufs, 1)
            + " LUFS · Ref " + juce::String(snapshot->levelMatch.referenceLufs, 1)
            + " LUFS · Monitor trim " + juce::String(snapshot->levelMatch.referenceMonitorTrimDb, 1) + " dB. Processing remains unchanged.",
            ins, juce::Justification::topLeft, 3);
    } else {
        g.drawFittedText("Measurements and proposals stay separate. Nothing is corrected automatically; create and review a proposal after evidence exists.",
            ins, juce::Justification::topLeft, 3);
    }
}

void RefHubAudioProcessorEditor::paintSources(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto title = bounds.removeFromTop(36);
    g.setColour(text); g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("Reference stems ↔ live RefTap buses", title, juce::Justification::centredLeft);
    bounds.removeFromTop(8);
    auto footer = bounds.removeFromBottom(112);
    bounds.removeFromBottom(10);

    constexpr int gap = 10;
    const auto sideWidth = static_cast<int>(bounds.getWidth() * 0.34f);
    auto left = bounds.removeFromLeft(sideWidth); bounds.removeFromLeft(gap);
    auto centre = bounds.removeFromLeft(std::max(250, bounds.getWidth() / 2)); bounds.removeFromLeft(gap);
    auto right = bounds;
    drawPanel(g, left); drawPanel(g, centre); drawPanel(g, right); drawPanel(g, footer);

    auto l = left.reduced(14);
    g.setColour(text); g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText("REFERENCE SOURCES", l.removeFromTop(28), juce::Justification::centredLeft);
    if (loadedReference_.package.sources.empty()) {
        g.setColour(secondary); g.setFont(juce::FontOptions(10.5f));
        g.drawFittedText("Load a validated reference folder. The full mix is authoritative; stems are diagnostic inputs.", l, juce::Justification::topLeft, 4);
    } else {
        for (const auto& source : loadedReference_.package.sources) {
            if (l.getHeight() < 46) break;
            auto row = l.removeFromTop(46); drawPanel(g, row, true); l.removeFromTop(5);
            auto c = row.reduced(9, 5);
            g.setColour(source.role == ehvota::state::ReferenceRole::fullMix ? violet : text);
            g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
            g.drawText(roleName(source.role), c.removeFromTop(18), juce::Justification::centredLeft);
            g.setColour(secondary); g.setFont(juce::FontOptions(9.5f));
            g.drawText(juce::String(source.relativePath), c, juce::Justification::centredLeft);
        }
    }

    auto m = centre.reduced(12);
    g.setColour(text); g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText("EXPLICIT MAPPING", m.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(secondary); g.setFont(juce::FontOptions(9.5f));
    g.drawFittedText("Track names are identification hints only. Each semantic role is assigned deliberately and can be excluded from analysis.",
        m.removeFromTop(42), juce::Justification::topLeft, 3);
    for (std::size_t i = 0; i < componentRoles.size(); ++i) {
        auto row = m.removeFromTop(58);
        g.setColour(scopeToggles_[i].getToggleState() ? cyan : muted);
        g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
        g.drawText(roleName(componentRoles[i]), row.removeFromTop(18), juce::Justification::centredLeft);
        m.removeFromTop(3);
    }

    auto r = right.reduced(14);
    g.setColour(text); g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    g.drawText("LIVE REFTAP SOURCES", r.removeFromTop(28), juce::Justification::centredLeft);
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linked {};
    const auto count = processor_.copyLinkedInstances(linked);
    int shown = 0;
    for (std::size_t i = 0; i < count && shown < 7; ++i) {
        if (linked[i].pluginKind != ehvota::communication::SharedPluginKind::refTap) continue;
        auto row = r.removeFromTop(47); drawPanel(g, row, true); r.removeFromTop(5); ++shown;
        g.setColour(linked[i].heartbeatAgeMs <= 5000 ? green : amber);
        g.fillEllipse(static_cast<float>(row.getX() + 9), static_cast<float>(row.getCentreY() - 3), 6.0f, 6.0f);
        auto c = row.reduced(24, 5);
        g.setColour(text); g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
        g.drawText(snapshotName(linked[i]), c.removeFromTop(18), juce::Justification::centredLeft);
        g.setColour(secondary); g.setFont(juce::FontOptions(9.0f));
        g.drawText("RefTap " + juce::String(linked[i].id.slot) + ':' + juce::String(static_cast<int>(linked[i].id.generation)),
            c, juce::Justification::centredLeft);
    }
    if (shown == 0) {
        g.setColour(secondary); g.setFont(juce::FontOptions(10.0f));
        g.drawFittedText("No live RefTap endpoints yet. Put RefTap on the buses or tracks you want to compare.", r, juce::Justification::topLeft, 4);
    }

    auto f = footer.reduced(14, 10);
    g.setColour(amber); g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("MANUAL SECTION WINDOW · temporary until saved Sections are implemented", f.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(secondary); g.setFont(juce::FontOptions(9.5f));
    g.drawText("Live duration", f.getX(), f.getY(), 90, 20, juce::Justification::centredLeft);
    g.drawText("Reference start", f.getX() + 116, f.getY(), 100, 20, juce::Justification::centredLeft);
    g.drawText("Reference duration", f.getX() + 246, f.getY(), 120, 20, juce::Justification::centredLeft);
}

void RefHubAudioProcessorEditor::paintProcess(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    drawPanel(g, bounds);
    auto c = bounds.reduced(18);
    g.setColour(text); g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("LINKED EHVOTA PROCESSORS", c.removeFromTop(30), juce::Justification::centredLeft);
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linked {};
    const auto count = processor_.copyLinkedInstances(linked);
    for (std::size_t i = 0; i < count && c.getHeight() > 120; ++i) {
        if (linked[i].pluginKind == ehvota::communication::SharedPluginKind::refTap) continue;
        auto row = c.removeFromTop(44); drawPanel(g, row, true); c.removeFromTop(5);
        g.setColour(green); g.fillEllipse(static_cast<float>(row.getX() + 10), static_cast<float>(row.getCentreY() - 3), 6.0f, 6.0f);
        g.setColour(text); g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
        g.drawText(snapshotName(linked[i]) + " · " + juce::String(ehvota::communication::sharedPluginKindName(linked[i].pluginKind).data()),
            row.reduced(26, 0), juce::Justification::centredLeft);
    }
    const auto proposal = processor_.copyStagedProposal();
    auto status = c.removeFromBottom(78); drawPanel(g, status, true); status.reduce(12, 8);
    g.setColour(proposal ? amber : secondary); g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText(proposal ? "CURRENT PROPOSAL" : "NO PROPOSAL", status.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(text); g.setFont(juce::FontOptions(11.0f));
    if (proposal)
        g.drawFittedText(juce::String(ehvota::safety::proposalStateName(proposal->state).data()) + " · "
            + juce::String(static_cast<int>(proposal->changes.size())) + " bounded change(s) · evidence " + juce::String(proposal->evidenceId),
            status, juce::Justification::topLeft, 3);
    else
        g.drawText("Analyse a captured section first, then create a reviewable proposal.", status, juce::Justification::centredLeft);
}

void RefHubAudioProcessorEditor::paintHistory(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    drawPanel(g, bounds);
    auto c = bounds.reduced(18);
    g.setColour(text); g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("PROPOSAL HISTORY", c.removeFromTop(30), juce::Justification::centredLeft);
    const auto history = processor_.copyProposalHistory();
    if (history.empty()) {
        g.setColour(secondary); g.setFont(juce::FontOptions(10.5f));
        g.drawText("No applied Ehvota proposals yet.", c, juce::Justification::centred);
        return;
    }
    int shown = 0;
    for (auto it = history.rbegin(); it != history.rend() && shown < 9; ++it, ++shown) {
        auto row = c.removeFromTop(50); drawPanel(g, row, true); c.removeFromTop(5);
        auto rr = row.reduced(10, 5);
        g.setColour(text); g.setFont(juce::FontOptions(10.5f).withStyle("Bold"));
        g.drawText(juce::String(it->proposal.id), rr.removeFromTop(19), juce::Justification::centredLeft);
        g.setColour(secondary); g.setFont(juce::FontOptions(9.5f));
        g.drawText(juce::String(it->proposal.evidenceId), rr, juce::Justification::centredLeft);
    }
}

void RefHubAudioProcessorEditor::paintSettings(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    drawPanel(g, bounds);
    auto c = bounds.reduced(20);
    g.setColour(text); g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText("SAFETY + MONITORING", c.removeFromTop(32), juce::Justification::centredLeft);
    g.setColour(secondary); g.setFont(juce::FontOptions(10.5f));
    g.drawFittedText("Render Safe blocks reference/audition-only monitoring from offline renders. Level Match remains monitoring gain only and never changes the project audio or exported source files.",
        c.removeFromTop(76), juce::Justification::topLeft, 4);
    c.removeFromTop(26);
    g.setColour(text); g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText("RefLink status", c.removeFromTop(24), juce::Justification::centredLeft);
    g.setColour(processor_.refLinkStatus() == ehvota::communication::SharedLinkOpenStatus::ready ? green : amber);
    g.drawText(processor_.refLinkStatus() == ehvota::communication::SharedLinkOpenStatus::ready ? "Ready" : "Unavailable",
        c.removeFromTop(24), juce::Justification::centredLeft);
}

void RefHubAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    auto bounds = getLocalBounds().reduced(12);
    paintCommon(g, bounds);
    switch (page_) {
    case Page::compare: paintCompare(g, bounds); break;
    case Page::sources: paintSources(g, bounds); break;
    case Page::process: paintProcess(g, bounds); break;
    case Page::history: paintHistory(g, bounds); break;
    case Page::settings: paintSettings(g, bounds); break;
    }
}

void RefHubAudioProcessorEditor::resized()
{
    auto all = getLocalBounds().reduced(12);
    auto header = all.removeFromTop(62);
    auto rail = all.removeFromLeft(82);
    all.removeFromLeft(10);
    all.removeFromBottom(32);

    auto nav = rail.reduced(6, 14);
    for (auto* button : { &navCompare_, &navSources_, &navProcess_, &navHistory_ }) {
        button->setBounds(nav.removeFromTop(58).reduced(2, 5));
        nav.removeFromTop(8);
    }
    navSettings_.setBounds(nav.removeFromBottom(58).reduced(2, 5));

    if (page_ == Page::compare) {
        header.removeFromLeft(340);
        loadFolderButton_.setBounds(header.removeFromLeft(98).reduced(2, 10));
        referenceSelector_.setBounds(header.removeFromLeft(142).reduced(3, 10));
        sectionSelector_.setBounds(header.removeFromLeft(112).reduced(3, 10));
        mixButton_.setBounds(header.removeFromLeft(48).reduced(2, 10));
        referenceButton_.setBounds(header.removeFromLeft(48).reduced(2, 10));
        blindButton_.setBounds(header.removeFromLeft(58).reduced(2, 10));
        captureButton_.setBounds(header.removeFromLeft(72).reduced(2, 10));
        levelMatchSelector_.setBounds(header.removeFromLeft(120).reduced(2, 10));
        liveMeterLabel_.setBounds(header.reduced(4, 10));

        auto body = all;
        body.removeFromTop(168 + 10);
        auto insight = body.removeFromBottom(126);
        proposalButton_.setBounds(insight.reduced(18, 42).removeFromRight(170).reduced(2));
    } else if (page_ == Page::sources) {
        header.removeFromLeft(340);
        loadFolderButton_.setBounds(header.removeFromLeft(104).reduced(2, 10));
        auto body = all;
        body.removeFromTop(44);
        auto footer = body.removeFromBottom(112);
        body.removeFromBottom(10);
        constexpr int gap = 10;
        const auto sideWidth = static_cast<int>(body.getWidth() * 0.34f);
        body.removeFromLeft(sideWidth); body.removeFromLeft(gap);
        auto centre = body.removeFromLeft(std::max(250, body.getWidth() / 2));
        auto m = centre.reduced(12);
        m.removeFromTop(28 + 42);
        for (std::size_t i = 0; i < mappingSelectors_.size(); ++i) {
            auto row = m.removeFromTop(58);
            scopeToggles_[i].setBounds(row.removeFromLeft(100).reduced(2, 14));
            mappingSelectors_[i].setBounds(row.reduced(2, 10));
        }
        auto f = footer.reduced(14, 10);
        f.removeFromTop(18);
        liveDurationEditor_.setBounds(f.getX(), f.getY() + 22, 90, 28);
        referenceStartEditor_.setBounds(f.getX() + 116, f.getY() + 22, 100, 28);
        referenceDurationEditor_.setBounds(f.getX() + 246, f.getY() + 22, 110, 28);
        componentCaptureButton_.setBounds(f.getRight() - 190, f.getY() + 14, 188, 34);
        componentStatusLabel_.setBounds(f.getX(), f.getBottom() - 28, f.getWidth(), 24);
    } else if (page_ == Page::process) {
        auto body = all.reduced(18);
        auto actions = body.removeFromBottom(40);
        createProposalButton_.setBounds(actions.removeFromLeft(142).reduced(2));
        approveProposalButton_.setBounds(actions.removeFromLeft(142).reduced(2));
        rejectProposalButton_.setBounds(actions.removeFromLeft(92).reduced(2));
        undoButton_.setBounds(actions.removeFromLeft(108).reduced(2));
    } else if (page_ == Page::settings) {
        renderSafeToggle_.setBounds(all.getX() + 20, all.getY() + 145, 180, 32);
    }
}

void RefHubAudioProcessorEditor::timerCallback()
{
    liveMeterLabel_.setText(
        "Peak " + juce::String(processor_.livePeakDb(), 1) + " dBFS · RMS " + juce::String(processor_.liveRmsDb(), 1) + " dBFS",
        juce::dontSendNotification);

    if (referenceButton_.getToggleState() && !processor_.referenceAuditionActive()) {
        referenceButton_.setToggleState(false, juce::dontSendNotification);
        mixButton_.setToggleState(true, juce::dontSendNotification);
    }

    const auto captureState = processor_.mixCaptureState();
    if (captureState == ehvota::audio::CaptureState::complete)
        captureButton_.setButtonText("CAPTURED");
    else if (captureState == ehvota::audio::CaptureState::capturing)
        captureButton_.setButtonText("CAPTURING");
    else if (captureState != ehvota::audio::CaptureState::armed)
        captureButton_.setButtonText("CAPTURE");

    const auto component = processor_.copyComponentCaptureBatch();
    if (component.batchId != 0) {
        juce::String status = componentBatchName(component.state);
        int ready = 0;
        int requested = 0;
        for (const auto& role : component.roles) {
            if (role.requested) ++requested;
            if (role.comparisonReady) ++ready;
        }
        status << " · " << ready << '/' << requested << " component comparisons ready";
        componentStatusLabel_.setText(status, juce::dontSendNotification);
    }

    if (++refreshCounter_ >= 15) {
        refreshCounter_ = 0;
        refreshLinkedSources();
    }
    repaint();
}

} // namespace ehvota::refhub
