#include "Trackball.h"
#include "Keys.h"

#include <deki/Time.h>

#include <algorithm>
#include <utility>

namespace DekiInput
{


Trackball::Trackball(Read read, Clock clock) : m_Read(std::move(read)), m_Clock(std::move(clock))
{
    if (!m_Clock)
        m_Clock = []() { return Deki::Time::GetTime(); };
}

void Trackball::SetPointerArea(int32_t width, int32_t height)
{
    m_Width = width > 0 ? width : 1;
    m_Height = height > 0 ? height : 1;
    m_X = std::clamp<int32_t>(m_X, 0, m_Width - 1);
    m_Y = std::clamp<int32_t>(m_Y, 0, m_Height - 1);
}

bool Trackball::Initialize()
{
    m_Initialized = static_cast<bool>(m_Read);
    m_Pressed = false;
    m_DownKeys.clear();
    m_X = m_Width / 2;
    m_Y = m_Height / 2;
    return m_Initialized;
}

void Trackball::Shutdown()
{
    m_Initialized = false;
    m_Callbacks.clear();
    m_DownKeys.clear();
}

void Trackball::RegisterEventCallback(const InputEventCallback& callback)
{
    m_Callbacks.push_back(callback);
}

bool Trackball::GetPointerPosition(int32_t* x, int32_t* y) const
{
    if (m_Mode != Mode::Pointer)
        return false;
    if (x) *x = m_X;
    if (y) *y = m_Y;
    return true;
}

bool Trackball::IsKeyPressed(uint32_t key) const
{
    if (m_Mode == Mode::Keys && key == Keys::Enter)
        return m_Pressed;
    return std::find(m_DownKeys.begin(), m_DownKeys.end(), key) != m_DownKeys.end();
}

void Trackball::Emit(InputEventType type, uint32_t key, int32_t x, int32_t y, bool pressed)
{
    InputEvent event{};
    event.type = type;
    event.key = key;
    event.x = x;
    event.y = y;
    event.pressed = pressed;
    event.timestamp = m_Clock();
    for (const auto& callback : m_Callbacks)
        callback(event);
}

void Trackball::Update()
{
    if (!m_Initialized)
        return;

    // Last update's arrows come up first, so a roll is a press per frame.
    for (uint32_t key : m_DownKeys)
        Emit(InputEventType::KEY_UP, key, 0, 0, false);
    m_DownKeys.clear();

    const Sample s = m_Read();
    if (m_Mode == Mode::Keys)
        UpdateKeys(s);
    else
        UpdatePointer(s);
}

void Trackball::UpdateKeys(const Sample& s)
{
    // Opposite steps in one frame cancel: the ball went nowhere.
    const int32_t dx = static_cast<int32_t>(s.right) - static_cast<int32_t>(s.left);
    const int32_t dy = static_cast<int32_t>(s.down) - static_cast<int32_t>(s.up);

    if (dx != 0)
        m_DownKeys.push_back(dx > 0 ? Keys::Right : Keys::Left);
    if (dy != 0)
        m_DownKeys.push_back(dy > 0 ? Keys::Down : Keys::Up);
    for (uint32_t key : m_DownKeys)
        Emit(InputEventType::KEY_DOWN, key, 0, 0, true);

    if (s.pressed != m_Pressed)
    {
        m_Pressed = s.pressed;
        Emit(m_Pressed ? InputEventType::KEY_DOWN : InputEventType::KEY_UP, Keys::Enter, 0, 0, m_Pressed);
    }
}

void Trackball::UpdatePointer(const Sample& s)
{
    if (m_AreaSource)
    {
        int32_t w = 0, h = 0;
        m_AreaSource(w, h);
        if (w <= 0 || h <= 0)
            return;  // no screen yet: nowhere to move
        if (!m_AreaKnown || w != m_Width || h != m_Height)
        {
            SetPointerArea(w, h);
            if (!m_AreaKnown)
            {
                m_X = m_Width / 2;
                m_Y = m_Height / 2;
                m_AreaKnown = true;
            }
        }
    }

    const int32_t dx = (static_cast<int32_t>(s.right) - static_cast<int32_t>(s.left)) * m_PixelsPerStep;
    const int32_t dy = (static_cast<int32_t>(s.down) - static_cast<int32_t>(s.up)) * m_PixelsPerStep;
    if (dx != 0 || dy != 0)
    {
        const int32_t x = std::clamp<int32_t>(m_X + dx, 0, m_Width - 1);
        const int32_t y = std::clamp<int32_t>(m_Y + dy, 0, m_Height - 1);
        if (x != m_X || y != m_Y)
        {
            m_X = x;
            m_Y = y;
            Emit(InputEventType::MOUSE_MOVE, 0, m_X, m_Y, m_Pressed);
        }
    }

    if (s.pressed != m_Pressed)
    {
        m_Pressed = s.pressed;
        Emit(m_Pressed ? InputEventType::MOUSE_BUTTON_DOWN : InputEventType::MOUSE_BUTTON_UP, 0, m_X, m_Y,
             m_Pressed);
    }
}

}  // namespace DekiInput
