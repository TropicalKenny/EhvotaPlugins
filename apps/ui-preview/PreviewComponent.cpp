#include "PreviewComponent.h"

#include "shared/ui/Theme.h"
#include "shared/viewmodels/LayoutPolicy.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>

namespace ehvota::previewapp {
namespace {

constexpr int outerPadding = 12;
constexpr int headerHeight = 48;
constexpr int comparisonHeight = 48;
constexpr int footerHeight = 32;
constexpr int compactMenuWidth = 92;

int reviewSelectorWidth(const bool compact, const bool wide)
{
    if (compact)
        return 276;
    return wide ? 430 : 350;
}

const std::array<const char*, 14> refHubViews {
    "Overview", "Compare", "Library", "Mapping", "Sections", "Bands", "Dynamics",
    "Space", "Components", "Links", "Proposals", "History", "Timing", "Settings"
};

juce::String areaName(const preview::ProductArea area)
{
    using enum preview::ProductArea;
    switch (area) {
    case refHub:
        return "REFHUB";
    case refTime:
        return "REFTIME";
    case stemPrep:
        return "STEM PREP";
    }

    return "EHVOTA";
}

} // namespace

PreviewComponent::PreviewComponent()
{
    setWantsKeyboardFocus(true);
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);

    addAndMakeVisible(suiteShell_);
    suiteShell_.onAction = [this](const ui::ShellAction action) {
        handleShellAction(action);
    };

    addAndMakeVisible(scenarioSelector_);
    const auto& scenarios = preview::allScenarios();
    for (std::size_t index = 0; index < scenarios.size(); ++index) {
        scenarioSelector_.addItem(
            juce::String(scenarios[index].journey) + " — " + scenarios[index].title,
            static_cast<int>(index + 1));
    }
    scenarioSelector_.setTooltip("Select a deterministic workflow or required state.");
    scenarioSelector_.onChange = [this] {
        loadScenario(scenarioSelector_.getSelectedItemIndex());
    };

    addAndMakeVisible(comparisonContextLabel_);
    comparisonContextLabel_.setJustificationType(juce::Justification::centredLeft);
    comparisonContextLabel_.setFont(ui::font(14.0f, true));
    comparisonContextLabel_.setColour(
        juce::Label::textColourId,
        juce::Colour(ui::colour::textPrimary));
    comparisonContextLabel_.setMinimumHorizontalScale(0.86f);

    for (auto* button : { &menuButton_, &mixButton_, &referenceButton_, &loudnessMatchButton_,
             &captureButton_, &blindButton_, &primaryActionButton_, &secondaryActionButton_,
             &journeyBackButton_, &journeyNextButton_ }) {
        addAndMakeVisible(*button);
        button->setWantsKeyboardFocus(true);
    }

    menuButton_.setTooltip("Open the labelled view menu.");
    menuButton_.onClick = [this] {
        juce::PopupMenu menu;
        for (std::size_t index = 0; index < currentViewNames_.size(); ++index) {
            const auto itemId = static_cast<int>(index + 1);
            menu.addItem(
                itemId,
                currentViewNames_[index],
                true,
                scenario_ != nullptr
                    && currentViewNames_[index] == juce::String(scenario_->activeView));
        }

        const juce::Component::SafePointer<PreviewComponent> safeThis(this);
        menu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&menuButton_),
            [safeThis](const int result) {
                if (safeThis == nullptr || result <= 0)
                    return;
                const auto index = static_cast<std::size_t>(result - 1);
                if (index < safeThis->currentViewNames_.size())
                    safeThis->setTransientStatus("Navigation preview: " + safeThis->currentViewNames_[index]);
            });
    };

    mixButton_.setClickingTogglesState(true);
    referenceButton_.setClickingTogglesState(true);
    loudnessMatchButton_.setClickingTogglesState(true);
    mixButton_.setToggleState(true, juce::dontSendNotification);
    loudnessMatchButton_.setToggleState(true, juce::dontSendNotification);

    mixButton_.setTooltip("Audition the live mix.");
    referenceButton_.setTooltip("Audition the loaded reference. Reference audio never enters export.");
    loudnessMatchButton_.setTooltip("Toggle audition-only loudness matching.");
    captureButton_.setTooltip("Start or inspect Capture Analysis.");
    blindButton_.setTooltip("Begin a blind comparison.");
    journeyBackButton_.setTooltip("Show the previous deterministic journey step. Ctrl+Alt+Left.");
    journeyNextButton_.setTooltip("Show the next deterministic journey step. Ctrl+Alt+Right.");

    mixButton_.onClick = [this] {
        referenceButton_.setToggleState(false, juce::dontSendNotification);
        setTransientStatus("Audition source: Mix");
    };
    referenceButton_.onClick = [this] {
        mixButton_.setToggleState(false, juce::dontSendNotification);
        referenceButton_.setToggleState(true, juce::dontSendNotification);
        setTransientStatus("Audition source: Reference · excluded from render");
    };
    loudnessMatchButton_.onClick = [this] {
        setTransientStatus(loudnessMatchButton_.getToggleState()
                ? "Loudness match enabled · audition only"
                : "Loudness match disabled");
    };
    captureButton_.onClick = [this] { setTransientStatus("Capture action previewed · deterministic data only"); };
    blindButton_.onClick = [this] { setTransientStatus("Blind comparison previewed"); };
    primaryActionButton_.onClick = [this] { setTransientStatus(primaryActionButton_.getButtonText() + " action previewed"); };
    secondaryActionButton_.onClick = [this] { setTransientStatus(secondaryActionButton_.getButtonText() + " action previewed"); };
    journeyBackButton_.onClick = [this] { moveJourneyStep(false); };
    journeyNextButton_.onClick = [this] { moveJourneyStep(true); };

    addAndMakeVisible(signalGraph_);
    addAndMakeVisible(timingReadout_);
    addAndMakeVisible(progressTimeline_);
    addAndMakeVisible(journeyProgress_);
    addAndMakeVisible(panelsViewport_);
    journeyBackButton_.toFront(false);
    journeyNextButton_.toFront(false);
    panelsViewport_.setViewedComponent(&panelsContent_, false);
    panelsViewport_.setScrollBarsShown(true, false);
    panelsViewport_.setTitle("Scenario details");
    panelsViewport_.setDescription("Scrollable measurement and state panels for the selected deterministic scenario.");
    scenarioSelector_.setSelectedId(1, juce::sendNotificationSync);
}

bool PreviewComponent::selectScenarioById(const std::string_view scenarioId)
{
    const auto& scenarios = preview::allScenarios();
    const auto match = std::find_if(
        scenarios.begin(),
        scenarios.end(),
        [scenarioId](const preview::Scenario& candidate) {
            return candidate.id == scenarioId;
        });

    if (match == scenarios.end())
        return false;

    const auto index = static_cast<int>(std::distance(scenarios.begin(), match));
    scenarioSelector_.setSelectedId(index + 1, juce::sendNotificationSync);
    return scenario_ == &*match;
}

bool PreviewComponent::selectJourneyStep(const std::size_t oneBasedStep)
{
    if (!journeyReplay_.setStepNumber(oneBasedStep))
        return false;

    updateJourneyReplay();
    return true;
}

void PreviewComponent::loadScenario(const int scenarioIndex)
{
    const auto& scenarios = preview::allScenarios();
    if (scenarios.empty())
        return;

    scenarioIndex_ = juce::jlimit(0, static_cast<int>(scenarios.size() - 1), scenarioIndex);
    scenario_ = &scenarios[static_cast<std::size_t>(scenarioIndex_)];
    signalGraph_.setScenario(scenario_->id);
    static_cast<void>(journeyReplay_.selectForScenario(scenario_->id));
    transientStatus_ = "Deterministic preview data · No audio is processed";
    const auto comparisonContext = juce::String(scenario_->reference)
        + "  ·  " + juce::String(scenario_->sectionPair);
    comparisonContextLabel_.setText(comparisonContext, juce::dontSendNotification);
    comparisonContextLabel_.setTooltip(comparisonContext);
    comparisonContextLabel_.setTitle("Reference and section context");
    comparisonContextLabel_.setDescription(comparisonContext);
    setTitle(areaName(scenario_->area) + " preview: " + juce::String(scenario_->title));
    setDescription(
        juce::String(scenario_->description) + ". Reference context: " + comparisonContext);
    shellComparisonB_ = false;
    shellProcessingActive_ = scenario_->id != "render-safe";
    shellCanUndo_ = scenario_->id == "proposal-applied";
    shellCanRedo_ = false;
    shellDefaultPresetName_ = scenario_->area == preview::ProductArea::refTime
        ? "Host Timing"
        : scenario_->area == preview::ProductArea::stemPrep
            ? "Balanced 4-Stem"
            : scenario_->reference;
    shellPresetName_ = shellDefaultPresetName_;

    if (scenario_->id == "proposal-review") {
        primaryActionButton_.setButtonText("APPLY");
        secondaryActionButton_.setButtonText("REJECT");
    }
    else if (scenario_->id == "stem-prep-queue") {
        primaryActionButton_.setButtonText("CANCEL SAFELY");
        secondaryActionButton_.setButtonText("OPEN FOLDER");
    }
    else if (scenario_->id == "reference-ready") {
        primaryActionButton_.setButtonText("LOAD REFERENCE");
        secondaryActionButton_.setButtonText("VALIDATION");
    }
    else if (scenario_->id == "reftime-120") {
        primaryActionButton_.setButtonText("COPY EXACT MS");
        secondaryActionButton_.setButtonText("COPY SAMPLES");
    }
    else {
        primaryActionButton_.setButtonText("OPEN");
        secondaryActionButton_.setButtonText("DETAILS");
    }

    menuButton_.setButtonText("VIEW · " + juce::String(scenario_->activeView));

    if (scenario_->area == preview::ProductArea::refTime && !scenario_->panels.empty()) {
        const auto& timingPanel = scenario_->panels.front();
        juce::String secondary;
        for (const auto& metric : timingPanel.metrics) {
            if (secondary.isNotEmpty())
                secondary += " · ";
            secondary += juce::String(metric.label) + " " + juce::String(metric.value);
        }
        timingReadout_.setValues(juce::String(timingPanel.summary), secondary);
    }

    if (scenario_->area == preview::ProductArea::stemPrep) {
        if (scenario_->id == "stem-prep-queue")
            progressTimeline_.setState("Ayo Nights", "Separating locally", 0.62);
        else if (scenario_->id == "stem-prep-cancelled")
            progressTimeline_.setState("River Glass", "Cancelled safely", 0.0);
        else
            progressTimeline_.setState(juce::String(scenario_->title), "Blocked · local repair required", 0.0);
    }

    rebuildScenarioComponents();
    updateNavigation();
    updateJourneyReplay();
    updateShellPresentation();
    resized();
    repaint();
}

void PreviewComponent::rebuildScenarioComponents()
{
    badges_.clear();
    panels_.clear();

    if (scenario_ == nullptr)
        return;

    for (const auto& badge : scenario_->badges) {
        auto component = std::make_unique<ui::StatusBadge>(badge);
        addAndMakeVisible(*component);
        badges_.push_back(std::move(component));
    }

    for (const auto& panel : scenario_->panels) {
        auto component = std::make_unique<ui::MetricPanel>(panel);
        panelsContent_.addAndMakeVisible(*component);
        panels_.push_back(std::move(component));
    }
}

void PreviewComponent::updateNavigation()
{
    navigationButtons_.clear();
    if (scenario_ == nullptr)
        return;

    currentViewNames_.clear();
    if (scenario_->area == preview::ProductArea::refHub) {
        for (const auto* view : refHubViews)
            currentViewNames_.emplace_back(view);
    }
    else if (scenario_->area == preview::ProductArea::refTime) {
        currentViewNames_ = { "Timing", "Table", "Favourites", "Settings" };
    }
    else {
        currentViewNames_ = { "Queue", "References", "Completed", "Models", "Settings" };
    }

    for (const auto& view : currentViewNames_) {
        auto button = std::make_unique<juce::TextButton>(view);
        button->setClickingTogglesState(true);
        button->setRadioGroupId(42, juce::dontSendNotification);
        button->setToggleState(
            view == juce::String(scenario_->activeView),
            juce::dontSendNotification);
        button->setTooltip("Open " + view);
        button->onClick = [this, view] { setTransientStatus("Navigation preview: " + view); };
        addAndMakeVisible(*button);
        navigationButtons_.push_back(std::move(button));
    }
}

void PreviewComponent::updateJourneyReplay()
{
    const auto* journey = journeyReplay_.journey();
    const auto* step = journeyReplay_.currentStep();
    const auto isAvailable = journey != nullptr && step != nullptr;

    journeyProgress_.setVisible(isAvailable);
    journeyBackButton_.setVisible(isAvailable);
    journeyNextButton_.setVisible(isAvailable);

    if (!isAvailable)
        return;

    journeyProgress_.setStep(
        journey->number,
        juce::String(journey->title),
        static_cast<int>(journeyReplay_.currentStepNumber()),
        static_cast<int>(journeyReplay_.totalSteps()),
        juce::String(step->title),
        juce::String(step->instruction));
    journeyBackButton_.setEnabled(journeyReplay_.canGoPrevious());
    journeyNextButton_.setEnabled(journeyReplay_.canGoNext());
}

void PreviewComponent::updateShellPresentation()
{
    if (scenario_ == nullptr)
        return;

    preview::ShellPresentation presentation;
    presentation.productName = areaName(scenario_->area).toStdString();
    presentation.presetName = shellPresetName_;
    presentation.canUndo = shellCanUndo_;
    presentation.canRedo = shellCanRedo_;
    presentation.comparisonB = shellComparisonB_;
    presentation.processingActive = shellProcessingActive_;
    presentation.scalePercent = shellScalePercent_;

    using enum preview::ProductArea;
    switch (scenario_->area) {
    case refHub:
        presentation.processingActionLabel = "MONITOR";
        presentation.showProcessingAction = true;
        presentation.showOutputMeter = true;
        presentation.outputLevelDb = scenario_->id == "render-safe" ? -60.0 : -8.4;
        if (scenario_->id == "render-safe") {
            presentation.processingActionEnabled = false;
            presentation.statusText = "RENDER SAFE";
            presentation.statusDetail = "Monitoring and reference audition are excluded from export.";
        }
        else if (scenario_->id == "mapping-disconnected"
            || scenario_->id == "bridge-unsupported"
            || scenario_->id == "high-cpu"
            || scenario_->id == "links-stale"
            || scenario_->id == "capture-failed"
            || scenario_->id == "reference-invalid-length") {
            presentation.statusText = "ATTENTION";
            presentation.statusDetail = scenario_->description;
            presentation.statusIsWarning = true;
        }
        else {
            presentation.statusText = "LINKS LOCAL";
            presentation.statusDetail = "Same-process preview status. No network transport.";
        }
        break;
    case refTime:
        presentation.processingActionLabel = "BYPASS";
        presentation.showProcessingAction = true;
        presentation.showOutputMeter = true;
        presentation.outputLevelDb = -12.0;
        presentation.statusText = "HOST SYNC";
        presentation.statusDetail = "120 BPM · 4/4 · technical latency 0 samples.";
        break;
    case stemPrep:
        presentation.showProcessingAction = false;
        presentation.showOutputMeter = false;
        presentation.statusText = scenario_->id == "stem-prep-queue" ? "LOCAL ACTIVE" : "LOCAL ONLY";
        presentation.statusDetail = "Standalone offline workflow. No audio leaves the computer.";
        presentation.statusIsWarning = scenario_->id == "model-missing"
            || scenario_->id == "model-license-error"
            || scenario_->id == "installer-repair";
        break;
    }

    suiteShell_.setPresentation(std::move(presentation));
}

void PreviewComponent::handleShellAction(const ui::ShellAction action)
{
    using enum ui::ShellAction;
    switch (action) {
    case selectPreset:
        shellPresetName_ = suiteShell_.selectedPresetName().toStdString();
        setTransientStatus(
            "Preset selected for preview: " + juce::String(shellPresetName_)
            + " · no project state written");
        break;
    case previousPreset:
        moveShellPreset(false);
        break;
    case nextPreset:
        moveShellPreset(true);
        break;
    case savePreset:
        setTransientStatus("Preset save previewed · no file written");
        return;
    case undo:
        if (!shellCanUndo_) {
            setTransientStatus("Nothing to undo in this deterministic state");
            return;
        }
        shellCanUndo_ = false;
        shellCanRedo_ = true;
        setTransientStatus("Scoped UI transaction undone · preview only");
        break;
    case redo:
        if (!shellCanRedo_) {
            setTransientStatus("Nothing to redo in this deterministic state");
            return;
        }
        shellCanUndo_ = true;
        shellCanRedo_ = false;
        setTransientStatus("Scoped UI transaction redone · preview only");
        break;
    case toggleComparison:
        shellComparisonB_ = !shellComparisonB_;
        setTransientStatus(
            shellComparisonB_ ? "Shell comparison state: B" : "Shell comparison state: A");
        break;
    case toggleProcessing:
        shellProcessingActive_ = !shellProcessingActive_;
        setTransientStatus(
            juce::String(scenario_ != nullptr
                    && scenario_->area == preview::ProductArea::refHub
                    ? "Monitoring "
                    : "Processing ")
            + (shellProcessingActive_ ? "enabled" : "bypassed")
            + " · deterministic preview only");
        break;
    case help:
        setTransientStatus(
            "Context help: "
            + juce::String(scenario_ != nullptr ? scenario_->journey : ""));
        return;
    case cycleScale: {
        constexpr std::array scaleValues { 75, 100, 125, 150, 175, 200 };
        const auto current = std::find(
            scaleValues.begin(),
            scaleValues.end(),
            shellScalePercent_);
        shellScalePercent_ = current == scaleValues.end()
                || std::next(current) == scaleValues.end()
            ? scaleValues.front()
            : *std::next(current);
        setTransientStatus(
            "Interface scale selection: " + juce::String(shellScalePercent_)
            + "% · preview token only; Windows DPI is unchanged");
        break;
    }
    }

    updateShellPresentation();
}

void PreviewComponent::moveShellPreset(const bool forward)
{
    std::vector<std::string> choices;
    for (const auto& candidate : std::array {
             shellDefaultPresetName_,
             std::string("Default"),
             std::string("Review A"),
             std::string("Review B") }) {
        if (std::find(choices.begin(), choices.end(), candidate) == choices.end())
            choices.push_back(candidate);
    }

    const auto current = std::find(
        choices.begin(),
        choices.end(),
        shellPresetName_);
    auto index = current == choices.end()
        ? std::size_t {}
        : static_cast<std::size_t>(std::distance(choices.begin(), current));
    if (forward)
        index = (index + 1) % choices.size();
    else
        index = index == 0 ? choices.size() - 1 : index - 1;

    shellPresetName_ = choices[index];
    setTransientStatus(
        "Preset preview: " + juce::String(shellPresetName_)
        + " · deterministic state only");
}

void PreviewComponent::moveJourneyStep(const bool forward)
{
    const auto moved = forward ? journeyReplay_.next() : journeyReplay_.previous();
    if (!moved)
        return;

    updateJourneyReplay();
    const auto* step = journeyReplay_.currentStep();
    setTransientStatus(
        "Journey replay · Step " + juce::String(static_cast<int>(journeyReplay_.currentStepNumber()))
        + " of " + juce::String(static_cast<int>(journeyReplay_.totalSteps()))
        + (step != nullptr ? " · " + juce::String(step->title) : juce::String {}));
}

void PreviewComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(ui::colour::canvas));
    if (scenario_ == nullptr)
        return;

    auto bounds = getLocalBounds().reduced(outerPadding);
    bounds.removeFromTop(headerHeight);

    auto comparison = bounds.removeFromTop(comparisonHeight);
    graphics.setColour(juce::Colour(ui::colour::base));
    graphics.fillRoundedRectangle(comparison.toFloat(), 8.0f);
    graphics.setColour(juce::Colour(ui::colour::borderSubtle));
    graphics.drawRoundedRectangle(comparison.toFloat().reduced(0.5f), 8.0f, 1.0f);

    bounds.removeFromTop(10);
    auto footer = bounds.removeFromBottom(footerHeight);
    graphics.setColour(juce::Colour(ui::colour::base));
    graphics.fillRoundedRectangle(footer.toFloat(), 6.0f);
    graphics.setColour(juce::Colour(ui::colour::textSecondary));
    graphics.setFont(ui::font(11.0f));
    auto footerText = footer.reduced(10, 0);
    footerText.removeFromRight(
        reviewSelectorWidth(isCompact(), isWide()) + 6);
    if (isCompact())
        footerText.removeFromLeft(compactMenuWidth + 6);
    graphics.drawFittedText(
        transientStatus_,
        footerText,
        juce::Justification::centredLeft,
        1);

    auto workspace = bounds;
    if (!isCompact())
        workspace.removeFromLeft(160 + 12);
    if (isWide())
        workspace.removeFromRight(246 + 12);

    auto contentHeading = workspace.removeFromTop(72);
    graphics.setColour(juce::Colour(ui::colour::textPrimary));
    graphics.setFont(ui::font(20.0f, true));
    graphics.drawFittedText(scenario_->title, contentHeading.removeFromTop(30), juce::Justification::centredLeft, 1);
    graphics.setColour(juce::Colour(ui::colour::textSecondary));
    graphics.setFont(ui::font(12.0f));
    graphics.drawFittedText(scenario_->description, contentHeading, juce::Justification::topLeft, 2);

    if (isWide()) {
        auto inspector = bounds.removeFromRight(246);
        graphics.setColour(juce::Colour(ui::colour::raised));
        graphics.fillRoundedRectangle(inspector.toFloat(), 8.0f);
        graphics.setColour(juce::Colour(ui::colour::borderSubtle));
        graphics.drawRoundedRectangle(inspector.toFloat().reduced(0.5f), 8.0f, 1.0f);

        auto inspectorContent = inspector.reduced(14);
        graphics.setColour(juce::Colour(ui::colour::textPrimary));
        graphics.setFont(ui::font(14.0f, true));
        graphics.drawFittedText(
            scenario_->inspectorTitle,
            inspectorContent.removeFromTop(42),
            juce::Justification::topLeft,
            2);

        graphics.setColour(juce::Colour(ui::colour::textSecondary));
        graphics.setFont(ui::font(12.0f));
        for (const auto& line : scenario_->inspectorLines) {
            auto lineArea = inspectorContent.removeFromTop(42);
            graphics.drawFittedText("• " + juce::String(line), lineArea, juce::Justification::topLeft, 2);
        }
    }
}

void PreviewComponent::resized()
{
    if (scenario_ == nullptr || getWidth() <= 0 || getHeight() <= 0)
        return;

    auto bounds = getLocalBounds().reduced(outerPadding);
    auto header = bounds.removeFromTop(headerHeight);
    suiteShell_.setBounds(header);

    auto comparison = bounds.removeFromTop(comparisonHeight).reduced(8, 7);
    const auto showComparisonControls = scenario_->area == preview::ProductArea::refHub;
    mixButton_.setVisible(showComparisonControls);
    referenceButton_.setVisible(showComparisonControls);
    loudnessMatchButton_.setVisible(showComparisonControls);
    captureButton_.setVisible(showComparisonControls);
    blindButton_.setVisible(showComparisonControls);

    if (showComparisonControls) {
        auto controls = comparison.removeFromRight(isCompact() ? 328 : 470);
        mixButton_.setBounds(controls.removeFromLeft(isCompact() ? 46 : 56));
        controls.removeFromLeft(4);
        referenceButton_.setBounds(controls.removeFromLeft(isCompact() ? 74 : 90));
        controls.removeFromLeft(isCompact() ? 4 : 6);
        loudnessMatchButton_.setBounds(controls.removeFromLeft(isCompact() ? 62 : 72));
        controls.removeFromLeft(isCompact() ? 4 : 6);
        captureButton_.setBounds(controls.removeFromLeft(isCompact() ? 66 : 78));
        controls.removeFromLeft(isCompact() ? 4 : 6);
        blindButton_.setBounds(controls.removeFromLeft(isCompact() ? 54 : 64));
    }
    comparisonContextLabel_.setBounds(comparison.reduced(6, 0));

    bounds.removeFromTop(10);
    auto footer = bounds.removeFromBottom(footerHeight);
    auto selectorArea = footer.removeFromRight(
        reviewSelectorWidth(isCompact(), isWide()));
    scenarioSelector_.setBounds(selectorArea.reduced(0, 3));
    menuButton_.setVisible(isCompact());
    if (isCompact())
        menuButton_.setBounds(footer.removeFromLeft(compactMenuWidth).reduced(0, 3));
    else
        menuButton_.setBounds({});
    bounds.removeFromBottom(46);

    if (!isCompact()) {
        auto navigation = bounds.removeFromLeft(160);
        for (auto& button : navigationButtons_) {
            button->setVisible(true);
            button->setBounds(navigation.removeFromTop(30));
            navigation.removeFromTop(4);
        }
        bounds.removeFromLeft(12);
    }
    else {
        for (auto& button : navigationButtons_)
            button->setVisible(false);
    }

    if (isWide())
        bounds.removeFromRight(246 + 12);

    bounds.removeFromTop(72);
    if (journeyProgress_.isVisible()) {
        auto journeyRow = bounds.removeFromTop(68);
        journeyProgress_.setBounds(journeyRow);

        auto journeyControls = journeyRow.removeFromRight(150).reduced(8, 16);
        journeyBackButton_.setBounds(journeyControls.removeFromLeft(64));
        journeyControls.removeFromLeft(6);
        journeyNextButton_.setBounds(journeyControls);
        bounds.removeFromTop(10);
    }
    else {
        journeyProgress_.setBounds({});
        journeyBackButton_.setBounds({});
        journeyNextButton_.setBounds({});
    }

    auto badgeRow = bounds.removeFromTop(28);
    for (auto& badge : badges_) {
        const auto width = juce::jmin(badge->idealWidth(), badgeRow.getWidth());
        badge->setBounds(badgeRow.removeFromLeft(width));
        badgeRow.removeFromLeft(6);
    }
    bounds.removeFromTop(10);

    const auto showSignalGraph = scenario_->area == preview::ProductArea::refHub;
    const auto showTimingReadout = scenario_->area == preview::ProductArea::refTime;
    const auto showProgressTimeline = scenario_->area == preview::ProductArea::stemPrep;
    signalGraph_.setVisible(showSignalGraph);
    timingReadout_.setVisible(showTimingReadout);
    progressTimeline_.setVisible(showProgressTimeline);

    auto heroBounds = bounds.removeFromTop(130);
    if (showSignalGraph)
        signalGraph_.setBounds(heroBounds);
    else if (showTimingReadout)
        timingReadout_.setBounds(heroBounds);
    else if (showProgressTimeline)
        progressTimeline_.setBounds(heroBounds);
    bounds.removeFromTop(10);

    panelsViewport_.setBounds(bounds);
    const auto viewportWidth = juce::jmax(1, panelsViewport_.getMaximumVisibleWidth());
    const auto columns = viewportWidth >= 720 ? 2 : 1;
    const auto gap = 10;
    const auto columnWidth = columns == 2 ? (viewportWidth - gap) / 2 : viewportWidth;
    std::array<int, 2> yPositions { 0, 0 };

    for (std::size_t index = 0; index < panels_.size(); ++index) {
        const auto column = columns == 2 ? static_cast<int>(index % 2) : 0;
        const auto x = column * (columnWidth + gap);
        const auto height = panels_[index]->preferredHeight();
        panels_[index]->setBounds(x, yPositions[static_cast<std::size_t>(column)], columnWidth, height);
        yPositions[static_cast<std::size_t>(column)] += height + gap;
    }
    const auto contentHeight = juce::jmax(
        panelsViewport_.getHeight(),
        juce::jmax(yPositions[0], yPositions[1]));
    panelsContent_.setSize(viewportWidth, contentHeight);

    auto actions = getLocalBounds().reduced(outerPadding);
    actions.removeFromTop(headerHeight + comparisonHeight + 10);
    actions.removeFromBottom(footerHeight + 8);
    if (!isCompact())
        actions.removeFromLeft(160 + 12);
    if (isWide())
        actions.removeFromRight(246 + 12);
    auto actionRow = actions.removeFromBottom(36);
    secondaryActionButton_.setBounds(actionRow.removeFromRight(122));
    actionRow.removeFromRight(8);
    primaryActionButton_.setBounds(actionRow.removeFromRight(148));
}

bool PreviewComponent::keyPressed(const juce::KeyPress& key)
{
    if (key.getModifiers().isAltDown()
        && key.getModifiers().isCtrlDown()
        && key.getKeyCode() == juce::KeyPress::leftKey) {
        moveJourneyStep(false);
        return true;
    }

    if (key.getModifiers().isAltDown()
        && key.getModifiers().isCtrlDown()
        && key.getKeyCode() == juce::KeyPress::rightKey) {
        moveJourneyStep(true);
        return true;
    }

    if (key == juce::KeyPress::F1Key) {
        handleShellAction(ui::ShellAction::help);
        return true;
    }

    if (key.getModifiers().isCtrlDown() && key.getKeyCode() == 'P') {
        suiteShell_.showPresetMenu();
        return true;
    }

    if (key.getModifiers().isCtrlDown() && key.getKeyCode() == 'S') {
        handleShellAction(ui::ShellAction::savePreset);
        return true;
    }

    if (key.getModifiers().isCtrlDown() && key.getKeyCode() == 'Z') {
        handleShellAction(
            key.getModifiers().isShiftDown()
                ? ui::ShellAction::redo
                : ui::ShellAction::undo);
        return true;
    }

    return false;
}

void PreviewComponent::setTransientStatus(juce::String status)
{
    transientStatus_ = std::move(status);
    repaint();
}

bool PreviewComponent::isCompact() const
{
    return preview::classifyLayout(getWidth()) == preview::LayoutClass::compact;
}

bool PreviewComponent::isWide() const
{
    return preview::classifyLayout(getWidth()) == preview::LayoutClass::wide;
}

} // namespace ehvota::previewapp
