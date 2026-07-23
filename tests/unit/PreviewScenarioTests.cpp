#include "shared/timing/RefTimeMath.h"
#include "shared/ui/DesignTokenValues.h"
#include "shared/viewmodels/LayoutPolicy.h"
#include "shared/viewmodels/PreviewScenario.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message)
{
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void testScenarioCatalogue()
{
    const auto errors = ehvota::preview::validateScenarioCatalogue();
    for (const auto& error : errors)
        std::cerr << "CATALOGUE: " << error << '\n';

    expect(errors.empty(), "preview scenario catalogue must be valid");
    expect(ehvota::preview::allScenarios().size() >= 21,
           "catalogue must include eight journeys plus all mandatory state fixtures");
    expect(ehvota::preview::findScenario("proposal-review") != nullptr,
           "proposal review fixture must exist");
    expect(ehvota::preview::findScenario("render-safe") != nullptr,
           "render-safe fixture must exist");
    expect(ehvota::preview::findScenario("not-a-scenario") == nullptr,
           "unknown fixture must not resolve");
}

void testRefTimeCalculation()
{
    using ehvota::timing::NoteModifier;
    using ehvota::timing::calculateNoteTiming;

    const auto at120 = calculateNoteTiming(120.0, 32, NoteModifier::dotted, 48'000.0);
    expect(std::abs(at120.exactMilliseconds - 93.75) < 1.0e-9,
           "1/32 dotted at 120 BPM must equal 93.750 ms");
    expect(at120.samples == 4'500,
           "1/32 dotted at 120 BPM and 48 kHz must equal 4,500 samples");

    const auto at128 = calculateNoteTiming(128.0, 32, NoteModifier::dotted, 48'000.0);
    expect(std::abs(at128.exactMilliseconds - 87.890625) < 1.0e-9,
           "1/32 dotted at 128 BPM must equal 87.890625 ms");

    bool rejectedInvalidBpm = false;
    try {
        static_cast<void>(calculateNoteTiming(0.0, 32, NoteModifier::dotted, 48'000.0));
    }
    catch (const std::invalid_argument&) {
        rejectedInvalidBpm = true;
    }
    expect(rejectedInvalidBpm, "invalid BPM must be rejected");
}

double channelLuminance(const int channel)
{
    const auto value = static_cast<double>(channel) / 255.0;
    return value <= 0.04045
        ? value / 12.92
        : std::pow((value + 0.055) / 1.055, 2.4);
}

double luminance(const std::uint32_t argb)
{
    const auto red = static_cast<int>((argb >> 16U) & 0xffU);
    const auto green = static_cast<int>((argb >> 8U) & 0xffU);
    const auto blue = static_cast<int>(argb & 0xffU);
    return 0.2126 * channelLuminance(red)
        + 0.7152 * channelLuminance(green)
        + 0.0722 * channelLuminance(blue);
}

double contrastRatio(const std::uint32_t first, const std::uint32_t second)
{
    const auto firstLuminance = luminance(first);
    const auto secondLuminance = luminance(second);
    const auto lighter = std::max(firstLuminance, secondLuminance);
    const auto darker = std::min(firstLuminance, secondLuminance);
    return (lighter + 0.05) / (darker + 0.05);
}

void testDesignTokenContrast()
{
    using namespace ehvota::ui::token;

    expect(contrastRatio(textPrimary, canvas) >= 7.0,
           "primary text on canvas must meet enhanced contrast");
    expect(contrastRatio(textSecondary, base) >= 4.5,
           "secondary text on base must meet ordinary text contrast");
    expect(contrastRatio(textMuted, base) >= 4.5,
           "muted text on base must remain readable");
    expect(contrastRatio(focus, base) >= 3.0,
           "focus ring against base must be visually distinct");
    expect(contrastRatio(error, raised) >= 4.5,
           "error labels on raised panels must meet ordinary text contrast");
    expect(contrastRatio(liveMix, canvas) >= 3.0,
           "live mix graph color must remain visible on canvas");
    expect(contrastRatio(reference, canvas) >= 3.0,
           "reference graph color must remain visible on canvas");
}

void testLayoutPolicy()
{
    using ehvota::preview::LayoutClass;
    using ehvota::preview::classifyLayout;

    expect(classifyLayout(720) == LayoutClass::compact,
           "minimum preview width must use compact layout");
    expect(classifyLayout(759) == LayoutClass::compact,
           "759 logical pixels must remain compact");
    expect(classifyLayout(760) == LayoutClass::standard,
           "760 logical pixels must enter standard layout");
    expect(classifyLayout(1'099) == LayoutClass::standard,
           "1099 logical pixels must remain standard");
    expect(classifyLayout(1'100) == LayoutClass::wide,
           "1100 logical pixels must enter wide layout");

    bool rejectedInvalidWidth = false;
    try {
        static_cast<void>(classifyLayout(0));
    }
    catch (const std::invalid_argument&) {
        rejectedInvalidWidth = true;
    }
    expect(rejectedInvalidWidth, "non-positive logical width must be rejected");
}

} // namespace

int main()
{
    testScenarioCatalogue();
    testRefTimeCalculation();
    testDesignTokenContrast();
    testLayoutPolicy();

    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed.\n";
        return 1;
    }

    std::cout << "All Ehvota deterministic unit tests passed.\n";
    return 0;
}
