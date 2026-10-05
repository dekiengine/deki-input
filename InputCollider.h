#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

#include <deki/Component.h>
#include <deki/reflection/Property.h>

namespace Deki
{
class Object;
}

namespace DekiInput
{

/// Hit area for pointer and touch input. Like Unity's Collider2D, it defines
/// an area that can be clicked or hovered and fires callbacks on pointer
/// events. Other components (Deki2D::ButtonComponent, ScrollComponent and so
/// on) register callbacks to react to input.
///
/// Coordinates are in WORLD UNITS (float). The dispatch system converts device
/// pixels to world units once, at the input boundary, so colliders work under
/// any camera PPM or zoom.
///
/// Subclass and override HitTest() for other shapes (circle, polygon).
///
/// Usage:
///
///     auto* collider = obj->AddComponent<InputCollider>();
///     collider->width = 100.0f;
///     collider->height = 40.0f;
///     collider->onPointerDown.push_back([](float x, float y) {
///         // Handle the press (x/y in world units)
///     });
DEKI_CATEGORY("Input")
DEKI_DESCRIPTION("Hit area for pointer and touch. Buttons and scrolls listen to it.")
class InputCollider : public Deki::Component
{
public:
    // Hit area dimensions (meters)
    DEKI_EXPORT
    DEKI_TOOLTIP("Width of the hit area in meters. It does not have to match what is drawn.")
    DEKI_UNIT(Distance)
    float width = 0.0f;

    DEKI_EXPORT
    DEKI_TOOLTIP("Height of the hit area in meters.")
    DEKI_UNIT(Distance)
    float height = 0.0f;

    // Hit area padding (meters, expands the hit area beyond width/height)
    DEKI_EXPORT
    DEKI_TOOLTIP("Extra hit area beyond the left edge, in meters. Useful for making a small control comfortable to hit "
                 "with a finger without making it look bigger.")
    DEKI_UNIT(Distance)
    float paddingLeft = 0.0f;

    DEKI_EXPORT
    DEKI_TOOLTIP("Extra hit area beyond the right edge, in meters.")
    DEKI_UNIT(Distance)
    float paddingRight = 0.0f;

    DEKI_EXPORT
    DEKI_TOOLTIP("Extra hit area above the top edge, in meters.")
    DEKI_UNIT(Distance)
    float paddingTop = 0.0f;

    DEKI_EXPORT
    DEKI_TOOLTIP("Extra hit area below the bottom edge, in meters.")
    DEKI_UNIT(Distance)
    float paddingBottom = 0.0f;

    // When true, blocks input from reaching child objects
    DEKI_EXPORT
    DEKI_TOOLTIP("Stop the press here instead of letting it reach objects underneath. Turn it off for an overlay that "
                 "should not block what is behind it.")
    bool consumeInput = true;

    // --- Pointer event callbacks ---
    // Components register these in Start() to react to input

    using PointerCallback = std::function<void(float x, float y)>;

    std::vector<PointerCallback> onPointerDown;
    std::vector<PointerCallback> onPointerUp;
    std::vector<PointerCallback> onPointerEnter;
    std::vector<PointerCallback> onPointerExit;
    std::vector<PointerCallback> onPointerMove;

    // --- Public API ---

    InputCollider();

    /// True when the world-space point (x, y) is inside the collider. The
    /// default is an axis-aligned box with padding; override it for other
    /// shapes.
    virtual bool HitTest(float x, float y) const;

    /// Called by DekiInputSystem with a point in world units: hit-tests it,
    /// tracks pointer state and fires the callbacks. Returns true when the
    /// event was handled (the hit test passed and callbacks fired).
    bool ProcessInput(float x, float y, bool down, bool move, bool up);

    /// Resets pressed and hover state and fires onPointerUp/onPointerExit so
    /// components can clean up. ScrollComponent calls it when it takes a drag
    /// gesture, to cancel its children's interactions.
    void CancelInput();

    // --- State queries ---
    bool IsPointerInside() const { return m_PointerInside; }
    bool IsPressed() const { return m_Pressed; }

private:
    bool m_PointerInside = false;
    bool m_Pressed = false;

    void InvokeCallbacks(const std::vector<PointerCallback>& callbacks, float x, float y);
};

}  // namespace DekiInput
