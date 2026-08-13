#pragma once

#include "PluginProcessor.h"
#include "ReferenceFolderLoader.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace ehvota::refhub {

class RefHubAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefHubAudioProcessorEditor(RefHubAudioProcessor&);
    ~RefHubAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void chooseReferenceFolder();
    void applyReferenceFolder(const juce::File& directory);

    RefHubAudioProcessor& processor_;
    juce::ComboBox referenceSelector_;
    juce::ComboBox sectionSelector_;
    juce::ComboBox levelMatchSelector_;
    juce::TextButton loadFolderButton_ { "LOAD FOLDER" };
    juce::TextButton mixButton_ { "MIX" };
    juce::TextButton referenceButton_ { "REF" };
    juce::TextButton blindButton_ { "BLIND" };
    juce::TextButton captureButton_ { "CAPTURE" };
    juce::TextButton proposalButton_ { "CREATE PROPOSAL" };
    juce::Label liveMeterLabel_;
    std::unique_ptr<juce::FileChooser> folderChooser_;
    ReferenceFolderLoadResult loadedReference_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessorEditor)
};

} // namespace ehvota::refhub
