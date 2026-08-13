#include "PairedSectionStore.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ehvota::refhub {
namespace {

constexpr int maxTextBytes = 512;

bool acceptableText(const std::string& value) noexcept
{
    return !value.empty() && value.size() <= static_cast<std::size_t>(maxTextBytes);
}

void writeBoundedString(juce::MemoryOutputStream& stream, const std::string& value)
{
    const auto count = static_cast<int>(std::min<std::size_t>(value.size(), maxTextBytes));
    stream.writeInt(count);
    if (count > 0)
        stream.write(value.data(), static_cast<std::size_t>(count));
}

std::optional<std::string> readBoundedString(juce::MemoryInputStream& stream)
{
    const auto count = stream.readInt();
    const auto remaining = stream.getTotalLength() - stream.getPosition();
    if (count < 0 || count > maxTextBytes || remaining < count)
        return std::nullopt;

    std::string value(static_cast<std::size_t>(count), '\0');
    if (count > 0 && stream.read(value.data(), count) != count)
        return std::nullopt;
    return value;
}

bool finiteRange(const double start, const double end) noexcept
{
    return std::isfinite(start) && std::isfinite(end) && start >= 0.0 && end > start;
}

} // namespace

bool PairedSectionStore::replace(
    std::vector<ehvota::state::PairedSection> sections,
    const std::optional<double> mixDurationSeconds,
    const std::optional<double> referenceDurationSeconds)
{
    if (sections.size() > maximumSections)
        return false;
    for (const auto& section : sections) {
        if (!acceptableText(section.id) || !acceptableText(section.label))
            return false;
    }

    const auto validation = ehvota::state::validatePairedSections(
        sections, mixDurationSeconds, referenceDurationSeconds);
    if (!validation.valid)
        return false;

    const juce::ScopedLock scoped(lock_);
    sections_ = std::move(sections);
    return true;
}

std::vector<ehvota::state::PairedSection> PairedSectionStore::copy() const
{
    const juce::ScopedLock scoped(lock_);
    return sections_;
}

std::optional<ehvota::state::PairedSection> PairedSectionStore::find(const std::string& id) const
{
    const juce::ScopedLock scoped(lock_);
    return ehvota::state::findPairedSection(sections_, id);
}

void PairedSectionStore::clear()
{
    const juce::ScopedLock scoped(lock_);
    sections_.clear();
}

void PairedSectionStore::writeState(juce::MemoryOutputStream& stream) const
{
    const auto sections = copy();
    stream.writeInt(static_cast<int>(sections.size()));
    for (const auto& section : sections) {
        writeBoundedString(stream, section.id);
        writeBoundedString(stream, section.label);
        stream.writeDouble(section.mix.startSeconds);
        stream.writeDouble(section.mix.endSeconds);
        stream.writeDouble(section.reference.startSeconds);
        stream.writeDouble(section.reference.endSeconds);
        stream.writeBool(section.enabled);
    }
}

bool PairedSectionStore::readState(
    juce::MemoryInputStream& stream,
    const std::optional<double> mixDurationSeconds,
    const std::optional<double> referenceDurationSeconds)
{
    const auto count = stream.readInt();
    if (count < 0 || count > static_cast<int>(maximumSections))
        return false;

    std::vector<ehvota::state::PairedSection> restored;
    restored.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const auto id = readBoundedString(stream);
        const auto label = readBoundedString(stream);
        if (!id || !label || !acceptableText(*id) || !acceptableText(*label))
            return false;

        const auto mixStart = stream.readDouble();
        const auto mixEnd = stream.readDouble();
        const auto referenceStart = stream.readDouble();
        const auto referenceEnd = stream.readDouble();
        const auto enabled = stream.readBool();
        if (!finiteRange(mixStart, mixEnd) || !finiteRange(referenceStart, referenceEnd))
            return false;

        ehvota::state::PairedSection section;
        section.id = *id;
        section.label = *label;
        section.mix = { section.id + ":mix", section.label, mixStart, mixEnd };
        section.reference = { section.id + ":reference", section.label, referenceStart, referenceEnd };
        section.enabled = enabled;
        restored.push_back(std::move(section));
    }

    return replace(std::move(restored), mixDurationSeconds, referenceDurationSeconds);
}

} // namespace ehvota::refhub
