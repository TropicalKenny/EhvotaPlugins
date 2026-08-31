#include "shared/audio/CaptureBuffer.h"

#include <array>
#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}
} // namespace

int main()
{
    ehvota::audio::CaptureBuffer capture;
    capture.prepare(1000.0, 2.0);
    require(capture.state() == ehvota::audio::CaptureState::idle, "prepared capture should be idle");
    require(capture.arm(1.0), "valid duration should arm capture");

    std::array<float, 400> left {};
    std::array<float, 400> right {};
    left.fill(0.25f);
    right.fill(-0.25f);

    capture.push(left, right);
    require(capture.state() == ehvota::audio::CaptureState::capturing, "first block should start capture");
    require(capture.capturedSamples() == 400, "first block should copy 400 samples");

    capture.push(left, right);
    require(capture.capturedSamples() == 800, "second block should advance without allocation");
    capture.push(left, right);
    require(capture.state() == ehvota::audio::CaptureState::complete, "capture should stop exactly at requested duration");
    require(capture.capturedSamples() == 1000, "capture must clamp final copy to target samples");
    require(capture.progress() == 1.0, "completed capture should report full progress");

    const auto view = capture.completedView();
    require(view.left.size() == 1000 && view.right.size() == 1000, "completed view should expose exact target length");
    require(view.left.front() == 0.25f && view.right.front() == -0.25f, "capture should preserve stereo samples");

    require(!capture.arm(3.0), "duration beyond prepared capacity must be rejected");
    require(capture.arm(0.5), "completed capture should be re-armable");
    const auto generation = capture.generation();
    capture.cancel();
    require(capture.state() == ehvota::audio::CaptureState::cancelled, "cancel should be explicit");
    require(capture.generation() == generation, "cancel must not fabricate a new capture generation");

    std::cout << "Capture buffer tests passed\n";
    return 0;
}
