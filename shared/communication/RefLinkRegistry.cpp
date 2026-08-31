#include "RefLinkRegistry.h"

#include <algorithm>

namespace ehvota::communication {

RefLinkRegistry& RefLinkRegistry::instance() noexcept
{
    static RefLinkRegistry registry;
    return registry;
}

RefLinkHandle RefLinkRegistry::registerInstance(RefLinkRole role, std::string_view name) noexcept
{
    for (std::size_t i = 0; i < slots_.size(); ++i) {
        auto& slot = slots_[i];
        auto expected = SlotState::free;
        if (!slot.state.compare_exchange_strong(expected, SlotState::initializing, std::memory_order_acq_rel))
            continue;

        const auto generation = slot.generation.fetch_add(1, std::memory_order_acq_rel) + 1;
        slot.role = role;
        slot.name.fill('\0');
        const auto count = std::min(name.size(), slot.name.size() - 1);
        std::copy_n(name.begin(), count, slot.name.begin());
        slot.peakDb.store(-100.0f, std::memory_order_relaxed);
        slot.rmsDb.store(-100.0f, std::memory_order_relaxed);
        slot.correlation.store(0.0f, std::memory_order_relaxed);
        slot.crestDb.store(0.0f, std::memory_order_relaxed);
        slot.sampleCounter.store(0, std::memory_order_relaxed);
        slot.state.store(SlotState::active, std::memory_order_release);
        return { static_cast<std::uint16_t>(i), generation };
    }
    return {};
}

void RefLinkRegistry::unregisterInstance(RefLinkHandle handle) noexcept
{
    if (!handle.valid() || handle.slot >= slots_.size())
        return;
    auto& slot = slots_[handle.slot];
    if (slot.state.load(std::memory_order_acquire) != SlotState::active
        || slot.generation.load(std::memory_order_acquire) != handle.generation)
        return;
    slot.state.store(SlotState::free, std::memory_order_release);
}

void RefLinkRegistry::publish(RefLinkHandle handle, const RefLinkMetrics& metrics) noexcept
{
    if (!handle.valid() || handle.slot >= slots_.size())
        return;
    auto& slot = slots_[handle.slot];
    if (slot.state.load(std::memory_order_acquire) != SlotState::active
        || slot.generation.load(std::memory_order_acquire) != handle.generation)
        return;

    slot.peakDb.store(metrics.peakDb, std::memory_order_relaxed);
    slot.rmsDb.store(metrics.rmsDb, std::memory_order_relaxed);
    slot.correlation.store(metrics.correlation, std::memory_order_relaxed);
    slot.crestDb.store(metrics.crestDb, std::memory_order_relaxed);
    slot.sampleCounter.store(metrics.sampleCounter, std::memory_order_release);
}

std::optional<RefLinkInstanceSnapshot> RefLinkRegistry::read(RefLinkHandle handle) const noexcept
{
    if (!handle.valid() || handle.slot >= slots_.size())
        return std::nullopt;
    const auto& slot = slots_[handle.slot];
    if (slot.state.load(std::memory_order_acquire) != SlotState::active
        || slot.generation.load(std::memory_order_acquire) != handle.generation)
        return std::nullopt;

    RefLinkInstanceSnapshot snapshot;
    snapshot.handle = handle;
    snapshot.role = slot.role;
    snapshot.name = slot.name;
    snapshot.metrics.peakDb = slot.peakDb.load(std::memory_order_relaxed);
    snapshot.metrics.rmsDb = slot.rmsDb.load(std::memory_order_relaxed);
    snapshot.metrics.correlation = slot.correlation.load(std::memory_order_relaxed);
    snapshot.metrics.crestDb = slot.crestDb.load(std::memory_order_relaxed);
    snapshot.metrics.sampleCounter = slot.sampleCounter.load(std::memory_order_acquire);
    return snapshot;
}

std::size_t RefLinkRegistry::readActive(
    std::array<RefLinkInstanceSnapshot, maxInstances>& destination) const noexcept
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < slots_.size() && count < destination.size(); ++i) {
        const auto& slot = slots_[i];
        if (slot.state.load(std::memory_order_acquire) != SlotState::active)
            continue;
        const RefLinkHandle handle { static_cast<std::uint16_t>(i), slot.generation.load(std::memory_order_acquire) };
        if (auto snapshot = read(handle))
            destination[count++] = *snapshot;
    }
    return count;
}

} // namespace ehvota::communication
