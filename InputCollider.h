#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

#include <deki/Component.h>
#include <deki/reflection/Property.h>
#include "deki-2d/Bounds2D.h"

namespace Deki { class Object; }

/**
 * @brief Hit area component for pointer/touch input
 *
 * Like Unity's Collider2D — defines a clickable/hoverable area and fires
 * callbacks when pointer events occur. Other components (ButtonComponent,
 * ScrollComponent, etc.) register callbacks to react to input.
 *
 * Coordinates are in WORLD UNITS (float). The dispatch system converts raw
 * device pixels → world units once at the input boundary, so colliders work
 * correctly under any camera PPM/zoom.
 *
 * Subclass and override HitTest() for custom shapes (circle, polygon, etc.).
 *
 * Usage:
 * @code
 * // Add InputCollider to make an object interactive
 * auto* collider = obj->AddComponent<InputCollider>();
 * collider->width = 100.0f;
 * collider->height = 40.0f;
 *
 * // Register callbacks
 * collider->onPointerDown.push_back([](float x, float y) {
 *     // Handle press (x/y in world units)
 * });
 * @endcode
 */
class InputCollider : public Deki::Component
{
    DEKI_COMPONENT(InputCollider, Deki::Component, "Input", "a1b2c3d4-e5f6-7890-abcd-ef1234567890", "DEKI_FEATURE_INPUT")
    DEKI_DESCRIPTION("Hit area for pointer and touch. Buttons and scrolls listen to it.")

public:
    // Hit area dimensions (meters)
    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float width = 0.0f;

    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float height = 0.0f;

    // Hit area padding (meters, expands the hit area beyond width/height)
    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float paddingLeft = 0.0f;

    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float paddingRight = 0.0f;

    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float paddingTop = 0.0f;

    DEKI_EXPORT
    DEKI_UNIT(Distance)
    float paddingBottom = 0.0f;

    // When true, blocks input from reaching child objects
    DEKI_EXPORT
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

    /**
     * @brief Hit test a world-space point against this collider
     *
     * Override in subclasses for custom shapes (circle, polygon, etc.).
     * Default implementation: axis-aligned box with padding.
     *
     * @param x World X coordinate (units)
     * @param y World Y coordinate (units)
     * @return true if the point is inside the collider
     */
    virtual bool HitTest(float x, float y) const;

    /**
     * @brief Process an input event
     *
     * Called by DekiInputSystem. Performs hit test, tracks pointer state,
     * and fires appropriate callbacks. x/y are in world units.
     *
     * @return true if the event was handled (hit test passed and callbacks fired)
     */
    bool ProcessInput(float x, float y, bool down, bool move, bool up);

    /**
     * @brief Cancel any active input state
     *
     * Resets pressed/hover state and fires onPointerUp/onPointerExit
     * so components can clean up. Used by ScrollComponent when confirming
     * a drag gesture to cancel child interactions.
     */
    void CancelInput();

    /**
     * @brief Get the Bounds2D for this collider (for editor visualization)
     *
     * Bounds2D stores world meters; editor gizmo paths multiply by the camera
     * ppm to get screen pixels. For sub-pixel collider math use HitTest().
     */
    Bounds2D GetBounds() const
    {
        Bounds2D b(width, height);
        b.paddingLeft = paddingLeft;
        b.paddingRight = paddingRight;
        b.paddingTop = paddingTop;
        b.paddingBottom = paddingBottom;
        return b;
    }

    // --- State queries ---
    bool IsPointerInside() const { return m_PointerInside; }
    bool IsPressed() const { return m_Pressed; }

private:
    bool m_PointerInside = false;
    bool m_Pressed = false;

    void InvokeCallbacks(const std::vector<PointerCallback>& callbacks, float x, float y);
};

// Generated property metadata (after class definition for offsetof)
#include "generated/InputCollider.gen.h"
