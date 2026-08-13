#include "RefLinkSharedMemoryWin.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <type_traits>

namespace ehvota::communication {
namespace {
constexpr wchar_t mappingName[] = L"Local\\EhvotaRefLink_v5";
constexpr wchar_t mutexName[] = L"Local\\EhvotaRefLinkMutex_v5";
constexpr std::uint64_t staleAfterMs = 10000;
HANDLE asHandle(void* value) noexcept { return static_cast<HANDLE>(value); }
void copyName(std::array<char, sharedLinkNameCapacity>& destination, std::string_view source) noexcept
{
    destination.fill('\0');
    std::copy_n(source.begin(), std::min(source.size(), destination.size() - 1), destination.begin());
}
bool captureTransitionAllowed(const SharedCaptureState from, const SharedCaptureState to) noexcept
{
    if (from == to) return true;
    if (to == SharedCaptureState::rejected || to == SharedCaptureState::error)
        return sharedCaptureStateInFlight(from);
    if (from == SharedCaptureState::armed && to == SharedCaptureState::capturing) return true;
    if (from == SharedCaptureState::capturing && to == SharedCaptureState::analysing) return true;
    if (from == SharedCaptureState::analysing && to == SharedCaptureState::complete) return true;
    return false;
}
} // namespace

static_assert(std::is_trivially_copyable_v<SharedLinkRegion>);

RefLinkSharedMemoryWin::RefLinkSharedMemoryWin() noexcept
{
    mutexHandle_ = CreateMutexW(nullptr, FALSE, mutexName);
    if (!mutexHandle_) return;
    mappingHandle_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
        static_cast<DWORD>(sizeof(SharedLinkRegion)), mappingName);
    if (!mappingHandle_) return;
    const auto fresh = GetLastError() != ERROR_ALREADY_EXISTS;
    region_ = static_cast<SharedLinkRegion*>(MapViewOfFile(
        asHandle(mappingHandle_), FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedLinkRegion)));
    if (!region_ || !lock(100)) return;
    if (fresh) {
        std::memset(region_, 0, sizeof(SharedLinkRegion));
        region_->magic = sharedLinkMagic;
        region_->version = sharedLinkVersion;
        region_->regionBytes = static_cast<std::uint32_t>(sizeof(SharedLinkRegion));
        region_->revision = 1;
        status_ = SharedLinkOpenStatus::ready;
    } else if (region_->magic != sharedLinkMagic
        || region_->regionBytes != static_cast<std::uint32_t>(sizeof(SharedLinkRegion))
        || region_->version != sharedLinkVersion) {
        status_ = SharedLinkOpenStatus::versionMismatch;
    } else {
        status_ = SharedLinkOpenStatus::ready;
    }
    unlock();
}

RefLinkSharedMemoryWin::~RefLinkSharedMemoryWin()
{
    if (region_) UnmapViewOfFile(region_);
    if (mappingHandle_) CloseHandle(asHandle(mappingHandle_));
    if (mutexHandle_) CloseHandle(asHandle(mutexHandle_));
}

bool RefLinkSharedMemoryWin::lock(const std::uint32_t timeoutMs) noexcept
{
    if (!mutexHandle_) return false;
    const auto result = WaitForSingleObject(asHandle(mutexHandle_), timeoutMs);
    return result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
}
void RefLinkSharedMemoryWin::unlock() noexcept { if (mutexHandle_) ReleaseMutex(asHandle(mutexHandle_)); }
std::uint64_t RefLinkSharedMemoryWin::nowTickMs() const noexcept { return static_cast<std::uint64_t>(GetTickCount64()); }
bool RefLinkSharedMemoryWin::recordMatches(const SharedInstanceRecord& record, const SharedInstanceId id) const noexcept
{
    return region_ && id.valid() && id.slot < region_->instances.size()
        && record.state == SharedSlotState::active && record.generation == id.generation;
}

SharedInstanceId RefLinkSharedMemoryWin::registerInstance(
    const SharedPluginKind kind, const RefLinkRole role, const std::string_view name) noexcept
{
    if (!ready() || !region_ || !lock(10)) return {};
    const auto now = nowTickMs();
    SharedInstanceId output;
    for (std::size_t i = 0; i < region_->instances.size(); ++i) {
        auto& record = region_->instances[i];
        const bool stale = record.state == SharedSlotState::active && now >= record.heartbeatTickMs
            && (now - record.heartbeatTickMs) > staleAfterMs;
        if (record.state != SharedSlotState::free && !stale) continue;
        const auto generation = record.generation + 1;
        record = {};
        record.state = SharedSlotState::active;
        record.generation = generation ? generation : 1;
        record.pluginKind = kind;
        record.role = role;
        copyName(record.name, name);
        record.metrics.peakDb = -100.0f;
        record.metrics.rmsDb = -100.0f;
        record.metrics.correlation = 1.0f;
        record.heartbeatTickMs = now;
        record.mailbox.state = SharedProposalState::empty;
        record.captureRequest.state = SharedCaptureState::empty;
        ++region_->revision;
        output = { static_cast<std::uint16_t>(i), record.generation };
        break;
    }
    unlock();
    return output;
}

void RefLinkSharedMemoryWin::unregisterInstance(const SharedInstanceId id) noexcept
{
    if (!ready() || !region_ || !id.valid() || id.slot >= region_->instances.size() || !lock(10)) return;
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
    const SharedInstanceId id, const RefLinkMetrics& metrics, const SharedParameterSnapshot* parameters) noexcept
{
    if (!ready() || !region_ || !id.valid() || id.slot >= region_->instances.size() || !lock(5)) return false;
    auto& record = region_->instances[id.slot];
    bool ok = recordMatches(record, id);
    if (ok && parameters && !validateSharedParameterSnapshot(record.pluginKind, *parameters)) ok = false;
    if (ok) {
        record.metrics = metrics;
        if (parameters) {
            const auto next = record.parameters.revision + 1;
            record.parameters = *parameters;
            record.parameters.revision = next ? next : 1;
        }
        record.heartbeatTickMs = nowTickMs();
        ++region_->revision;
    }
    unlock();
    return ok;
}

std::size_t RefLinkSharedMemoryWin::readActive(
    std::array<SharedInstanceSnapshot, sharedLinkMaxInstances>& destination) noexcept
{
    if (!ready() || !region_ || !lock(10)) return 0;
    const auto now = nowTickMs();
    std::size_t count = 0;
    for (std::size_t i = 0; i < region_->instances.size() && count < destination.size(); ++i) {
        auto& record = region_->instances[i];
        if (record.state != SharedSlotState::active) continue;
        SharedInstanceSnapshot snapshot;
        snapshot.id = { static_cast<std::uint16_t>(i), record.generation };
        snapshot.pluginKind = record.pluginKind;
        snapshot.role = record.role;
        snapshot.name = record.name;
        snapshot.metrics = record.metrics;
        snapshot.parameters = record.parameters;
        snapshot.heartbeatAgeMs = now >= record.heartbeatTickMs ? now - record.heartbeatTickMs : 0;
        snapshot.mailbox = record.mailbox;
        snapshot.captureRequest = record.captureRequest;
        snapshot.capturedAnalysis = record.capturedAnalysis;
        destination[count++] = snapshot;
    }
    unlock();
    return count;
}

bool RefLinkSharedMemoryWin::submitProposal(const SharedInstanceId target, SharedProposalMailbox proposal) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return false;
    auto& record = region_->instances[target.slot];
    bool ok = false;
    if (recordMatches(record, target) && record.pluginKind != SharedPluginKind::unknown
        && validateSharedProposal(record.pluginKind, proposal) && proposal.targetGeneration == target.generation
        && record.mailbox.state != SharedProposalState::pending && record.mailbox.state != SharedProposalState::processing) {
        proposal.sequence = record.mailbox.sequence + 1;
        proposal.state = SharedProposalState::pending;
        proposal.resultCode = 0;
        record.mailbox = proposal;
        ++region_->revision;
        ok = true;
    }
    unlock();
    return ok;
}

std::optional<SharedProposalMailbox> RefLinkSharedMemoryWin::consumePendingProposal(const SharedInstanceId target) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return std::nullopt;
    std::optional<SharedProposalMailbox> output;
    auto& record = region_->instances[target.slot];
    if (recordMatches(record, target) && record.mailbox.state == SharedProposalState::pending
        && record.mailbox.targetGeneration == target.generation) {
        record.mailbox.state = SharedProposalState::processing;
        ++region_->revision;
        output = record.mailbox;
    }
    unlock();
    return output;
}

bool RefLinkSharedMemoryWin::completeProposal(
    const SharedInstanceId target, const std::uint64_t sequence,
    const SharedProposalState state, const std::uint32_t resultCode) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return false;
    if (state != SharedProposalState::applied && state != SharedProposalState::rejected && state != SharedProposalState::error) {
        unlock();
        return false;
    }
    auto& record = region_->instances[target.slot];
    const bool ok = recordMatches(record, target) && record.mailbox.state == SharedProposalState::processing
        && record.mailbox.sequence == sequence;
    if (ok) {
        record.mailbox.state = state;
        record.mailbox.resultCode = resultCode;
        ++region_->revision;
    }
    unlock();
    return ok;
}

bool RefLinkSharedMemoryWin::submitCaptureRequest(
    const SharedInstanceId target, SharedCaptureRequest request) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return false;
    auto& record = region_->instances[target.slot];
    bool ok = false;
    if (recordMatches(record, target) && record.pluginKind == SharedPluginKind::refTap
        && request.targetGeneration == target.generation && validateSharedCaptureRequest(request)
        && !sharedCaptureStateInFlight(record.captureRequest.state)) {
        request.sequence = record.captureRequest.sequence + 1;
        request.state = SharedCaptureState::pending;
        request.resultCode = 0;
        record.captureRequest = request;
        ++region_->revision;
        ok = true;
    }
    unlock();
    return ok;
}

std::optional<SharedCaptureRequest> RefLinkSharedMemoryWin::consumePendingCaptureRequest(
    const SharedInstanceId target) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return std::nullopt;
    std::optional<SharedCaptureRequest> output;
    auto& record = region_->instances[target.slot];
    if (recordMatches(record, target) && record.pluginKind == SharedPluginKind::refTap
        && record.captureRequest.state == SharedCaptureState::pending
        && record.captureRequest.targetGeneration == target.generation) {
        record.captureRequest.state = SharedCaptureState::armed;
        ++region_->revision;
        output = record.captureRequest;
    }
    unlock();
    return output;
}

bool RefLinkSharedMemoryWin::updateCaptureState(
    const SharedInstanceId target, const std::uint64_t sequence,
    const SharedCaptureState state, const std::uint32_t resultCode) noexcept
{
    if (!ready() || !region_ || !target.valid() || target.slot >= region_->instances.size() || !lock(10)) return false;
    auto& record = region_->instances[target.slot];
    bool ok = recordMatches(record, target) && record.pluginKind == SharedPluginKind::refTap
        && record.captureRequest.sequence == sequence
        && captureTransitionAllowed(record.captureRequest.state, state);
    if (ok && state == SharedCaptureState::complete) {
        ok = record.capturedAnalysis.valid == 1
            && record.capturedAnalysis.captureId == record.captureRequest.captureId
            && record.capturedAnalysis.sourceGeneration == target.generation;
    }
    if (ok) {
        record.captureRequest.state = state;
        record.captureRequest.resultCode = resultCode;
        ++region_->revision;
    }
    unlock();
    return ok;
}

bool RefLinkSharedMemoryWin::publishCapturedAnalysis(
    const SharedInstanceId id, SharedCapturedAnalysis analysis) noexcept
{
    if (!ready() || !region_ || !id.valid() || id.slot >= region_->instances.size() || !lock(10)) return false;
    auto& record = region_->instances[id.slot];
    bool ok = recordMatches(record, id) && record.pluginKind == SharedPluginKind::refTap
        && analysis.sourceGeneration == id.generation && validateSharedCapturedAnalysis(analysis)
        && record.captureRequest.captureId == analysis.captureId
        && (record.captureRequest.state == SharedCaptureState::analysing
            || record.captureRequest.state == SharedCaptureState::capturing);
    if (ok) {
        const auto next = record.capturedAnalysis.revision + 1;
        record.capturedAnalysis = analysis;
        record.capturedAnalysis.revision = next ? next : 1;
        ++region_->revision;
    }
    unlock();
    return ok;
}

} // namespace ehvota::communication
