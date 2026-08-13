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

enum class SharedLinkOpenStatus : std::uint8_t { unavailable, ready, versionMismatch };

struct SharedInstanceSnapshot {
    SharedInstanceId id;
    SharedPluginKind pluginKind { SharedPluginKind::unknown };
    RefLinkRole role { RefLinkRole::unknown };
    std::array<char, sharedLinkNameCapacity> name {};
    RefLinkMetrics metrics {};
    SharedParameterSnapshot parameters {};
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
    [[nodiscard]] SharedInstanceId registerInstance(SharedPluginKind, RefLinkRole, std::string_view) noexcept;
    void unregisterInstance(SharedInstanceId) noexcept;
    bool publish(SharedInstanceId, const RefLinkMetrics&, const SharedParameterSnapshot* parameters = nullptr) noexcept;
    [[nodiscard]] std::size_t readActive(std::array<SharedInstanceSnapshot, sharedLinkMaxInstances>&) noexcept;
    [[nodiscard]] bool submitProposal(SharedInstanceId, SharedProposalMailbox) noexcept;
    [[nodiscard]] std::optional<SharedProposalMailbox> consumePendingProposal(SharedInstanceId) noexcept;
    [[nodiscard]] bool completeProposal(SharedInstanceId, std::uint64_t, SharedProposalState, std::uint32_t resultCode = 0) noexcept;

private:
    bool lock(std::uint32_t timeoutMs) noexcept;
    void unlock() noexcept;
    [[nodiscard]] std::uint64_t nowTickMs() const noexcept;
    [[nodiscard]] bool recordMatches(const SharedInstanceRecord&, SharedInstanceId) const noexcept;

    void* mappingHandle_ {};
    void* mutexHandle_ {};
    SharedLinkRegion* region_ {};
    SharedLinkOpenStatus status_ { SharedLinkOpenStatus::unavailable };
};

} // namespace ehvota::communication
