#pragma once

#include "RefLinkRegistry.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ehvota::communication {

constexpr std::uint32_t sharedLinkMagic = 0x4556484cU; // EVHL
constexpr std::uint32_t sharedLinkVersion = 3U;
constexpr std::size_t sharedLinkMaxInstances = 64;
constexpr std::size_t sharedLinkMaxOperations = 8;
constexpr std::size_t sharedLinkNameCapacity = 48;

enum class SharedPluginKind : std::uint16_t {
    unknown,
    refHub,
    refTap,
    refEQ,
    refComp,
    refTime,
    refColor,
    refTransient
};

enum class SharedSlotState : std::uint32_t { free, active };

enum class SharedParameterKey : std::uint16_t {
    none,
    eqHighPassEnabled, eqHighPassFrequency, eqLowPassEnabled, eqLowPassFrequency,
    eqBand1Frequency, eqBand1Gain, eqBand1Q,
    eqBand2Frequency, eqBand2Gain, eqBand2Q,
    eqBand3Frequency, eqBand3Gain, eqBand3Q,
    eqBand4Frequency, eqBand4Gain, eqBand4Q,
    eqBand5Frequency, eqBand5Gain, eqBand5Q,
    eqBand6Frequency, eqBand6Gain, eqBand6Q, eqOutput, eqMode,
    compMode, compThreshold, compRatio, compAttack, compAttackSync, compAttackNote,
    compAttackModifier, compRelease, compReleaseSync, compReleaseNote,
    compReleaseModifier, compKnee, compMakeup, compMix,
    colorMode, colorAmount, colorDrive, colorAsymmetry, colorOutput,
    transientAttack, transientSustain, transientSensitivity, transientOutput
};

enum class SharedProposalState : std::uint32_t { empty, pending, processing, applied, rejected, error };

struct SharedInstanceId {
    std::uint16_t slot { 0xffff };
    std::uint32_t generation {};
    [[nodiscard]] bool valid() const noexcept { return slot != 0xffff; }
};

struct SharedParameterOperation {
    SharedParameterKey key { SharedParameterKey::none };
    float expectedCurrentValue {};
    float expectedTolerance { 0.001f };
    float value {};
};

struct SharedProposalMailbox {
    std::uint64_t sequence {};
    std::uint64_t proposalId {};
    std::uint32_t targetGeneration {};
    SharedProposalState state { SharedProposalState::empty };
    std::uint32_t operationCount {};
    std::uint32_t resultCode {};
    std::array<SharedParameterOperation, sharedLinkMaxOperations> operations {};
};

struct SharedInstanceRecord {
    SharedSlotState state { SharedSlotState::free };
    std::uint32_t generation {};
    SharedPluginKind pluginKind { SharedPluginKind::unknown };
    RefLinkRole role { RefLinkRole::unknown };
    std::array<char, sharedLinkNameCapacity> name {};
    RefLinkMetrics metrics {};
    std::uint64_t heartbeatTickMs {};
    SharedProposalMailbox mailbox {};
};

struct SharedLinkRegion {
    std::uint32_t magic {};
    std::uint32_t version {};
    std::uint32_t regionBytes {};
    std::uint32_t reserved {};
    std::uint64_t revision {};
    std::array<SharedInstanceRecord, sharedLinkMaxInstances> instances {};
};

struct ParameterBounds { float minimum {}; float maximum {}; bool integerOnly {}; };

[[nodiscard]] std::optional<ParameterBounds> sharedParameterBounds(SharedPluginKind, SharedParameterKey) noexcept;
[[nodiscard]] bool sharedParameterAllowed(SharedPluginKind, SharedParameterKey) noexcept;
[[nodiscard]] bool validateSharedOperation(SharedPluginKind, const SharedParameterOperation&) noexcept;
[[nodiscard]] bool validateSharedProposal(SharedPluginKind, const SharedProposalMailbox&) noexcept;
[[nodiscard]] std::string_view sharedPluginKindName(SharedPluginKind) noexcept;

} // namespace ehvota::communication
