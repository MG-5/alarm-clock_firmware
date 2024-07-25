#include "wrappers/EventGroup.hpp"

namespace sync
{
constexpr EventBits_t WaitForDisplayInitBit = 1 << 1;
constexpr EventBits_t WaitForLedInit = 1 << 2;
constexpr EventBits_t WaitForRtc = 1 << 3;
} // namespace sync