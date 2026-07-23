#include "PreviewComponent.h"

#include "shared/ui/EhvotaLookAndFeel.h"
#include "shared/ui/Theme.h"
#include "shared/viewmodels/PreviewLaunchOptions.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

#if JUCE_WINDOWS
#include <io.h>
#endif

namespace {

bool shouldShowLaunchErrorDialog()
{
#if JUCE_WINDOWS
    return _fileno(stderr) < 0;
#else
    return false;
#endif
}

std::vector<std::string> launchArgumentsFromJuce()
{
    const auto arguments = juce::JUCEApplicationBase::getCommandLineParameterArray();
    std::vector<std::string> result;
    result.reserve(static_cast<std::size_t>(arguments.size()));

    for (const auto& argument : arguments)
        result.push_back(argument.toStdString());

    return result;
}

class PreviewWindow final : public juce::DocumentWindow {
public:
    explicit PreviewWindow(const ehvota::preview::LaunchOptions& options)
        : juce::DocumentWindow(
              "Ehvota UI Preview",
              juce::Colour(ehvota::ui::colour::canvas),
              juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(
            ehvota::preview::minimumLaunchWidth,
            ehvota::preview::minimumLaunchHeight,
            ehvota::preview::maximumLaunchWidth,
            ehvota::preview::maximumLaunchHeight);

        auto content = std::make_unique<ehvota::previewapp::PreviewComponent>();
        content->setSize(options.contentWidth, options.contentHeight);
        if (!content->selectScenarioById(options.scenarioId))
            throw std::invalid_argument("Unknown preview scenario: " + options.scenarioId);
        if (options.journeyStep.has_value()
            && !content->selectJourneyStep(static_cast<std::size_t>(*options.journeyStep))) {
            throw std::invalid_argument(
                "Journey step is unavailable for scenario "
                + options.scenarioId + ": "
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
        juce::ignoreUnused(commandLine);
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);

        const auto parsed = ehvota::preview::parseLaunchOptions(launchArgumentsFromJuce());
        if (!parsed.options.has_value()) {
            failLaunch(parsed.error);
            return;
        }

        try {
            window_ = std::make_unique<PreviewWindow>(*parsed.options);
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
        std::cerr << "Ehvota UI Preview: " << message << std::endl;
        if (!shouldShowLaunchErrorDialog()) {
            setApplicationReturnValue(2);
            quit();
            return;
        }

#if JUCE_MODAL_LOOPS_PERMITTED
        juce::NativeMessageBox::showMessageBox(
            juce::MessageBoxIconType::WarningIcon,
            "Ehvota UI Preview could not start",
            message);
#else
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Ehvota UI Preview could not start",
            message);
#endif
        setApplicationReturnValue(2);
        quit();
    }

    ehvota::ui::EhvotaLookAndFeel lookAndFeel_;
    std::unique_ptr<PreviewWindow> window_;
};

} // namespace

START_JUCE_APPLICATION(PreviewApplication)
