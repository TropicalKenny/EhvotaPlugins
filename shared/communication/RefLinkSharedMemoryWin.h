#pragma once

#if !defined(_WIN32)
#error RefLinkSharedMemoryWin is a Windows-only product backend.
#endif

#include "SharedLinkProtocol.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ehvota::communication {

enum class SharedLinkOpenStatus : std::uint8_t {
    unavailable,
    ready,
    versionMismatch
};

struct SharedInstanceSnapshot {
    SharedInstanceId id;
    SharedPluginKind pluginKind { SharedPluginKind::unknown };
    RefLinkRole role { RefLinkRole::unknown };
    std::array<char, sharedLinkNameCapacity> name {};
    RefLinkMetrics metrics {};
    std::uint64_t heartbeatAgeMs {};
    SharedProposalMailbox mailbox {};
};

class RefLinkSharedMemoryWin {
public:
    RefLinkSharedMemoryWin() noexcept;
    ~RefLinkSharedMemoryWin();

    RefLinkSharedMemoryWin(const RefLinkSharedMemoryWin&) = delete;
    RefLinkSharedMemoryWin& operator=(const RefLinkSharedMemoryWin&) = delete;

    [[nodiscard]] SharedLinkOpenStatus status() const noexcept { return status_; }
    [[nodiscard]] bool ready() const noexcept { return status_ == SharedLinkOpenStatus::ready; }

    [[nodiscard]] SharedInstanceId registerInstance(
        SharedPluginKind pluginKind,
        RefLinkRole role,
        std::string_view name) noexcept;
    void unregisterInstance(SharedInstanceId id) noexcept;
    bool publish(SharedInstanceId id, const RefLinkMetrics& metrics) noexcept;

    [[nodiscard]] std::size_t readActive(
        std::array<SharedInstanceSnapshot, sharedLinkMaxInstances>& destination) noexcept;

    [[nodiscard]] bool submitProposal(
        SharedInstanceId target,
        SharedProposalMailbox proposal) noexcept;
    [[nodiscard]] std::optional<SharedProposalMailbox> consumePendingProposal(
        SharedInstanceId target) noexcept;
    [[nodiscard]] bool completeProposal(
        SharedInstanceId target,
        std::uint64_t sequence,
        SharedProposalState finalState,
        std::uint32_t resultCode = 0) noexcept;

private:
    bool lock(std::uint32_t timeoutMs) noexcept;
    void unlock() noexcept;
    [[nodiscard]] std::uint64_t nowTickMs() const noexcept;
    [[nodiscard]] bool recordMatches(const SharedInstanceRecord& record, SharedInstanceId id) const noexcept;

    void* mappingHandle_ {};
    void* mutexHandle_ {};
    SharedLinkRegion* region_ {};
    SharedLinkOpenStatus status_ { SharedLinkOpenStatus::unavailable };
};

} // namespace ehvota::communication
