#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ehvota::analysis {

struct LoudnessResult {
    bool valid {};
    double integratedLufs {-100.0};
    double ungatedLufs {-100.0};
    double absoluteGateLufs {-70.0};
    double relativeGateLufs {-100.0};
    std::size_t totalBlocks {};
    std::size_t gatedBlocks {};
};

class Loudness1770 {
public:
    explicit Loudness1770(double sampleRate);

    [[nodiscard]] LoudnessResult measureStereo(
        std::span<const float> left,
        std::span<const float> right) const;

private:
    double sampleRate_ {};
};

} // namespace ehvota::analysis
