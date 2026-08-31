#pragma once

#include "PluginProcessor.h"
#include "ReferenceFolderLoader.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>
#include <vector>

namespace ehvota::refhub {

class RefHubAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefHubAudioProcessorEditor(RefHubAudioProcessor&);
    ~RefHubAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    enum class Page { compare, sources, process, history, settings };

    void timerCallback() override;
    void chooseReferenceFolder();
    void applyReferenceFolder(const juce::File& directory);
    void setPage(Page page);
    void updateVisibility();
    void refreshLinkedSources();
    void configureMappingSelector(std::size_t roleIndex);
    void beginComponentCaptureFromWindow();
    void beginMainAnalysisFromWindow();

    void paintCommon(juce::Graphics&, juce::Rectangle<int>& bounds);
    void paintCompare(juce::Graphics&, juce::Rectangle<int> bounds);
    void paintSources(juce::Graphics&, juce::Rectangle<int> bounds);
    void paintProcess(juce::Graphics&, juce::Rectangle<int> bounds);
    void paintHistory(juce::Graphics&, juce::Rectangle<int> bounds);
    void paintSettings(juce::Graphics&, juce::Rectangle<int> bounds);

    [[nodiscard]] std::optional<double> editorDouble(const juce::TextEditor&) const;
    [[nodiscard]] std::optional<ehvota::communication::SharedInstanceId> firstLinked(
        ehvota::communication::SharedPluginKind kind) const;

    RefHubAudioProcessor& processor_;
    Page page_ { Page::compare };

    juce::TextButton navCompare_ { "COMPARE" };
    juce::TextButton navSources_ { "SOURCES" };
    juce::TextButton navProcess_ { "PROCESS" };
    juce::TextButton navHistory_ { "HISTORY" };
    juce::TextButton navSettings_ { "SETTINGS" };

    juce::ComboBox referenceSelector_;
    juce::ComboBox sectionSelector_;
    juce::ComboBox levelMatchSelector_;
    juce::TextButton loadFolderButton_ { "LOAD FOLDER" };
    juce::TextButton mixButton_ { "MIX" };
    juce::TextButton referenceButton_ { "REF" };
    juce::TextButton blindButton_ { "BLIND" };
    juce::TextButton captureButton_ { "CAPTURE" };
    juce::TextButton proposalButton_ { "ANALYSE / PROPOSE" };
    juce::Label liveMeterLabel_;

    std::array<juce::ComboBox, 4> mappingSelectors_;
    std::array<juce::ToggleButton, 4> scopeToggles_;
    std::array<std::vector<ehvota::communication::SharedInstanceId>, 4> mappingChoices_;
    ehvota::state::AnalysisScope analysisScope_;
    juce::TextEditor liveDurationEditor_;
    juce::TextEditor referenceStartEditor_;
    juce::TextEditor referenceDurationEditor_;
    juce::TextButton componentCaptureButton_ { "CAPTURE MAPPED BUSES" };
    juce::Label componentStatusLabel_;

    juce::TextButton createProposalButton_ { "CREATE PROPOSAL" };
    juce::TextButton approveProposalButton_ { "APPROVE + SEND" };
    juce::TextButton rejectProposalButton_ { "REJECT" };
    juce::TextButton undoButton_ { "UNDO LAST" };

    juce::ToggleButton renderSafeToggle_ { "Render Safe" };

    std::unique_ptr<juce::FileChooser> folderChooser_;
    ReferenceFolderLoadResult loadedReference_;
    juce::String linkedSignature_;
    int refreshCounter_ {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessorEditor)
};

} // namespace ehvota::refhub
