#include "I2CKeyboard.h"
#include "Keys.h"

#include <deki/LogSystem.h>
#include <deki/Time.h>

#include <utility>

namespace DekiInput
{

I2CKeyboard::I2CKeyboard(ReadByte read, Probe probe, Clock clock)
    : m_Read(std::move(read)),
      m_Probe(std::move(probe)),
      m_Clock(std::move(clock))
{
    if (!m_Clock)
    {
        m_Clock = []() { return Deki::Time::GetTime(); };
    }
}

bool I2CKeyboard::Initialize()
{
    m_Initialized = m_Read && m_Probe;
    m_State = State::Searching;
    m_Probed = false;
    m_Failures = 0;
    m_DownKey = 0;
    m_MatrixModeSet = false;
    for (auto& column : m_HeldKey)
    {
        for (uint32_t& held : column)
        {
            held = 0;
        }
    }
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
    if (key == 0)
    {
        return false;
    }
    if (m_DownKey == key)
    {
        return true;
    }
    for (const auto& column : m_HeldKey)
    {
        for (uint32_t held : column)
        {
            if (held == key)
            {
                return true;
            }
        }
    }
    return false;
}

void I2CKeyboard::UseKeyMatrix(ReadBytes read, WriteRegister write)
{
    m_ReadMatrix = std::move(read);
    m_WriteMatrixRegister = std::move(write);
}

void I2CKeyboard::Emit(InputEventType type, uint32_t key, uint32_t character)
{
    InputEvent event{};
    event.type = type;
    event.key = key;
    event.character = character;
    event.pressed = (type == InputEventType::KeyDown);
    event.timestamp = m_Clock();
    for (const auto& callback : m_Callbacks)
    {
        callback(event);
    }
}

bool I2CKeyboard::SetBacklight(uint8_t level)
{
    m_Backlight = level;
    return m_WriteRegister && m_WriteRegister(kBacklightRegister, level);
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
                       static_cast<unsigned>(kGiveUpAfterMs));
        m_State = State::GaveUp;
    }
}

void I2CKeyboard::Update()
{
    if (!m_Initialized)
    {
        return;
    }

    // Last update's press is released first, so a key typed twice in a row is
    // two presses rather than one long one.
    if (m_DownKey != 0)
    {
        const uint32_t key = m_DownKey;
        m_DownKey = 0;
        Emit(InputEventType::KeyUp, key, 0);
    }

    if (m_State == State::GaveUp)
    {
        return;
    }
    if (m_State == State::Searching)
    {
        Search(m_Clock());
        return;
    }

    if (m_ReadMatrix)
    {
        UpdateMatrix();
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
    {
        return;
    }

    m_DownKey = Keys::ForCharacter(byte);
    // Control codes (Enter, Backspace, Tab) are keys that type nothing.
    const uint32_t character = (byte >= 32 && byte != 127) ? byte : 0;
    Emit(InputEventType::KeyDown, m_DownKey, character);
}

namespace
{
// The T-Deck keyboard's layout, by column and row of its matrix, as its own
// firmware types it: the plain layer, and the layer under Sym. 0: no key.
constexpr char kPlain[I2CKeyboard::kMatrixColumns][I2CKeyboard::kMatrixRows] = {
    { 'q', 'w', 0, 'a', 0, ' ', 0 },
    { 'e', 's', 'd', 'p', 'x', 'z', 0 },
    { 'r', 'g', 't', 0, 'v', 'c', 'f' },
    { 'u', 'h', 'y', 0, 'b', 'n', 'j' },
    { 'o', 'l', 'i', 0, '$', 'm', 'k' },
};
constexpr char kSymbol[I2CKeyboard::kMatrixColumns][I2CKeyboard::kMatrixRows] = {
    { '#', '1', 0, '*', 0, 0, '0' },
    { '2', '4', '5', '@', '8', '7', 0 },
    { '3', '/', '(', 0, '?', '9', '6' },
    { '_', ':', ')', 0, '!', ',', ';' },
    { '+', '"', '-', 0, 0, '.', '\'' },
};
// Keys that type nothing themselves.
constexpr int kSymColumn = 0, kSymRow = 2;
constexpr int kAltColumn = 0, kAltRow = 4;
constexpr int kShiftColumnA = 1, kShiftRowA = 6, kShiftColumnB = 2, kShiftRowB = 3;
constexpr int kEnterColumn = 3, kEnterRow = 3;
constexpr int kBackspaceColumn = 4, kBackspaceRow = 3;
}  // namespace

void I2CKeyboard::ReleaseAll()
{
    for (int c = 0; c < kMatrixColumns; ++c)
    {
        for (int r = 0; r < kMatrixRows; ++r)
        {
            if (m_HeldKey[c][r] != 0)
            {
                const uint32_t key = m_HeldKey[c][r];
                m_HeldKey[c][r] = 0;
                Emit(InputEventType::KeyUp, key, 0);
            }
        }
    }
}

void I2CKeyboard::UpdateMatrix()
{
    if (!m_MatrixModeSet)
    {
        // Register 0x03 and a byte the firmware reads as no command.
        if (!m_WriteMatrixRegister || !m_WriteMatrixRegister(kMatrixModeRegister, 0))
        {
            return;
        }
        m_MatrixModeSet = true;
    }

    uint8_t columns[kMatrixColumns] = {};
    if (!m_ReadMatrix(columns, kMatrixColumns))
    {
        if (++m_Failures >= kLostAfterFailures)
        {
            DEKI_LOG_WARNING("I2CKeyboard: the keyboard stopped answering; looking for it again");
            ReleaseAll();
            m_State = State::Searching;
            m_Probed = false;
            m_MatrixModeSet = false;  // a keyboard that restarted is back in its character mode
        }
        return;
    }
    m_Failures = 0;

    auto down = [&](int c, int r) { return (columns[c] >> r) & 1u; };
    const bool sym = down(kSymColumn, kSymRow) != 0;
    const bool shift = down(kShiftColumnA, kShiftRowA) != 0 || down(kShiftColumnB, kShiftRowB) != 0;

    for (int c = 0; c < kMatrixColumns; ++c)
    {
        for (int r = 0; r < kMatrixRows; ++r)
        {
            const bool isDown = down(c, r) != 0;
            if (!isDown)
            {
                if (m_HeldKey[c][r] != 0)
                {
                    const uint32_t key = m_HeldKey[c][r];
                    m_HeldKey[c][r] = 0;
                    Emit(InputEventType::KeyUp, key, 0);
                }
                continue;
            }
            if (m_HeldKey[c][r] != 0)
            {
                continue;  // still held
            }
            // A key goes down: what it types now, with Sym and Shift as held now.
            uint32_t key = 0;
            uint32_t character = 0;
            if (c == kEnterColumn && r == kEnterRow)
            {
                key = Keys::Enter;
            }
            else if (c == kBackspaceColumn && r == kBackspaceRow)
            {
                key = Keys::Backspace;
            }
            else if ((c == kSymColumn && r == kSymRow) || (c == kAltColumn && r == kAltRow) ||
                     (c == kShiftColumnA && r == kShiftRowA) || (c == kShiftColumnB && r == kShiftRowB))
            {
                continue;  // modifiers
            }
            else
            {
                char typed = sym ? kSymbol[c][r] : kPlain[c][r];
                if (typed == 0)
                {
                    continue;
                }
                if (shift && typed >= 'a' && typed <= 'z')
                {
                    typed = static_cast<char>(typed - 'a' + 'A');
                }
                character = static_cast<uint8_t>(typed);
                key = Keys::ForCharacter(character);
            }
            m_HeldKey[c][r] = key;
            Emit(InputEventType::KeyDown, key, character);
        }
    }
}

}  // namespace DekiInput
