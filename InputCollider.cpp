#include "InputCollider.h"
#include "InputDispatch.h"
#include <deki/Object.h>
#include <cmath>

InputCollider::InputCollider()
    : width(0.0f),
      height(0.0f),
      paddingLeft(0.0f),
      paddingRight(0.0f),
      paddingTop(0.0f),
      paddingBottom(0.0f),
      consumeInput(true)
{
}

bool InputCollider::HitTest(float x, float y) const
{
    Deki::Object* owner = GetOwner();
    if (!owner)
        return false;

    const float ownerX = owner->GetWorldX();
    const float ownerY = owner->GetWorldY();

    // Convert world coordinates (Y-up, center origin) to bounds-local coords
    // (Y-down, top-left origin). Object position is the visual center
    // (pivot 0.5), so the box spans (ownerX - w/2, ownerY + h/2) to
    // (ownerX + w/2, ownerY - h/2) in world (Y-up) terms.
    const float localX = (x - ownerX) + width  * 0.5f;
    const float localY = (ownerY - y) + height * 0.5f;

    return localX >= -paddingLeft  &&
           localX <= width  + paddingRight &&
           localY >= -paddingTop   &&
           localY <= height + paddingBottom;
}

bool InputCollider::ProcessInput(float x, float y, bool down, bool move, bool up)
{
    // If another component has claimed the gesture (e.g. scroll is dragging),
    // suppress all input on consuming colliders — no hover, no press, nothing.
    if (consumeInput && InputDispatch::IsGestureClaimed())
        return false;

    bool inside = HitTest(x, y);
    bool handled = false;

    // Pointer down
    if (down && inside)
    {
        m_Pressed = true;
        InvokeCallbacks(onPointerDown, x, y);
        handled = true;
    }

    // Pointer up
    if (up && m_Pressed)
    {
        InvokeCallbacks(onPointerUp, x, y);
        m_Pressed = false;
        handled = true;
    }

    // Pointer up — clear hover state (touch devices have no hover after release)
    if (up && m_PointerInside)
    {
        m_PointerInside = false;
        InvokeCallbacks(onPointerExit, x, y);
        handled = true;
    }

    // Pointer enter/exit (hover tracking)
    if (move || down)
    {
        if (inside && !m_PointerInside)
        {
            m_PointerInside = true;
            InvokeCallbacks(onPointerEnter, x, y);
            handled = true;
        }
        else if (!inside && m_PointerInside)
        {
            m_PointerInside = false;
            InvokeCallbacks(onPointerExit, x, y);
            handled = true;
        }
    }

    // Pointer move (while inside or pressed)
    if (move && (inside || m_Pressed))
    {
        InvokeCallbacks(onPointerMove, x, y);
        handled = true;
    }

    return handled;
}

void InputCollider::CancelInput()
{
    bool wasPressed = m_Pressed;
    bool wasInside = m_PointerInside;

    // Clear all state BEFORE callbacks so handlers see the cancelled state.
    // Without this, ButtonComponent's onPointerUp handler sees IsPointerInside()
    // still true and fires a spurious on_click.
    m_Pressed = false;
    m_PointerInside = false;

    if (wasPressed)
        InvokeCallbacks(onPointerUp, 0.0f, 0.0f);
    if (wasInside)
        InvokeCallbacks(onPointerExit, 0.0f, 0.0f);
}

void InputCollider::InvokeCallbacks(const std::vector<PointerCallback>& callbacks, float x, float y)
{
    for (const auto& cb : callbacks)
    {
        if (cb) cb(x, y);
    }
}
