#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "shared/analysis/CapturedSectionAnalysis.h"
#include "shared/analysis/ComparisonAnalysis.h"
#include "shared/analysis/SpectrumAnalysis.h"

#include <algorithm>
#include <cmath>
#include <span>
#include <utility>
#include <vector>

namespace ehvota::reftap {

RefTapAudioProcessor::RefTapAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    persistentInstanceId_ = juce::Uuid().toString();
    publishPersistentIdentity();
    startTimerHz(20);
}

RefTapAudioProcessor::~RefTapAudioProcessor()
{
    stopTimer();
    stopCaptureAnalysis();
}

void RefTapAudioProcessor::publishPersistentIdentity()
{
    if (persistentInstanceId_.isEmpty())
        persistentInstanceId_ = juce::Uuid().toString();
    (void) refLink_.updatePersistentId(persistentInstanceId_.toStdString());
}

void RefTapAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    sampleCounter_.store(0, std::memory_order_relaxed);
    scheduledHostStart_.store(-1, std::memory_order_relaxed);
    scheduleMissed_.store(false, std::memory_order_relaxed);
    activeCaptureSequence_.store(0, std::memory_order_relaxed);
    activeCaptureId_.store(0, std::memory_order_relaxed);
    capture_.prepare(sampleRate, 60.0);
    publishPersistentIdentity();
}

void RefTapAudioProcessor::releaseResources()
{
    capture_.cancel();
    stopCaptureAnalysis();
}

bool RefTapAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefTapAudioProcessor::updateTrackProperties(const TrackProperties& properties)
{
    if (properties.name && properties.name->isNotEmpty())
        (void) refLink_.updateName(properties.name->toStdString());
    else
        (void) refLink_.updateName("RefTap");
}

void RefTapAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto channels = buffer.getNumChannels();
    const auto samples = buffer.getNumSamples();
    if (channels <= 0 || samples <= 0)
        return;

    float maxMagnitude = 0.0f;
    double sumSquares = 0.0;
    std::uint64_t sampleCount = 0;
    for (int channel = 0; channel < channels; ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < samples; ++sample) {
            const auto value = data[sample];
            maxMagnitude = juce::jmax(maxMagnitude, std::abs(value));
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            ++sampleCount;
        }
    }

    const auto rms = sampleCount > 0
        ? static_cast<float>(std::sqrt(sumSquares / static_cast<double>(sampleCount)))
        : 0.0f;
    const auto peakDb = juce::Decibels::gainToDecibels(maxMagnitude, -100.0f);
    const auto rmsDb = juce::Decibels::gainToDecibels(rms, -100.0f);
    peakDb_.store(peakDb, std::memory_order_relaxed);
    rmsDb_.store(rmsDb, std::memory_order_relaxed);

    float correlation = 1.0f;
    if (channels >= 2) {
        const auto* left = buffer.getReadPointer(0);
        const auto* right = buffer.getReadPointer(1);
        double lr = 0.0;
        double ll = 0.0;
        double rr = 0.0;
        for (int sample = 0; sample < samples; ++sample) {
            const auto l = static_cast<double>(left[sample]);
            const auto r = static_cast<double>(right[sample]);
            lr += l * r;
            ll += l * l;
            rr += r * r;
        }
        const auto denominator = std::sqrt(ll * rr);
        const auto value = denominator > 1.0e-12 ? lr / denominator : 1.0;
        correlation = static_cast<float>(juce::jlimit(-1.0, 1.0, value));
    }
    correlation_.store(correlation, std::memory_order_relaxed);
    crestDb_.store(maxMagnitude > 0.0f && rms > 0.0f ? peakDb - rmsDb : 0.0f, std::memory_order_relaxed);
    sampleCounter_.fetch_add(static_cast<std::uint64_t>(samples), std::memory_order_relaxed);

    const auto captureState = capture_.state();
    if (captureState == ehvota::audio::CaptureState::armed
        || captureState == ehvota::audio::CaptureState::capturing) {
        const auto left = std::span<const float>(
            buffer.getReadPointer(0), static_cast<std::size_t>(samples));
        const auto right = channels > 1
            ? std::span<const float>(buffer.getReadPointer(1), static_cast<std::size_t>(samples))
            : left;

        if (captureState == ehvota::audio::CaptureState::capturing) {
            capture_.push(left, right);
        } else {
            const auto scheduled = scheduledHostStart_.load(std::memory_order_acquire);
            if (scheduled < 0) {
                capture_.push(left, right);
            } else {
                bool haveHostSample = false;
                std::int64_t blockStart = 0;
                if (auto* playHead = getPlayHead()) {
                    if (auto position = playHead->getPosition()) {
                        if (auto hostSample = position->getTimeInSamples()) {
                            haveHostSample = true;
                            blockStart = static_cast<std::int64_t>(*hostSample);
                        }
                    }
                }

                if (!haveHostSample) {
                    scheduleMissed_.store(true, std::memory_order_release);
                    capture_.cancel();
                } else {
                    const auto blockEnd = blockStart + static_cast<std::int64_t>(samples);
                    if (scheduled < blockStart) {
                        scheduleMissed_.store(true, std::memory_order_release);
                        capture_.cancel();
                    } else if (scheduled < blockEnd) {
                        const auto offset = static_cast<std::size_t>(scheduled - blockStart);
                        capture_.push(left.subspan(offset), right.subspan(offset));
                    }
                }
            }
        }
    }

    // RefTap remains bit-transparent; it never writes to the host audio buffer.
}

void RefTapAudioProcessor::handleCaptureRequest()
{
    using namespace ehvota::communication;
    const auto request = refLink_.consumePendingCaptureRequest();
    if (!request)
        return;

    if (captureAnalysisRunning_.load(std::memory_order_acquire)
        || capture_.state() == ehvota::audio::CaptureState::armed
        || capture_.state() == ehvota::audio::CaptureState::capturing) {
        (void) refLink_.updateCaptureState(request->sequence, SharedCaptureState::rejected, 1);
        return;
    }

    const auto durationSeconds = static_cast<double>(request->durationSamples) / sampleRate_;
    if (!(sampleRate_ > 0.0) || !capture_.arm(durationSeconds)) {
        (void) refLink_.updateCaptureState(request->sequence, SharedCaptureState::rejected, 2);
        return;
    }

    activeCaptureSequence_.store(request->sequence, std::memory_order_release);
    activeCaptureId_.store(request->captureId, std::memory_order_release);
    scheduledHostStart_.store(request->startHostSample, std::memory_order_release);
    scheduleMissed_.store(false, std::memory_order_release);
}

void RefTapAudioProcessor::launchCaptureAnalysis()
{
    using namespace ehvota::communication;
    const auto view = capture_.completedView();
    const auto sequence = activeCaptureSequence_.load(std::memory_order_acquire);
    const auto captureId = activeCaptureId_.load(std::memory_order_acquire);
    const auto instance = refLink_.id();
    if (view.left.empty() || view.right.empty() || sequence == 0 || captureId == 0 || !instance.valid()) {
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::error, 4);
        return;
    }

    std::vector<float> left;
    std::vector<float> right;
    try {
        left.assign(view.left.begin(), view.left.end());
        right.assign(view.right.begin(), view.right.end());
    } catch (...) {
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::error, 5);
        return;
    }

    if (captureAnalysisThread_.joinable())
        captureAnalysisThread_.join();

    captureAnalysisRunning_.store(true, std::memory_order_release);
    (void) refLink_.updateCaptureState(sequence, SharedCaptureState::analysing, 0);
    const auto sampleRate = view.sampleRate;
    const auto sourceGeneration = instance.generation;

    captureAnalysisThread_ = std::jthread(
        [this, captureId, sourceGeneration, sampleRate,
            left = std::move(left), right = std::move(right)](const std::stop_token stopToken) mutable {
            CaptureAnalysisResult output;
            if (!stopToken.stop_requested()) {
                auto analysed = ehvota::analysis::analyzeCapturedSection(
                    "reftap:" + std::to_string(captureId), left, right, sampleRate);
                if (analysed.valid && !stopToken.stop_requested()) {
                    auto& summary = output.analysis;
                    summary.captureId = captureId;
                    summary.sourceGeneration = sourceGeneration;
                    summary.valid = 1;
                    summary.sampleRate = static_cast<float>(sampleRate);
                    summary.capturedSamples = static_cast<std::uint32_t>(
                        std::min<std::size_t>(left.size(), ehvota::communication::sharedCaptureMaximumSamples));
                    summary.integratedLufs = static_cast<float>(analysed.loudness.integratedLufs);
                    summary.peakDb = static_cast<float>(analysed.stereo.peakDbfs);
                    summary.rmsDb = static_cast<float>(analysed.stereo.rmsDbfs);
                    summary.crestDb = static_cast<float>(analysed.stereo.crestFactorDb);
                    summary.correlation = static_cast<float>(analysed.stereo.correlation);
                    summary.sideToMidDb = static_cast<float>(analysed.stereo.sideToMidDb);
                    const auto& definitions = ehvota::analysis::defaultComparisonBands();
                    for (std::size_t i = 0; i < definitions.size(); ++i) {
                        const auto band = ehvota::analysis::summarizeBand(
                            analysed.spectrum, definitions[i].lowHz, definitions[i].highHz);
                        summary.bandPowerDb[i] = static_cast<float>(band.averagePowerDb);
                    }
                    output.valid = ehvota::communication::validateSharedCapturedAnalysis(summary);
                    output.resultCode = output.valid ? 0U : 7U;
                } else {
                    output.resultCode = 6;
                }
            } else {
                output.resultCode = 8;
            }

            if (!stopToken.stop_requested()) {
                const juce::ScopedLock lock(captureAnalysisLock_);
                pendingCaptureAnalysis_ = std::move(output);
            }
            captureAnalysisRunning_.store(false, std::memory_order_release);
        });
}

void RefTapAudioProcessor::handleCaptureProgress()
{
    using namespace ehvota::communication;
    const auto sequence = activeCaptureSequence_.load(std::memory_order_acquire);
    if (sequence == 0)
        return;

    if (scheduleMissed_.exchange(false, std::memory_order_acq_rel)) {
        capture_.cancel();
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::error, 3);
        activeCaptureSequence_.store(0, std::memory_order_release);
        activeCaptureId_.store(0, std::memory_order_release);
        scheduledHostStart_.store(-1, std::memory_order_release);
        return;
    }

    std::optional<CaptureAnalysisResult> finished;
    {
        const juce::ScopedLock lock(captureAnalysisLock_);
        if (pendingCaptureAnalysis_) {
            finished = std::move(pendingCaptureAnalysis_);
            pendingCaptureAnalysis_.reset();
        }
    }
    if (finished) {
        if (finished->valid && refLink_.publishCapturedAnalysis(finished->analysis))
            (void) refLink_.updateCaptureState(sequence, SharedCaptureState::complete, 0);
        else
            (void) refLink_.updateCaptureState(sequence, SharedCaptureState::error, finished->resultCode ? finished->resultCode : 9);
        capture_.reset();
        activeCaptureSequence_.store(0, std::memory_order_release);
        activeCaptureId_.store(0, std::memory_order_release);
        scheduledHostStart_.store(-1, std::memory_order_release);
        return;
    }

    const auto state = capture_.state();
    if (state == ehvota::audio::CaptureState::capturing) {
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::capturing, 0);
    } else if (state == ehvota::audio::CaptureState::complete
        && !captureAnalysisRunning_.load(std::memory_order_acquire)) {
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::capturing, 0);
        launchCaptureAnalysis();
    } else if (state == ehvota::audio::CaptureState::cancelled) {
        (void) refLink_.updateCaptureState(sequence, SharedCaptureState::error, 10);
        activeCaptureSequence_.store(0, std::memory_order_release);
        activeCaptureId_.store(0, std::memory_order_release);
        scheduledHostStart_.store(-1, std::memory_order_release);
    }
}

void RefTapAudioProcessor::stopCaptureAnalysis() noexcept
{
    if (captureAnalysisThread_.joinable()) {
        captureAnalysisThread_.request_stop();
        captureAnalysisThread_.join();
    }
    captureAnalysisRunning_.store(false, std::memory_order_release);
}

void RefTapAudioProcessor::timerCallback()
{
    refLink_.publish({
        peakDb_.load(std::memory_order_relaxed),
        rmsDb_.load(std::memory_order_relaxed),
        correlation_.load(std::memory_order_relaxed),
        crestDb_.load(std::memory_order_relaxed),
        sampleCounter_.load(std::memory_order_relaxed)
    });
    handleCaptureRequest();
    handleCaptureProgress();
}

juce::AudioProcessorEditor* RefTapAudioProcessor::createEditor()
{
    return new RefTapAudioProcessorEditor(*this);
}

void RefTapAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    juce::MemoryOutputStream stream(destination, true);
    stream.writeInt(4);
    stream.writeString(persistentInstanceId_);
}

void RefTapAudioProcessor::setStateInformation(const void* data, const int bytes)
{
    if (data == nullptr || bytes <= 0)
        return;
    juce::MemoryInputStream stream(data, static_cast<std::size_t>(bytes), false);
    const auto version = stream.readInt();
    if (version >= 4) {
        const auto restored = stream.readString();
        if (ehvota::communication::validateSharedPersistentId(restored.toStdString()))
            persistentInstanceId_ = restored;
    }
    publishPersistentIdentity();
}

} // namespace ehvota::reftap

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::reftap::RefTapAudioProcessor();
}
