#include "RefLinkSharedMemoryWin.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <type_traits>

namespace ehvota::communication {
namespace {

constexpr wchar_t mappingName[] = L"Local\\EhvotaRefLink_v1";
constexpr wchar_t mutexName[] = L"Local\\EhvotaRefLinkMutex_v1";
constexpr std::uint64_t staleAfterMs = 10000;

HANDLE asHandle(void* value) noexcept
{
    return static_cast<HANDLE>(value);
}

void copyName(std::array<char, sharedLinkNameCapacity>& destination, const std::string_view source) noexcept
{
    destination.fill('\0');
    const auto count = std::min(source.size(), destination.size() - 1);
    std::copy_n(source.begin(), count, destination.begin());
}

} // namespace

static_assert(std::is_trivially_copyable_v<SharedLinkRegion>);

RefLinkSharedMemoryWin::RefLinkSharedMemoryWin() noexcept
{
    mutexHandle_ = CreateMutexW(nullptr, FALSE, mutexName);
    if (mutexHandle_ == nullptr)
        return;

    mappingHandle_ = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        static_cast<DWORD>(sizeof(SharedLinkRegion)),
        mappingName);
    if (mappingHandle_ == nullptr)
        return;

    const auto createdNewMapping = GetLastError() != ERROR_ALREADY_EXISTS;
    region_ = static_cast<SharedLinkRegion*>(MapViewOfFile(
        asHandle(mappingHandle_),
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(SharedLinkRegion)));
    if (region_ == nullptr)
        return;

    if (!lock(100))
        return;

    if (createdNewMapping) {
        std::memset(region_, 0, sizeof(SharedLinkRegion));
        region_->magic = sharedLinkMagic;
        region_->version = sharedLinkVersion;
        region_->regionBytes = static_cast<std::uint32_t>(sizeof(SharedLinkRegion));
        region_->revision = 1;
        status_ = SharedLinkOpenStatus::ready;
    }
    else if (region_->magic != sharedLinkMagic
        || region_->regionBytes != static_cast<std::uint32_t>(sizeof(SharedLinkRegion))
        || region_->version != sharedLinkVersion) {
        status_ = SharedLinkOpenStatus::versionMismatch;
    }
    else {
        status_ = SharedLinkOpenStatus::ready;
    }

    unlock();
}

RefLinkSharedMemoryWin::~RefLinkSharedMemoryWin()
{
    if (region_ != nullptr)
        UnmapViewOfFile(region_);
    if (mappingHandle_ != nullptr)
        CloseHandle(asHandle(mappingHandle_));
    if (mutexHandle_ != nullptr)
        CloseHandle(asHandle(mutexHandle_));
}

bool RefLinkSharedMemoryWin::lock(const std::uint32_t timeoutMs) noexcept
{
    if (mutexHandle_ == nullptr)
        return false;
    const auto result = WaitForSingleObject(asHandle(mutexHandle_), timeoutMs);
    return result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
}

void RefLinkSharedMemoryWin::unlock() noexcept
{
    if (mutexHandle_ != nullptr)
        ReleaseMutex(asHandle(mutexHandle_));
}

std::uint64_t RefLinkSharedMemoryWin::nowTickMs() const noexcept
{
    return static_cast<std::uint64_t>(GetTickCount64());
}

bool RefLinkSharedMemoryWin::recordMatches(
    const SharedInstanceRecord& record,
    const SharedInstanceId id) const noexcept
{
    return region_ != nullptr
        && id.valid()
        && id.slot < region_->instances.size()
        && record.state == SharedSlotState::active
        && record.generation == id.generation;
}

SharedInstanceId RefLinkSharedMemoryWin::registerInstance(
    const SharedPluginKind pluginKind,
    const RefLinkRole role,
    const std::string_view name) noexcept
{
    if (!ready() || region_ == nullptr || !lock(10))
        return {};

    const auto now = nowTickMs();
    SharedInstanceId result;
    for (std::size_t i = 0; i < region_->instances.size(); ++i) {
        auto& record = region_->instances[i];
        const bool stale = record.state == SharedSlotState::active
            && now >= record.heartbeatTickMs
            && (now - record.heartbeatTickMs) > staleAfterMs;
        if (record.state != SharedSlotState::free && !stale)
            continue;

        const auto nextGeneration = record.generation + 1U;
        record = {};
        record.state = SharedSlotState::active;
        record.generation = nextGeneration == 0 ? 1U : nextGeneration;
        record.pluginKind = pluginKind;
        record.role = role;
        copyName(record.name, name);
        record.metrics.peakDb = -100.0f;
        record.metrics.rmsDb = -100.0f;
        record.metrics.correlation = 1.0f;
        record.heartbeatTickMs = now;
        record.mailbox.state = SharedProposalState::empty;
        ++region_->revision;
        result = { static_cast<std::uint16_t>(i), record.generation };
        break;
    }

    unlock();
    return result;
}

void RefLinkSharedMemoryWin::unregisterInstance(const SharedInstanceId id) noexcept
{
    if (!ready() || region_ == nullptr || !id.valid() || id.slot >= region_->instances.size() || !lock(10))
        return;
    auto& record = region_->instances[id.slot];
    if (recordMatches(record, id)) {
        const auto generation = record.generation;
        record = {};
        record.generation = generation;
        record.state = SharedSlotState::free;
        ++region_->revision;
    }
    unlock();
}

bool RefLinkSharedMemoryWin::publish(
    const SharedInstanceId id,
    const RefLinkMetrics& metrics) noexcept
{
    if (!ready() || region_ == nullptr || !id.valid() || id.slot >= region_->instances.size() || !lock(5))
        return false;
    auto& record = region_->instances[id.slot];
    const auto valid = recordMatches(record, id);
    if (valid) {
        record.metrics = metrics;
        record.heartbeatTickMs = nowTickMs();
        ++region_->revision;
    }
    unlock();
    return valid;
}

std::size_t RefLinkSharedMemoryWin::readActive(
    std::array<SharedInstanceSnapshot, sharedLinkMaxInstances>& destination) noexcept
{
    if (!ready() || region_ == nullptr || !lock(10))
        return 0;

    const auto now = nowTickMs();
    std::size_t count = 0;
    for (std::size_t i = 0; i < region_->instances.size() && count < destination.size(); ++i) {
        const auto& record = region_->instances[i];
        if (record.state != SharedSlotState::active)
            continue;

        SharedInstanceSnapshot snapshot;
        snapshot.id = { static_cast<std::uint16_t>(i), record.generation };
        snapshot.pluginKind = record.pluginKind;
        snapshot.role = record.role;
        snapshot.name = record.name;
        snapshot.metrics = record.metrics;
        snapshot.heartbeatAgeMs = now >= record.heartbeatTickMs ? now - record.heartbeatTickMs : 0;
        snapshot.mailbox = record.mailbox;
        destination[count++] = snapshot;
    }

    unlock();
    return count;
}

bool RefLinkSharedMemoryWin::submitProposal(
    const SharedInstanceId target,
    SharedProposalMailbox proposal) noexcept
{
    if (!ready() || region_ == nullptr || !target.valid() || target.slot >= region_->instances.size() || !lock(10))
        return false;

    auto& record = region_->instances[target.slot];
    bool accepted = false;
    if (recordMatches(record, target)
        && record.pluginKind != SharedPluginKind::unknown
        && validateSharedProposal(record.pluginKind, proposal)
        && proposal.targetGeneration == target.generation
        && record.mailbox.state != SharedProposalState::pending
        && record.mailbox.state != SharedProposalState::processing) {
        proposal.sequence = record.mailbox.sequence + 1U;
        proposal.state = SharedProposalState::pending;
        proposal.resultCode = 0;
        record.mailbox = proposal;
        ++region_->revision;
        accepted = true;
    }

    unlock();
    return accepted;
}

std::optional<SharedProposalMailbox> RefLinkSharedMemoryWin::consumePendingProposal(
    const SharedInstanceId target) noexcept
{
    if (!ready() || region_ == nullptr || !target.valid() || target.slot >= region_->instances.size() || !lock(10))
        return std::nullopt;

    std::optional<SharedProposalMailbox> result;
    auto& record = region_->instances[target.slot];
    if (recordMatches(record, target)
        && record.mailbox.state == SharedProposalState::pending
        && record.mailbox.targetGeneration == target.generation) {
        record.mailbox.state = SharedProposalState::processing;
        ++region_->revision;
        result = record.mailbox;
    }

    unlock();
    return result;
}

bool RefLinkSharedMemoryWin::completeProposal(
    const SharedInstanceId target,
    const std::uint64_t sequence,
    const SharedProposalState finalState,
    const std::uint32_t resultCode) noexcept
{
    if (!ready() || region_ == nullptr || !target.valid() || target.slot >= region_->instances.size() || !lock(10))
        return false;
    if (finalState != SharedProposalState::applied
        && finalState != SharedProposalState::rejected
        && finalState != SharedProposalState::error) {
        unlock();
        return false;
    }

    auto& record = region_->instances[target.slot];
    const auto valid = recordMatches(record, target)
        && record.mailbox.state == SharedProposalState::processing
        && record.mailbox.sequence == sequence;
    if (valid) {
        record.mailbox.state = finalState;
        record.mailbox.resultCode = resultCode;
        ++region_->revision;
    }
    unlock();
    return valid;
}

} // namespace ehvota::communication
