#pragma once

#include <cstddef>

#include "InputApi.h"

namespace DekiInput
{

/// Lets one component own the current pointer gesture. A component calls
/// ClaimGesture() to take it; others check IsGestureClaimed() before acting
/// on input and back off when someone else owns it.
///
/// Example: ScrollComponent claims the gesture once a drag passes its
/// threshold, so nested scrolls and child components stop handling it.
class DEKI_INPUT_API InputDispatch
{
public:
    static void ClaimGesture(void* owner) { s_GestureOwner = owner; }
    static void ReleaseGesture() { s_GestureOwner = nullptr; }
    static bool IsGestureClaimed() { return s_GestureOwner != nullptr; }
    static bool IsGestureClaimedBy(void* owner) { return s_GestureOwner == owner; }

private:
    static void* s_GestureOwner;
};

}  // namespace DekiInput
