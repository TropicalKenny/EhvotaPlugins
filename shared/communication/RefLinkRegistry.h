#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ehvota::communication {

enum class RefLinkRole : std::uint8_t {
    unknown,
    master,
    drums,
    bass,
    vocals,
    music,
    bus,
    track
};

struct RefLinkHandle {
    std::uint16_t slot { 0xffff };
    std::uint32_t generation {};

    [[nodiscard]] bool valid() const noexcept { return slot != 0xffff; }
};

struct RefLinkMetrics {
    float peakDb {-100.0f};
    float rmsDb {-100.0f};
    float correlation {};
    float crestDb {};
    std::uint64_t sampleCounter {};
};

struct RefLinkInstanceSnapshot {
    RefLinkHandle handle;
    RefLinkRole role { RefLinkRole::unknown };
    std::array<char, 48> name {};
    RefLinkMetrics metrics;
};

class RefLinkRegistry {
public:
    static constexpr std::size_t maxInstances = 64;

    static RefLinkRegistry& instance() noexcept;

    [[nodiscard]] RefLinkHandle registerInstance(RefLinkRole role, std::string_view name) noexcept;
    void unregisterInstance(RefLinkHandle handle) noexcept;

    // Audio-thread safe: fixed-cost atomic stores, no allocation and no locks.
    void publish(RefLinkHandle handle, const RefLinkMetrics& metrics) noexcept;

    [[nodiscard]] std::optional<RefLinkInstanceSnapshot> read(RefLinkHandle handle) const noexcept;
    [[nodiscard]] std::size_t readActive(
        std::array<RefLinkInstanceSnapshot, maxInstances>& destination) const noexcept;

private:
    enum class SlotState : std::uint8_t { free, initializing, active };

    struct Slot {
        std::atomic<SlotState> state { SlotState::free };
        std::atomic<std::uint32_t> generation { 0 };
        RefLinkRole role { RefLinkRole::unknown };
        std::array<char, 48> name {};
        std::atomic<float> peakDb { -100.0f };
        std::atomic<float> rmsDb { -100.0f };
        std::atomic<float> correlation {};
        std::atomic<float> crestDb {};
        std::atomic<std::uint64_t> sampleCounter {};
    };

    std::array<Slot, maxInstances> slots_ {};
};

} // namespace ehvota::communication
