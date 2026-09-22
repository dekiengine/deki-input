#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "IDekiInput.h"
#include "InputApi.h"

namespace DekiInput
{

/**
 * @brief A keyboard that answers a read with the next character typed
 *
 * The LilyGO T-Deck's keyboard and the M5Stack CardKB are both this: a small
 * controller on the I2C bus that does the matrix scanning, the shift and
 * symbol layers and the debouncing itself, and hands over one finished
 * character per read, or 0 when nothing was typed.
 *
 * Such a device reports presses and never releases, so each character becomes
 * a KEY_DOWN and, on the next update, the matching KEY_UP. IsKeyPressed is
 * true in between: long enough for a game that polls once a frame to see it.
 *
 * How a byte is read is handed in rather than known here, so the behaviour
 * can be tested without a bus. I2CKeyboardComponent supplies the I2C one.
 *
 */
class DEKI_INPUT_API I2CKeyboard : public IDekiInput
{
public:
    /// Read one byte. False when the device did not answer.
    using ReadByte = std::function<bool(uint8_t& out)>;
    /// Whether the device is on the bus at all.
    using Probe = std::function<bool()>;
    /// Milliseconds, monotonic.
    using Clock = std::function<uint32_t()>;

    /// `clock` defaults to the engine's.
    I2CKeyboard(ReadByte read, Probe probe, Clock clock = nullptr);

    bool Initialize() override;
    void Shutdown() override;
    void Update() override;
    void RegisterEventCallback(const InputEventCallback& callback) override;
    bool IsInitialized() const override { return m_Initialized; }
    bool GetPointerPosition(int32_t* x, int32_t* y) const override;
    bool IsKeyPressed(uint32_t key) const override;

    /// Searching until the device answers; GaveUp when it never did.
    enum class State { Searching, Active, GaveUp };
    State GetState() const { return m_State; }

    // The keyboard's own controller takes a moment to start after the board
    // powers it, so it is looked for over a while rather than once, and not
    // every frame: a read nobody answers is a bus timeout.
    static constexpr uint32_t kProbeIntervalMs = 250;
    static constexpr uint32_t kGiveUpAfterMs = 5000;
    // This many unanswered reads in a row and it is looked for again.
    static constexpr int kLostAfterFailures = 30;

private:
    void Emit(InputEventType type, uint32_t key, uint32_t character);
    void Search(uint32_t now);

    ReadByte m_Read;
    Probe m_Probe;
    Clock m_Clock;
    std::vector<InputEventCallback> m_Callbacks;

    bool m_Initialized = false;
    State m_State = State::Searching;
    uint32_t m_SearchStarted = 0;
    uint32_t m_LastProbe = 0;
    bool m_Probed = false;
    int m_Failures = 0;
    uint32_t m_DownKey = 0;  // awaiting its KEY_UP; 0 = none
};

}  // namespace DekiInput
