#include "PreviewComponent.h"

#include "shared/ui/Theme.h"

#include <array>

namespace ehvota::previewapp {
namespace {

constexpr int outerPadding = 12;
constexpr int headerHeight = 48;
constexpr int comparisonHeight = 48;
constexpr int footerHeight = 32;

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

    for (auto* button : { &mixButton_, &referenceButton_, &loudnessMatchButton_,
             &captureButton_, &blindButton_, &primaryActionButton_, &secondaryActionButton_ }) {
        addAndMakeVisible(*button);
        button->setWantsKeyboardFocus(true);
    }

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

    addAndMakeVisible(signalGraph_);
    scenarioSelector_.setSelectedId(1, juce::sendNotificationSync);
}

void PreviewComponent::loadScenario(const int scenarioIndex)
{
    const auto& scenarios = preview::allScenarios();
    if (scenarios.empty())
        return;

    scenarioIndex_ = juce::jlimit(0, static_cast<int>(scenarios.size() - 1), scenarioIndex);
    scenario_ = &scenarios[static_cast<std::size_t>(scenarioIndex_)];
    signalGraph_.setScenario(scenario_->id);
    transientStatus_ = "Deterministic preview data · No audio is processed";

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

    const auto refHub = scenario_->area == preview::ProductArea::refHub;
    mixButton_.setVisible(refHub);
    referenceButton_.setVisible(refHub);
    loudnessMatchButton_.setVisible(refHub);
    captureButton_.setVisible(refHub);
    blindButton_.setVisible(refHub);

    rebuildScenarioComponents();
    updateNavigation();
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
        addAndMakeVisible(*component);
        panels_.push_back(std::move(component));
    }
}

void PreviewComponent::updateNavigation()
{
    navigationButtons_.clear();
    if (scenario_ == nullptr)
        return;

    std::vector<juce::String> views;
    if (scenario_->area == preview::ProductArea::refHub) {
        for (const auto* view : refHubViews)
            views.emplace_back(view);
    }
    else if (scenario_->area == preview::ProductArea::refTime) {
        views = { "Timing", "Table", "Favourites", "Settings" };
    }
    else {
        views = { "Queue", "References", "Completed", "Models", "Settings" };
    }

    for (const auto& view : views) {
        auto button = std::make_unique<juce::TextButton>(view);
        button->setClickingTogglesState(true);
        button->setRadioGroupId(42, juce::dontSendNotification);
        button->setToggleState(view == scenario_->activeView, juce::dontSendNotification);
        button->setTooltip("Open " + view);
        button->onClick = [this, view] { setTransientStatus("Navigation preview: " + view); };
        addAndMakeVisible(*button);
        navigationButtons_.push_back(std::move(button));
    }
}

void PreviewComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(ui::colour::canvas));
    if (scenario_ == nullptr)
        return;

    auto bounds = getLocalBounds().reduced(outerPadding);
    auto header = bounds.removeFromTop(headerHeight);

    graphics.setColour(juce::Colour(ui::colour::textMuted));
    graphics.setFont(ui::font(11.0f, true));
    graphics.drawText("EHVOTA / " + areaName(scenario_->area),
        header.removeFromLeft(190), juce::Justification::centredLeft);

    auto comparison = bounds.removeFromTop(comparisonHeight);
    graphics.setColour(juce::Colour(ui::colour::base));
    graphics.fillRoundedRectangle(comparison.toFloat(), 8.0f);
    graphics.setColour(juce::Colour(ui::colour::borderSubtle));
    graphics.drawRoundedRectangle(comparison.toFloat().reduced(0.5f), 8.0f, 1.0f);

    auto comparisonText = comparison.reduced(14, 0);
    if (!isCompact())
        comparisonText.removeFromRight(480);
    graphics.setColour(juce::Colour(ui::colour::textPrimary));
    graphics.setFont(ui::font(14.0f, true));
    graphics.drawFittedText(
        juce::String(scenario_->reference) + "  ·  " + scenario_->sectionPair,
        comparisonText,
        juce::Justification::centredLeft,
        1);

    bounds.removeFromTop(10);
    auto footer = bounds.removeFromBottom(footerHeight);
    graphics.setColour(juce::Colour(ui::colour::base));
    graphics.fillRoundedRectangle(footer.toFloat(), 6.0f);
    graphics.setColour(juce::Colour(ui::colour::textSecondary));
    graphics.setFont(ui::font(11.0f));
    graphics.drawFittedText(transientStatus_, footer.reduced(10, 0), juce::Justification::centredLeft, 1);

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
    if (scenario_ == nullptr)
        return;

    auto bounds = getLocalBounds().reduced(outerPadding);
    auto header = bounds.removeFromTop(headerHeight);
    header.removeFromLeft(190);
    scenarioSelector_.setBounds(header.reduced(0, 6));

    auto comparison = bounds.removeFromTop(comparisonHeight).reduced(8, 7);
    const auto showComparisonControls = scenario_->area == preview::ProductArea::refHub && !isCompact();
    mixButton_.setVisible(showComparisonControls);
    referenceButton_.setVisible(showComparisonControls);
    loudnessMatchButton_.setVisible(showComparisonControls);
    captureButton_.setVisible(showComparisonControls);
    blindButton_.setVisible(showComparisonControls);

    if (showComparisonControls) {
        auto controls = comparison.removeFromRight(470);
        mixButton_.setBounds(controls.removeFromLeft(56));
        controls.removeFromLeft(4);
        referenceButton_.setBounds(controls.removeFromLeft(90));
        controls.removeFromLeft(6);
        loudnessMatchButton_.setBounds(controls.removeFromLeft(72));
        controls.removeFromLeft(6);
        captureButton_.setBounds(controls.removeFromLeft(78));
        controls.removeFromLeft(6);
        blindButton_.setBounds(controls.removeFromLeft(64));
    }

    bounds.removeFromTop(10);
    bounds.removeFromBottom(footerHeight);
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

    auto badgeRow = bounds.removeFromTop(28);
    for (auto& badge : badges_) {
        const auto width = juce::jmin(badge->idealWidth(), badgeRow.getWidth());
        badge->setBounds(badgeRow.removeFromLeft(width));
        badgeRow.removeFromLeft(6);
    }
    bounds.removeFromTop(10);

    signalGraph_.setBounds(bounds.removeFromTop(130));
    bounds.removeFromTop(10);

    const auto columns = bounds.getWidth() >= 720 ? 2 : 1;
    const auto gap = 10;
    const auto columnWidth = columns == 2 ? (bounds.getWidth() - gap) / 2 : bounds.getWidth();
    std::array<int, 2> yPositions { bounds.getY(), bounds.getY() };

    for (std::size_t index = 0; index < panels_.size(); ++index) {
        const auto column = columns == 2 ? static_cast<int>(index % 2) : 0;
        const auto x = bounds.getX() + column * (columnWidth + gap);
        const auto availableHeight = juce::jmax(80, bounds.getBottom() - yPositions[static_cast<std::size_t>(column)]);
        const auto height = juce::jmin(panels_[index]->preferredHeight(), availableHeight);
        panels_[index]->setBounds(x, yPositions[static_cast<std::size_t>(column)], columnWidth, height);
        yPositions[static_cast<std::size_t>(column)] += height + gap;
    }

    auto actions = getLocalBounds().reduced(outerPadding);
    actions.removeFromTop(headerHeight + comparisonHeight + 10);
    actions.removeFromBottom(footerHeight + 8);
    auto actionRow = actions.removeFromBottom(36);
    secondaryActionButton_.setBounds(actionRow.removeFromRight(122));
    actionRow.removeFromRight(8);
    primaryActionButton_.setBounds(actionRow.removeFromRight(148));
}

bool PreviewComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::F1Key) {
        setTransientStatus("Context help: " + juce::String(scenario_ != nullptr ? scenario_->journey : ""));
        return true;
    }

    if (key.getModifiers().isCtrlDown() && key.getKeyCode() == 'Z') {
        setTransientStatus(key.getModifiers().isShiftDown() ? "Redo preview" : "Undo preview");
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
    return getWidth() < 760;
}

bool PreviewComponent::isWide() const
{
    return getWidth() >= 1100;
}

} // namespace ehvota::previewapp
