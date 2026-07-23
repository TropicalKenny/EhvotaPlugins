#include "PreviewComponent.h"

#include "shared/ui/EhvotaLookAndFeel.h"
#include "shared/ui/Theme.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace {

class PreviewWindow final : public juce::DocumentWindow {
public:
    PreviewWindow()
        : juce::DocumentWindow(
              "Ehvota UI Preview",
              juce::Colour(ehvota::ui::colour::canvas),
              juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setResizeLimits(720, 500, 1920, 1200);
        setContentOwned(new ehvota::previewapp::PreviewComponent(), true);
        centreWithSize(1180, 760);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class PreviewApplication final : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "Ehvota UI Preview"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);
        window_ = std::make_unique<PreviewWindow>();
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
    ehvota::ui::EhvotaLookAndFeel lookAndFeel_;
    std::unique_ptr<PreviewWindow> window_;
};

} // namespace

START_JUCE_APPLICATION(PreviewApplication)
