#include "I2CKeyboard.h"
#include "Keys.h"

#include <deki/LogSystem.h>
#include <deki/Time.h>

#include <utility>

namespace DekiInput
{


I2CKeyboard::I2CKeyboard(ReadByte read, Probe probe, Clock clock)
    : m_Read(std::move(read)), m_Probe(std::move(probe)), m_Clock(std::move(clock))
{
    if (!m_Clock)
        m_Clock = []() { return Deki::Time::GetTime(); };
}

bool I2CKeyboard::Initialize()
{
    m_Initialized = m_Read && m_Probe;
    m_State = State::Searching;
    m_Probed = false;
    m_Failures = 0;
    m_DownKey = 0;
    return m_Initialized;
}

void I2CKeyboard::Shutdown()
{
    m_Initialized = false;
    m_Callbacks.clear();
    m_DownKey = 0;
}

void I2CKeyboard::RegisterEventCallback(const InputEventCallback& callback)
{
    m_Callbacks.push_back(callback);
}

bool I2CKeyboard::GetPointerPosition(int32_t*, int32_t*) const
{
    return false;
}

bool I2CKeyboard::IsKeyPressed(uint32_t key) const
{
    return m_DownKey != 0 && m_DownKey == key;
}

void I2CKeyboard::Emit(InputEventType type, uint32_t key, uint32_t character)
{
    InputEvent event{};
    event.type = type;
    event.key = key;
    event.character = character;
    event.pressed = (type == InputEventType::KEY_DOWN);
    event.timestamp = m_Clock();
    for (const auto& callback : m_Callbacks)
        callback(event);
}

void I2CKeyboard::Search(uint32_t now)
{
    if (!m_Probed)
    {
        m_SearchStarted = now;
    }
    else if (now - m_LastProbe < kProbeIntervalMs)
    {
        return;
    }

    m_Probed = true;
    m_LastProbe = now;

    if (m_Probe())
    {
        DEKI_LOG_INFO("I2CKeyboard: keyboard found");
        m_State = State::Active;
        m_Failures = 0;
    }
    else if (now - m_SearchStarted >= kGiveUpAfterMs)
    {
        DEKI_LOG_ERROR("I2CKeyboard: nothing answered at the keyboard's address in %u ms; not reading it",
                       (unsigned)kGiveUpAfterMs);
        m_State = State::GaveUp;
    }
}

void I2CKeyboard::Update()
{
    if (!m_Initialized)
        return;

    // Last update's press is released first, so a key typed twice in a row is
    // two presses rather than one long one.
    if (m_DownKey != 0)
    {
        const uint32_t key = m_DownKey;
        m_DownKey = 0;
        Emit(InputEventType::KEY_UP, key, 0);
    }

    if (m_State == State::GaveUp)
        return;
    if (m_State == State::Searching)
    {
        Search(m_Clock());
        return;
    }

    uint8_t byte = 0;
    if (!m_Read(byte))
    {
        if (++m_Failures >= kLostAfterFailures)
        {
            DEKI_LOG_WARNING("I2CKeyboard: the keyboard stopped answering; looking for it again");
            m_State = State::Searching;
            m_Probed = false;
        }
        return;
    }
    m_Failures = 0;

    if (byte == 0)
        return;

    m_DownKey = Keys::ForCharacter(byte);
    // Control codes (Enter, Backspace, Tab) are keys that type nothing.
    const uint32_t character = (byte >= 32 && byte != 127) ? byte : 0;
    Emit(InputEventType::KEY_DOWN, m_DownKey, character);
}

}  // namespace DekiInput
