#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ReferenceSectionReader.h"
#include "shared/analysis/CapturedSectionAnalysis.h"
#include "shared/safety/LinkProposalAdapter.h"
#include "shared/safety/ReferenceProposalGenerator.h"

#include <cmath>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace ehvota::refhub {

RefHubAudioProcessor::RefHubAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager_.registerBasicFormats();
    startTimerHz(10);
}

RefHubAudioProcessor::~RefHubAudioProcessor() { stopTimer(); stopAnalysisThread(); }

void RefHubAudioProcessor::stopAnalysisThread() noexcept
{
    if (!analysisThread_.joinable()) return;
    analysisThread_.request_stop();
    analysisThread_.join();
    analysisRunning_.store(false, std::memory_order_release);
}

void RefHubAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    referenceTransport_.prepareToPlay(samplesPerBlock, sampleRate);
    mixCapture_.prepare(sampleRate, 60.0);
    sampleCounter_.store(0, std::memory_order_relaxed);
}

void RefHubAudioProcessor::releaseResources() { mixCapture_.cancel(); referenceTransport_.releaseResources(); }

bool RefHubAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet() == layout.getMainOutputChannelSet()
        && (layout.getMainInputChannelSet() == juce::AudioChannelSet::mono() || layout.getMainInputChannelSet() == juce::AudioChannelSet::stereo());
}

bool RefHubAudioProcessor::loadReferenceFile(const juce::File& file)
{
    stopReferenceAudition();
    referenceTransport_.setSource(nullptr);
    referenceReaderSource_.reset();
    referenceLoaded_.store(false, std::memory_order_release);
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager_.createReaderFor(file));
    if (!reader) return false;
    const auto sourceRate = reader->sampleRate;
    referenceReaderSource_ = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    referenceTransport_.setSource(referenceReaderSource_.get(), 0, nullptr, sourceRate, 2);
    referenceTransport_.setPosition(0.0);
    referenceFile_ = file;
    referenceLoaded_.store(true, std::memory_order_release);
    return true;
}

void RefHubAudioProcessor::unloadReferenceFile()
{
    stopReferenceAudition();
    referenceTransport_.setSource(nullptr);
    referenceReaderSource_.reset();
    referenceFile_ = {};
    referenceLoaded_.store(false, std::memory_order_release);
}

void RefHubAudioProcessor::startReferenceAudition()
{
    if (!hasReferenceLoaded()) return;
    if (referenceTransport_.hasStreamFinished()) referenceTransport_.setPosition(0.0);
    referenceTransport_.start();
    referenceAudition_.store(true, std::memory_order_release);
}

void RefHubAudioProcessor::stopReferenceAudition() { referenceAudition_.store(false, std::memory_order_release); referenceTransport_.stop(); }
void RefHubAudioProcessor::setReferenceMonitorTrimDb(const float db) noexcept { referenceTrimDb_.store(juce::jlimit(-24.0f, 24.0f, db), std::memory_order_relaxed); }

bool RefHubAudioProcessor::beginMixCapture(const double durationSeconds) noexcept
{
    if (analysisRunning_.load(std::memory_order_acquire)) return false;
    return mixCapture_.arm(durationSeconds);
}
void RefHubAudioProcessor::cancelMixCapture() noexcept { if (!analysisRunning_.load(std::memory_order_acquire)) mixCapture_.cancel(); }

bool RefHubAudioProcessor::startSectionAnalysis(
    const ehvota::communication::SharedInstanceId refEqTarget,
    const double referenceStartSeconds,
    const double referenceDurationSeconds,
    std::string sectionId)
{
    if (!refEqTarget.valid() || !(referenceStartSeconds >= 0.0) || !(referenceDurationSeconds > 0.0)
        || referenceDurationSeconds > 60.001 || !std::isfinite(referenceStartSeconds)
        || !std::isfinite(referenceDurationSeconds) || sectionId.empty()
        || !hasReferenceLoaded() || !referenceFile_.existsAsFile())
        return false;

    if (const auto staged = copyStagedProposal()) {
        if (staged->state == ehvota::safety::ProposalState::readyForReview || staged->state == ehvota::safety::ProposalState::approved)
            return false;
    }

    bool expected = false;
    if (!analysisRunning_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return false;

    const auto capture = mixCapture_.completedView();
    if (capture.left.empty() || capture.right.empty() || !(capture.sampleRate > 0.0)) {
        analysisRunning_.store(false, std::memory_order_release);
        analysisState_.store(SectionAnalysisState::error, std::memory_order_release);
        return false;
    }

    const auto refSamplesDouble = std::llround(referenceDurationSeconds * capture.sampleRate);
    if (refSamplesDouble <= 0 || static_cast<unsigned long long>(refSamplesDouble) > std::numeric_limits<std::size_t>::max()) {
        analysisRunning_.store(false, std::memory_order_release);
        analysisState_.store(SectionAnalysisState::error, std::memory_order_release);
        return false;
    }
    const auto referenceSamples = static_cast<std::size_t>(refSamplesDouble);

    std::vector<float> mixLeft;
    std::vector<float> mixRight;
    try {
        mixLeft.assign(capture.left.begin(), capture.left.end());
        mixRight.assign(capture.right.begin(), capture.right.end());
    } catch (...) {
        analysisRunning_.store(false, std::memory_order_release);
        analysisState_.store(SectionAnalysisState::error, std::memory_order_release);
        return false;
    }

    if (analysisThread_.joinable()) analysisThread_.join();
    const auto file = referenceFile_;
    const auto sampleRate = capture.sampleRate;
    const auto captureGeneration = capture.generation;
    analysisState_.store(SectionAnalysisState::running, std::memory_order_release);
    { const juce::ScopedLock lock(comparisonLock_); latestComparison_.reset(); }

    analysisThread_ = std::jthread(
        [this, file, referenceStartSeconds, referenceSamples, sampleRate, captureGeneration,
            target = refEqTarget, id = std::move(sectionId), left = std::move(mixLeft), right = std::move(mixRight)](const std::stop_token stopToken) mutable {
            const auto finish = [this](const SectionAnalysisState state) {
                analysisState_.store(state, std::memory_order_release);
                analysisRunning_.store(false, std::memory_order_release);
            };
            if (stopToken.stop_requested()) { finish(SectionAnalysisState::idle); return; }

            const auto reference = readReferenceSection(file, referenceStartSeconds, sampleRate, referenceSamples);
            if (!reference.valid || stopToken.stop_requested()) {
                finish(stopToken.stop_requested() ? SectionAnalysisState::idle : SectionAnalysisState::error);
                return;
            }

            auto mixAnalysis = ehvota::analysis::analyzeCapturedSection(id + ":mix", left, right, sampleRate);
            auto refAnalysis = ehvota::analysis::analyzeCapturedSection(id + ":reference", reference.left, reference.right, sampleRate);
            auto comparison = ehvota::analysis::compareCapturedSections(mixAnalysis, refAnalysis);
            { const juce::ScopedLock lock(comparisonLock_); latestComparison_ = comparison; }
            if (!comparison.valid || stopToken.stop_requested()) {
                finish(stopToken.stop_requested() ? SectionAnalysisState::idle : SectionAnalysisState::error);
                return;
            }

            const auto evidence = id + ":capture:" + std::to_string(captureGeneration);
            finish(stageRefEqProposal(comparison, target, evidence) ? SectionAnalysisState::proposalReady : SectionAnalysisState::noProposal);
        });
    return true;
}

std::optional<ehvota::analysis::SectionComparison> RefHubAudioProcessor::copyLatestComparison() const
{
    const juce::ScopedLock lock(comparisonLock_);
    return latestComparison_;
}

void RefHubAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    float peak = 0.0f; double sumSquares = 0.0; std::uint64_t count = 0;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i) { const auto sample = data[i]; peak = juce::jmax(peak, std::abs(sample)); sumSquares += static_cast<double>(sample) * sample; ++count; }
    }
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        const auto left = std::span<const float>(buffer.getReadPointer(0), static_cast<std::size_t>(buffer.getNumSamples()));
        const auto right = buffer.getNumChannels() > 1 ? std::span<const float>(buffer.getReadPointer(1), static_cast<std::size_t>(buffer.getNumSamples())) : left;
        mixCapture_.push(left, right);
    }
    const auto rms = count > 0 ? static_cast<float>(std::sqrt(sumSquares / static_cast<double>(count))) : 0.0f;
    livePeakDb_.store(juce::Decibels::gainToDecibels(peak, -100.0f), std::memory_order_relaxed);
    liveRmsDb_.store(juce::Decibels::gainToDecibels(rms, -100.0f), std::memory_order_relaxed);
    sampleCounter_.fetch_add(static_cast<std::uint64_t>(buffer.getNumSamples()), std::memory_order_relaxed);

    const bool protectedRender = renderSafe_.load(std::memory_order_acquire) && isNonRealtime();
    const bool audition = referenceAudition_.load(std::memory_order_acquire) && referenceLoaded_.load(std::memory_order_acquire) && !protectedRender;
    if (audition) {
        juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
        referenceTransport_.getNextAudioBlock(info);
        buffer.applyGain(juce::Decibels::decibelsToGain(referenceTrimDb_.load(std::memory_order_relaxed)));
        if (referenceTransport_.hasStreamFinished()) referenceAudition_.store(false, std::memory_order_release);
    }
}

void RefHubAudioProcessor::timerCallback()
{
    const auto peak = livePeakDb_.load(std::memory_order_relaxed), rms = liveRmsDb_.load(std::memory_order_relaxed);
    refLink_.publish({peak, rms, 1.0f, peak > -99.0f && rms > -99.0f ? peak - rms : 0.0f, sampleCounter_.load(std::memory_order_relaxed)});
    std::array<ehvota::communication::SharedInstanceSnapshot, ehvota::communication::sharedLinkMaxInstances> found {};
    const auto foundCount = refLink_.readActive(found);
    std::size_t kept = 0;
    {
        const juce::ScopedLock lock(linkCacheLock_);
        for (std::size_t i = 0; i < foundCount && kept < linkedInstances_.size(); ++i) {
            const auto& instance = found[i];
            if (instance.pluginKind == ehvota::communication::SharedPluginKind::refHub || instance.heartbeatAgeMs > 5000) continue;
            linkedInstances_[kept++] = instance;
        }
        for (std::size_t i = kept; i < linkedInstances_.size(); ++i) linkedInstances_[i] = {};
        linkedCount_.store(kept, std::memory_order_release);
    }
    reconcileStagedProposal(found, foundCount);
}

std::size_t RefHubAudioProcessor::copyLinkedInstances(
    std::array<ehvota::communication::SharedInstanceSnapshot, ehvota::communication::sharedLinkMaxInstances>& destination) const
{
    const juce::ScopedLock lock(linkCacheLock_);
    const auto count = linkedCount_.load(std::memory_order_acquire);
    for (std::size_t i = 0; i < count; ++i) destination[i] = linkedInstances_[i];
    return count;
}

bool RefHubAudioProcessor::stageRefEqProposal(
    const ehvota::analysis::SectionComparison& comparison,
    const ehvota::communication::SharedInstanceId target,
    std::string evidenceId)
{
    std::optional<ehvota::communication::SharedInstanceSnapshot> targetSnapshot;
    {
        const juce::ScopedLock lock(linkCacheLock_);
        const auto count = linkedCount_.load(std::memory_order_acquire);
        for (std::size_t i = 0; i < count; ++i) {
            const auto& candidate = linkedInstances_[i];
            if (candidate.id.slot == target.slot && candidate.id.generation == target.generation && candidate.pluginKind == ehvota::communication::SharedPluginKind::refEQ) { targetSnapshot = candidate; break; }
        }
    }
    if (!targetSnapshot) return false;
    auto proposal = ehvota::safety::generateRefEqProposal(comparison, target, targetSnapshot->parameters, targetSnapshot->heartbeatAgeMs, std::move(evidenceId));
    if (!proposal) return false;
    const juce::ScopedLock lock(proposalLock_);
    stagedProposal_ = std::move(*proposal); stagedTarget_ = target; return true;
}

std::optional<ehvota::safety::Proposal> RefHubAudioProcessor::copyStagedProposal() const { const juce::ScopedLock lock(proposalLock_); return stagedProposal_; }
void RefHubAudioProcessor::rejectStagedProposal(){const juce::ScopedLock lock(proposalLock_);if(!stagedProposal_||stagedProposal_->state==ehvota::safety::ProposalState::applied)return;stagedProposal_->state=ehvota::safety::ProposalState::rejected;stagedProposal_->userExplicitlyApproved=false;}

bool RefHubAudioProcessor::approveAndSubmitStagedProposal()
{
    std::optional<ehvota::safety::Proposal> candidate; ehvota::communication::SharedInstanceId target;
    { const juce::ScopedLock lock(proposalLock_); if(!stagedProposal_||stagedProposal_->state!=ehvota::safety::ProposalState::readyForReview)return false; candidate=stagedProposal_; target=stagedTarget_; }
    candidate->state=ehvota::safety::ProposalState::approved; candidate->userExplicitlyApproved=true;
    if(!ehvota::safety::mayApply(*candidate)||!submitApprovedProposal(*candidate,target))return false;
    const juce::ScopedLock lock(proposalLock_); if(stagedProposal_&&stagedProposal_->id==candidate->id)stagedProposal_=std::move(*candidate); return true;
}

bool RefHubAudioProcessor::submitApprovedProposal(const ehvota::safety::Proposal& proposal,const ehvota::communication::SharedInstanceId target)
{
    auto kind=ehvota::communication::SharedPluginKind::unknown;
    { const juce::ScopedLock lock(linkCacheLock_); const auto count=linkedCount_.load(std::memory_order_acquire); for(std::size_t i=0;i<count;++i){const auto&s=linkedInstances_[i];if(s.id.slot==target.slot&&s.id.generation==target.generation&&s.heartbeatAgeMs<=5000){kind=s.pluginKind;break;}} }
    if(kind==ehvota::communication::SharedPluginKind::unknown)return false;
    auto mailbox=ehvota::safety::encodeApprovedSharedProposal(proposal,target,kind); return mailbox.has_value()&&refLink_.submitProposal(target,*mailbox);
}

std::vector<ehvota::safety::ProposalHistoryEntry> RefHubAudioProcessor::copyProposalHistory() const
{
    const juce::ScopedLock lock(proposalLock_);
    return proposalHistory_.snapshot();
}

bool RefHubAudioProcessor::undoLastAppliedProposal()
{
    std::optional<ehvota::safety::ProposalHistoryEntry> undo;
    {
        const juce::ScopedLock lock(proposalLock_);
        if (stagedProposal_ && stagedProposal_->state == ehvota::safety::ProposalState::approved)
            return false;
        undo = proposalHistory_.makeApprovedUndoForLatest();
    }
    if (!undo || !submitApprovedProposal(undo->proposal, undo->target))
        return false;
    const juce::ScopedLock lock(proposalLock_);
    stagedProposal_ = undo->proposal;
    stagedTarget_ = undo->target;
    return true;
}

void RefHubAudioProcessor::reconcileStagedProposal(
    const std::array<ehvota::communication::SharedInstanceSnapshot, ehvota::communication::sharedLinkMaxInstances>& snapshots,
    const std::size_t count)
{
    const juce::ScopedLock lock(proposalLock_);
    if(!stagedProposal_||stagedProposal_->state!=ehvota::safety::ProposalState::approved)return;
    auto expected=ehvota::safety::encodeApprovedSharedProposal(*stagedProposal_,stagedTarget_,ehvota::communication::SharedPluginKind::refEQ); if(!expected)return;
    for(std::size_t i=0;i<count;++i){const auto&s=snapshots[i];if(s.id.slot!=stagedTarget_.slot||s.id.generation!=stagedTarget_.generation)continue;if(s.mailbox.proposalId!=expected->proposalId)return;switch(s.mailbox.state){case ehvota::communication::SharedProposalState::applied:stagedProposal_->state=ehvota::safety::ProposalState::applied;(void)proposalHistory_.recordApplied(*stagedProposal_,stagedTarget_);return;case ehvota::communication::SharedProposalState::rejected:stagedProposal_->state=ehvota::safety::ProposalState::rejected;if(s.mailbox.resultCode==3)stagedProposal_->targetStateFresh=false;return;case ehvota::communication::SharedProposalState::error:stagedProposal_->state=ehvota::safety::ProposalState::invalid;return;default:return;}}
}

juce::AudioProcessorEditor* RefHubAudioProcessor::createEditor(){return new RefHubAudioProcessorEditor(*this);}void RefHubAudioProcessor::getStateInformation(juce::MemoryBlock&d){juce::MemoryOutputStream s(d,true);s.writeInt(3);s.writeFloat(referenceTrimDb_.load(std::memory_order_relaxed));s.writeBool(renderSafe_.load(std::memory_order_acquire));}void RefHubAudioProcessor::setStateInformation(const void*d,const int n){juce::MemoryInputStream s(d,(std::size_t)n,false);auto v=s.readInt();if(v>=2){setReferenceMonitorTrimDb(s.readFloat());setRenderSafe(s.readBool());}}

} // namespace ehvota::refhub

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ehvota::refhub::RefHubAudioProcessor();}
