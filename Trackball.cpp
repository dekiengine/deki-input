#include "Trackball.h"
#include "Keys.h"

#include <deki/Time.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

namespace DekiInput
{

Trackball::Trackball(Read read, Clock clock)
    : m_Read(std::move(read)),
      m_Clock(std::move(clock))
{
    if (!m_Clock)
    {
        m_Clock = []() { return Deki::Time::GetTime(); };
    }
}

void Trackball::SetPointerArea(int32_t width, int32_t height)
{
    m_Width = width > 0 ? width : 1;
    m_Height = height > 0 ? height : 1;
    m_X = std::clamp<int32_t>(m_X, 0, m_Width - 1);
    m_Y = std::clamp<int32_t>(m_Y, 0, m_Height - 1);
    m_FX = static_cast<float>(m_X);
    m_FY = static_cast<float>(m_Y);
}

bool Trackball::Initialize()
{
    m_Initialized = static_cast<bool>(m_Read);
    m_Pressed = false;
    m_DownKeys.clear();
    m_X = m_Width / 2;
    m_Y = m_Height / 2;
    m_FX = static_cast<float>(m_X);
    m_FY = static_cast<float>(m_Y);
    m_VX = m_VY = m_StepRate = 0.0f;
    m_HaveLastUpdate = false;
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
    {
        return false;
    }
    if (x)
    {
        *x = m_X;
    }
    if (y)
    {
        *y = m_Y;
    }
    return true;
}

bool Trackball::IsKeyPressed(uint32_t key) const
{
    if (m_Mode == Mode::Keys && key == Keys::Enter)
    {
        return m_Pressed;
    }
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
    {
        callback(event);
    }
}

void Trackball::Update()
{
    if (!m_Initialized)
    {
        return;
    }

    // Last update's arrows come up first, so a roll is a press per frame.
    for (uint32_t key : m_DownKeys)
    {
        Emit(InputEventType::KeyUp, key, 0, 0, false);
    }
    m_DownKeys.clear();

    const Sample s = m_Read();
    if (m_Mode == Mode::Keys)
    {
        UpdateKeys(s);
    }
    else
    {
        UpdatePointer(s);
    }
}

void Trackball::UpdateKeys(const Sample& s)
{
    // Opposite steps in one frame cancel: the ball went nowhere.
    const int32_t dx = static_cast<int32_t>(s.right) - static_cast<int32_t>(s.left);
    const int32_t dy = static_cast<int32_t>(s.down) - static_cast<int32_t>(s.up);

    if (dx != 0)
    {
        m_DownKeys.push_back(dx > 0 ? Keys::Right : Keys::Left);
    }
    if (dy != 0)
    {
        m_DownKeys.push_back(dy > 0 ? Keys::Down : Keys::Up);
    }
    for (uint32_t key : m_DownKeys)
    {
        Emit(InputEventType::KeyDown, key, 0, 0, true);
    }

    if (s.pressed != m_Pressed)
    {
        m_Pressed = s.pressed;
        Emit(m_Pressed ? InputEventType::KeyDown : InputEventType::KeyUp, Keys::Enter, 0, 0, m_Pressed);
    }
}

void Trackball::UpdatePointer(const Sample& s)
{
    if (m_AreaSource)
    {
        int32_t w = 0, h = 0;
        m_AreaSource(w, h);
        if (w <= 0 || h <= 0)
        {
            return;  // no screen yet: nowhere to move
        }
        if (!m_AreaKnown || w != m_Width || h != m_Height)
        {
            SetPointerArea(w, h);
            if (!m_AreaKnown)
            {
                m_X = m_Width / 2;
                m_Y = m_Height / 2;
                m_FX = static_cast<float>(m_X);
                m_FY = static_cast<float>(m_Y);
                m_AreaKnown = true;
            }
        }
    }

    const int32_t stepsX = static_cast<int32_t>(s.right) - static_cast<int32_t>(s.left);
    const int32_t stepsY = static_cast<int32_t>(s.down) - static_cast<int32_t>(s.up);
    int32_t dx = stepsX * m_PixelsPerStep;
    int32_t dy = stepsY * m_PixelsPerStep;
    if (m_GlideMs > 0.0f)
    {
        Glide(stepsX, stepsY, dx, dy);
    }
    if (dx != 0 || dy != 0)
    {
        const int32_t x = std::clamp<int32_t>(m_X + dx, 0, m_Width - 1);
        const int32_t y = std::clamp<int32_t>(m_Y + dy, 0, m_Height - 1);
        if (x != m_X || y != m_Y)
        {
            m_X = x;
            m_Y = y;
            Emit(InputEventType::MouseMove, 0, m_X, m_Y, m_Pressed);
        }
    }

    if (s.pressed != m_Pressed)
    {
        m_Pressed = s.pressed;
        Emit(m_Pressed ? InputEventType::MouseButtonDown : InputEventType::MouseButtonUp, 0, m_X, m_Y, m_Pressed);
    }
}

void Trackball::Glide(int32_t stepsX, int32_t stepsY, int32_t& dx, int32_t& dy)
{
    // Time since the last update, held to a sane range: the first update and
    // a long stall (a scene loading) count as one frame.
    const uint32_t now = m_Clock();
    float dtMs = m_HaveLastUpdate ? static_cast<float>(now - m_LastUpdateMs) : 16.0f;
    m_LastUpdateMs = now;
    m_HaveLastUpdate = true;
    dtMs = std::clamp(dtMs, 1.0f, 100.0f);
    const float dt = dtMs / 1000.0f;
    const float tau = m_GlideMs / 1000.0f;

    // How fast the ball turns, smoothed, sets the acceleration: from 1x
    // below 10 steps a second to the full factor at 60.
    const float steps = static_cast<float>(std::abs(stepsX) + std::abs(stepsY));
    m_StepRate += (steps / dt - m_StepRate) * std::min(1.0f, dtMs / 50.0f);
    const float boost = 1.0f + (m_Acceleration - 1.0f) * std::clamp((m_StepRate - 10.0f) / 50.0f, 0.0f, 1.0f);

    // Each step adds speed that, decaying with the glide, adds up to
    // pixelsPerStep * boost of travel.
    const float kick = static_cast<float>(m_PixelsPerStep) * boost / tau;
    m_VX += static_cast<float>(stepsX) * kick;
    m_VY += static_cast<float>(stepsY) * kick;

    m_FX = std::clamp(m_FX + m_VX * dt, 0.0f, static_cast<float>(m_Width - 1));
    m_FY = std::clamp(m_FY + m_VY * dt, 0.0f, static_cast<float>(m_Height - 1));
    const float decay = std::exp(-dt / tau);
    m_VX *= decay;
    m_VY *= decay;
    // Stopped against an edge, or slow enough to count as still.
    if (m_FX <= 0.0f || m_FX >= static_cast<float>(m_Width - 1) || std::fabs(m_VX) < 2.0f)
    {
        m_VX = 0.0f;
    }
    if (m_FY <= 0.0f || m_FY >= static_cast<float>(m_Height - 1) || std::fabs(m_VY) < 2.0f)
    {
        m_VY = 0.0f;
    }

    dx = static_cast<int32_t>(std::lround(m_FX)) - m_X;
    dy = static_cast<int32_t>(std::lround(m_FY)) - m_Y;
}

}  // namespace DekiInput
