#include "shared/safety/AuditionPolicy.h"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}
} // namespace

int main()
{
    using ehvota::safety::AuditionPolicyInput;
    using ehvota::safety::decideReferenceAudition;

    auto realtime = decideReferenceAudition(AuditionPolicyInput{ true, true, true, false });
    require(realtime.playReference, "reference may be auditioned during realtime monitoring");
    require(!realtime.preserveProjectSignal, "realtime reference audition intentionally replaces monitor output");

    auto offline = decideReferenceAudition(AuditionPolicyInput{ true, true, true, true });
    require(!offline.playReference, "Render Safe must suppress reference playback during offline render");
    require(offline.preserveProjectSignal, "Render Safe must preserve the project signal during offline render");
    require(offline.suppressedByRenderSafe, "policy must expose why reference audition was suppressed");

    auto unavailable = decideReferenceAudition(AuditionPolicyInput{ true, false, true, false });
    require(!unavailable.playReference, "missing reference must never replace project audio");
    require(unavailable.preserveProjectSignal, "missing reference must fail safe to project audio");

    std::cout << "Audition policy tests passed\n";
    return 0;
}
