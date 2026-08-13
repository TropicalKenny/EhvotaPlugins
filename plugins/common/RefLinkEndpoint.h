#pragma once

#if !defined(_WIN32)
#error RefLinkEndpoint is used only by Windows product targets.
#endif

#include "shared/communication/RefLinkSharedMemoryWin.h"

#include <array>
#include <string>
#include <string_view>

namespace ehvota::plugins {

class RefLinkEndpoint {
public:
    RefLinkEndpoint(const ehvota::communication::SharedPluginKind pluginKind,const ehvota::communication::RefLinkRole role,const std::string_view name)
        : pluginKind_(pluginKind), role_(role), name_(name) { ensureRegistered(); }
    ~RefLinkEndpoint(){if(id_.valid())transport_.unregisterInstance(id_);}
    RefLinkEndpoint(const RefLinkEndpoint&)=delete;RefLinkEndpoint&operator=(const RefLinkEndpoint&)=delete;
    bool ensureRegistered()noexcept{if(id_.valid())return true;if(!transport_.ready())return false;id_=transport_.registerInstance(pluginKind_,role_,name_);return id_.valid();}
    bool publish(const ehvota::communication::RefLinkMetrics&metrics,const ehvota::communication::SharedParameterSnapshot*parameters=nullptr)noexcept{if(!ensureRegistered())return false;if(transport_.publish(id_,metrics,parameters))return true;id_={};return false;}
    [[nodiscard]]std::size_t readActive(std::array<ehvota::communication::SharedInstanceSnapshot,ehvota::communication::sharedLinkMaxInstances>&destination)noexcept{return transport_.readActive(destination);}
    [[nodiscard]]std::optional<ehvota::communication::SharedProposalMailbox>consumePendingProposal()noexcept{if(!ensureRegistered())return std::nullopt;return transport_.consumePendingProposal(id_);}
    bool completeProposal(std::uint64_t sequence,ehvota::communication::SharedProposalState state,std::uint32_t resultCode=0)noexcept{return id_.valid()&&transport_.completeProposal(id_,sequence,state,resultCode);}
    bool submitProposal(ehvota::communication::SharedInstanceId target,ehvota::communication::SharedProposalMailbox proposal)noexcept{return transport_.submitProposal(target,proposal);}
    [[nodiscard]]ehvota::communication::SharedInstanceId id()const noexcept{return id_;}
    [[nodiscard]]ehvota::communication::SharedLinkOpenStatus status()const noexcept{return transport_.status();}
private:
    ehvota::communication::RefLinkSharedMemoryWin transport_;
    ehvota::communication::SharedPluginKind pluginKind_{ehvota::communication::SharedPluginKind::unknown};
    ehvota::communication::RefLinkRole role_{ehvota::communication::RefLinkRole::unknown};
    std::string name_;
    ehvota::communication::SharedInstanceId id_;
};

} // namespace ehvota::plugins
