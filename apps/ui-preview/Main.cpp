#include "ApprovedRefHubPreview.h"
#include "ApprovedRefTimePreview.h"
#include "ApprovedSourcesPreview.h"
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

std::unique_ptr<juce::Component> makePreviewComponent(const ehvota::preview::LaunchOptions& options)
{
    std::unique_ptr<juce::Component> content;

    if (options.scenarioId == "compare-chorus") {
        content = std::make_unique<ehvota::previewapp::ApprovedRefHubPreview>();
    }
    else if (options.scenarioId == "mapping-disconnected") {
        content = std::make_unique<ehvota::previewapp::ApprovedSourcesPreview>();
    }
    else if (options.scenarioId == "reftime-120") {
        content = std::make_unique<ehvota::previewapp::ApprovedRefTimePreview>();
    }
    else {
        auto legacy = std::make_unique<ehvota::previewapp::PreviewComponent>();
        if (!legacy->selectScenarioById(options.scenarioId))
            throw std::invalid_argument("Unknown preview scenario: " + options.scenarioId);
        if (options.journeyStep.has_value()
            && !legacy->selectJourneyStep(static_cast<std::size_t>(*options.journeyStep))) {
            throw std::invalid_argument(
                "Journey step is unavailable for scenario "
                + options.scenarioId + ": "
                + std::to_string(*options.journeyStep));
        }
        content = std::move(legacy);
    }

    content->setSize(options.contentWidth, options.contentHeight);
    return content;
}

void renderSnapshot(const ehvota::preview::LaunchOptions& options, const juce::File& destination)
{
    auto content = makePreviewComponent(options);

    const auto parent = destination.getParentDirectory();
    if (!parent.exists() && !parent.createDirectory())
        throw std::runtime_error("Could not create snapshot directory: " + parent.getFullPathName().toStdString());

    auto image = content->createComponentSnapshot(content->getLocalBounds(), false, 1.0f);
    if (!image.isValid())
        throw std::runtime_error("Could not render preview snapshot image.");

    juce::FileOutputStream stream(destination);
    if (!stream.openedOk())
        throw std::runtime_error("Could not open snapshot destination: " + destination.getFullPathName().toStdString());

    juce::PNGImageFormat png;
    if (!png.writeImageToStream(image, stream))
        throw std::runtime_error("Could not encode preview snapshot as PNG.");

    stream.flush();
    std::cout << "Ehvota UI Preview snapshot: " << destination.getFullPathName() << std::endl;
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

        auto content = makePreviewComponent(options);
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
    juce::Component* preview_ { nullptr };
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
            const auto snapshotPath = juce::SystemStats::getEnvironmentVariable("EHVOTA_SNAPSHOT_PATH", {});
            if (snapshotPath.isNotEmpty()) {
                renderSnapshot(*parsed.options, juce::File(snapshotPath));
                setApplicationReturnValue(0);
                quit();
                return;
            }

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
