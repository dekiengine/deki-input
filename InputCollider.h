#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

#include "DekiComponent.h"
#include "reflection/DekiProperty.h"
#include "modules/2d/Bounds2D.h"

class DekiObject;

/**
 * @brief Hit area component for pointer/touch input
 *
 * Like Unity's Collider2D — defines a clickable/hoverable area and fires
 * callbacks when pointer events occur. Other components (ButtonComponent,
 * ScrollComponent, etc.) register callbacks to react to input.
 *
 * Subclass and override HitTest() for custom shapes (circle, polygon, etc.).
 *
 * Usage:
 * @code
 * // Add InputCollider to make an object interactive
 * auto* collider = obj->AddComponent<InputCollider>();
 * collider->width = 100;
 * collider->height = 40;
 *
 * // Register callbacks
 * collider->on_pointer_down.push_back([](int32_t x, int32_t y) {
 *     // Handle press
 * });
 * @endcode
 */
class InputCollider : public DekiComponent
{
    DEKI_COMPONENT(InputCollider, DekiComponent, "Input", "a1b2c3d4-e5f6-7890-abcd-ef1234567890", "DEKI_FEATURE_INPUT")

public:
    // Hit area dimensions
    DEKI_EXPORT
    int32_t width = 0;

    DEKI_EXPORT
    int32_t height = 0;

    // Hit area padding (expands the hit area beyond width/height)
    DEKI_EXPORT
    int32_t padding_left = 0;

    DEKI_EXPORT
    int32_t padding_right = 0;

    DEKI_EXPORT
    int32_t padding_top = 0;

    DEKI_EXPORT
    int32_t padding_bottom = 0;

    // When true, blocks input from reaching child objects
    DEKI_EXPORT
    bool consume_input = true;

    // --- Pointer event callbacks ---
    // Components register these in Start() to react to input

    using PointerCallback = std::function<void(int32_t x, int32_t y)>;

    std::vector<PointerCallback> on_pointer_down;
    std::vector<PointerCallback> on_pointer_up;
    std::vector<PointerCallback> on_pointer_enter;
    std::vector<PointerCallback> on_pointer_exit;
    std::vector<PointerCallback> on_pointer_move;

    // --- Public API ---

    InputCollider();

    /**
     * @brief Hit test a world-space point against this collider
     *
     * Override in subclasses for custom shapes (circle, polygon, etc.).
     * Default implementation: axis-aligned box with padding.
     *
     * @param x World X coordinate
     * @param y World Y coordinate
     * @return true if the point is inside the collider
     */
    virtual bool HitTest(int32_t x, int32_t y) const;

    /**
     * @brief Process an input event
     *
     * Called by DekiInputSystem. Performs hit test, tracks pointer state,
     * and fires appropriate callbacks.
     *
     * @return true if the event was handled (hit test passed and callbacks fired)
     */
    bool ProcessInput(int32_t x, int32_t y, bool down, bool move, bool up);

    /**
     * @brief Cancel any active input state
     *
     * Resets pressed/hover state and fires on_pointer_up/on_pointer_exit
     * so components can clean up. Used by ScrollComponent when confirming
     * a drag gesture to cancel child interactions.
     */
    void CancelInput();

    /**
     * @brief Get the Bounds2D for this collider (for editor visualization)
     */
    Bounds2D GetBounds() const
    {
        Bounds2D b(width, height);
        b.padding_left = padding_left;
        b.padding_right = padding_right;
        b.padding_top = padding_top;
        b.padding_bottom = padding_bottom;
        return b;
    }

    // --- State queries ---
    bool IsPointerInside() const { return m_PointerInside; }
    bool IsPressed() const { return m_Pressed; }

private:
    bool m_PointerInside = false;
    bool m_Pressed = false;

    void InvokeCallbacks(const std::vector<PointerCallback>& callbacks, int32_t x, int32_t y);
};

// Generated property metadata (after class definition for offsetof)
#include "generated/InputCollider.gen.h"
