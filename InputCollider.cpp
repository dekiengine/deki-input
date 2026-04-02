#include "InputCollider.h"
#include "InputDispatch.h"
#include "DekiObject.h"
#include <cmath>

InputCollider::InputCollider()
    : width(0),
      height(0),
      padding_left(0),
      padding_right(0),
      padding_top(0),
      padding_bottom(0),
      consume_input(true)
{
}

bool InputCollider::HitTest(int32_t x, int32_t y) const
{
    DekiObject* owner = GetOwner();
    if (!owner)
        return false;

    float ownerX = owner->GetWorldX();
    float ownerY = owner->GetWorldY();

    // Convert world coordinates (Y-up, center origin) to bounds coordinates
    // (Y-down, top-left origin). Object position is the visual center (pivot 0.5).
    Bounds2D bounds = GetBounds();
    return bounds.Contains(
        static_cast<int32_t>(x - ownerX) + width / 2,
        static_cast<int32_t>(ownerY - y) + height / 2
    );
}

bool InputCollider::ProcessInput(int32_t x, int32_t y, bool down, bool move, bool up)
{
    // If another component has claimed the gesture (e.g. scroll is dragging),
    // suppress all input on consuming colliders — no hover, no press, nothing.
    if (consume_input && InputDispatch::IsGestureClaimed())
        return false;

    bool inside = HitTest(x, y);
    bool handled = false;

    // Pointer down
    if (down && inside)
    {
        m_Pressed = true;
        InvokeCallbacks(on_pointer_down, x, y);
        handled = true;
    }

    // Pointer up
    if (up && m_Pressed)
    {
        InvokeCallbacks(on_pointer_up, x, y);
        m_Pressed = false;
        handled = true;
    }

    // Pointer enter/exit (hover tracking)
    if (move || down)
    {
        if (inside && !m_PointerInside)
        {
            m_PointerInside = true;
            InvokeCallbacks(on_pointer_enter, x, y);
            handled = true;
        }
        else if (!inside && m_PointerInside)
        {
            m_PointerInside = false;
            InvokeCallbacks(on_pointer_exit, x, y);
            handled = true;
        }
    }

    // Pointer move (while inside or pressed)
    if (move && (inside || m_Pressed))
    {
        InvokeCallbacks(on_pointer_move, x, y);
        handled = true;
    }

    return handled;
}

void InputCollider::CancelInput()
{
    bool wasPressed = m_Pressed;
    bool wasInside = m_PointerInside;

    // Clear all state BEFORE callbacks so handlers see the cancelled state.
    // Without this, ButtonComponent's on_pointer_up handler sees IsPointerInside()
    // still true and fires a spurious on_click.
    m_Pressed = false;
    m_PointerInside = false;

    if (wasPressed)
        InvokeCallbacks(on_pointer_up, 0, 0);
    if (wasInside)
        InvokeCallbacks(on_pointer_exit, 0, 0);
}

void InputCollider::InvokeCallbacks(const std::vector<PointerCallback>& callbacks, int32_t x, int32_t y)
{
    for (const auto& cb : callbacks)
    {
        if (cb) cb(x, y);
    }
}
