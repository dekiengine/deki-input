#pragma once

#include "DekiEngine.h"

/**
 * @brief Gesture claim system for input dispatch
 *
 * Components call ClaimGesture() to take ownership of the current
 * pointer gesture. Other components check IsGestureClaimed() before
 * acting on input, and back off if another component owns the gesture.
 *
 * Example: ScrollComponent claims the gesture when drag threshold is
 * exceeded, preventing nested scrolls and child components from
 * processing further input.
 */
class DEKI_ENGINE_API InputDispatch
{
public:
    static void ClaimGesture(void* owner) { s_GestureOwner = owner; }
    static void ReleaseGesture() { s_GestureOwner = nullptr; }
    static bool IsGestureClaimed() { return s_GestureOwner != nullptr; }
    static bool IsGestureClaimedBy(void* owner) { return s_GestureOwner == owner; }

private:
    static void* s_GestureOwner;
};
