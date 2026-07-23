#include "PreviewComponent.h"

#include "shared/ui/EhvotaLookAndFeel.h"
#include "shared/ui/Theme.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <optional>
#include <stdexcept>
#include <string>

namespace {

constexpr int minimumWidth = 720;
constexpr int minimumHeight = 500;
constexpr int maximumWidth = 1920;
constexpr int maximumHeight = 1200;
constexpr int defaultWidth = 1180;
constexpr int defaultHeight = 760;

struct LaunchOptions {
    juce::String scenarioId { "reference-ready" };
    int contentWidth { defaultWidth };
    int contentHeight { defaultHeight };
    std::optional<int> journeyStep;
};

std::optional<int> parseBoundedInteger(
    const juce::ArgumentList& arguments,
    const juce::String& option,
    const int minimum,
    const int maximum,
    const juce::String& units,
    juce::String& error)
{
    if (!arguments.containsOption(option))
        return std::nullopt;

    const auto value = arguments.getValueForOption(option);
    if (value.isEmpty() || !value.containsOnly("0123456789")) {
        error = option + " requires an integer value using " + option + "=<value>.";
        return std::nullopt;
    }

    const auto parsed = value.getIntValue();
    if (parsed < minimum || parsed > maximum) {
        error = option + " must be between " + juce::String(minimum)
            + " and " + juce::String(maximum)
            + (units.isNotEmpty() ? " " + units : juce::String {})
            + ".";
        return std::nullopt;
    }

    return parsed;
}

std::optional<LaunchOptions> parseLaunchOptions(
    const juce::String& executable,
    const juce::String& commandLine,
    juce::String& error)
{
    const juce::ArgumentList arguments(executable, commandLine);
    for (int index = 0; index < arguments.size(); ++index) {
        const auto argument = arguments[index];
        if (!argument.isLongOption("--scenario")
            && !argument.isLongOption("--width")
            && !argument.isLongOption("--height")
            && !argument.isLongOption("--step")) {
            error = "Unknown option: " + argument.text;
            return std::nullopt;
        }
    }

    LaunchOptions options;
    if (arguments.containsOption("--scenario")) {
        options.scenarioId = arguments.getValueForOption("--scenario");
        if (options.scenarioId.isEmpty()) {
            error = "--scenario requires a value using --scenario=<id>.";
            return std::nullopt;
        }
    }

    if (arguments.containsOption("--width")) {
        const auto width = parseBoundedInteger(
            arguments, "--width", minimumWidth, maximumWidth, "logical pixels", error);
        if (!width.has_value())
            return std::nullopt;
        options.contentWidth = *width;
    }

    if (arguments.containsOption("--height")) {
        const auto height = parseBoundedInteger(
            arguments, "--height", minimumHeight, maximumHeight, "logical pixels", error);
        if (!height.has_value())
            return std::nullopt;
        options.contentHeight = *height;
    }

    if (arguments.containsOption("--step")) {
        const auto step = parseBoundedInteger(arguments, "--step", 1, 32, "steps", error);
        if (!step.has_value())
            return std::nullopt;
        options.journeyStep = *step;
    }

    return options;
}

class PreviewWindow final : public juce::DocumentWindow {
public:
    explicit PreviewWindow(const LaunchOptions& options)
        : juce::DocumentWindow(
              "Ehvota UI Preview",
              juce::Colour(ehvota::ui::colour::canvas),
              juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(minimumWidth, minimumHeight, maximumWidth, maximumHeight);

        auto content = std::make_unique<ehvota::previewapp::PreviewComponent>();
        content->setSize(options.contentWidth, options.contentHeight);
        if (!content->selectScenarioById(options.scenarioId.toStdString()))
            throw std::invalid_argument("Unknown preview scenario: " + options.scenarioId.toStdString());
        if (options.journeyStep.has_value()
            && !content->selectJourneyStep(static_cast<std::size_t>(*options.journeyStep))) {
            throw std::invalid_argument(
                "Journey step is unavailable for scenario "
                + options.scenarioId.toStdString() + ": "
                + std::to_string(*options.journeyStep));
        }

        preview_ = content.get();
        setContentOwned(content.release(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        preview_->grabKeyboardFocus();
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    ehvota::previewapp::PreviewComponent* preview_ { nullptr };
};

class PreviewApplication final : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "Ehvota UI Preview"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);

        juce::String error;
        const auto options = parseLaunchOptions(getApplicationName(), commandLine, error);
        if (!options.has_value()) {
            failLaunch(error);
            return;
        }

        try {
            window_ = std::make_unique<PreviewWindow>(*options);
        }
        catch (const std::exception& exception) {
            failLaunch(exception.what());
        }
    }

    void shutdown() override
    {
        window_.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override
    {
        if (window_ != nullptr)
            window_->toFront(true);
    }

private:
    void failLaunch(const juce::String& message)
    {
        juce::Logger::writeToLog("Ehvota UI Preview: " + message);
        setApplicationReturnValue(2);
        quit();
    }

    ehvota::ui::EhvotaLookAndFeel lookAndFeel_;
    std::unique_ptr<PreviewWindow> window_;
};

} // namespace

START_JUCE_APPLICATION(PreviewApplication)
