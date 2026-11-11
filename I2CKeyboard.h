#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "IDekiInput.h"
#include "InputApi.h"

namespace DekiInput
{

/// A keyboard that answers a read with the next character typed.
///
/// The LilyGO T-Deck's keyboard and the M5Stack CardKB are both this: a small
/// controller on the I2C bus that does the matrix scanning, the shift and
/// symbol layers and the debouncing itself, and hands over one finished
/// character per read, or 0 when nothing was typed.
///
/// Such a device reports presses but no releases, so each character becomes
/// a KeyDown and, on the next update, the matching KeyUp. IsKeyPressed is
/// true in between: long enough for a game that polls once a frame to see it.
///
/// The T-Deck's keyboard can also hand over its whole key matrix instead
/// (UseKeyMatrix). Keys are then down for as long as they are held, as on a
/// desktop keyboard, which a game needs to walk while a key is held.
///
/// How a byte is read is passed in, so the behaviour can be tested without a
/// bus. I2CKeyboardComponent supplies the I2C one.
class DEKI_INPUT_API I2CKeyboard : public IDekiInput
{
public:
    /// Read one byte. False when the device did not answer.
    using ReadByte = std::function<bool(uint8_t& out)>;
    /// Whether the device is on the bus at all.
    using Probe = std::function<bool()>;
    /// Milliseconds, monotonic.
    using Clock = std::function<uint32_t()>;
    /// Writes `value` to the device's register `reg`. False when it did not
    /// answer.
    using WriteRegister = std::function<bool(uint8_t reg, uint8_t value)>;

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
    enum class State
    {
        Searching,
        Active,
        GaveUp
    };
    State GetState() const { return m_State; }

    /// For a keyboard with a backlight it controls itself over the bus: the
    /// LilyGO T-Deck's takes its brightness, 0 (off) to 255, in register 0x01.
    /// Without a writer there is no backlight to set.
    void SetBacklightWriter(WriteRegister writer) { m_WriteRegister = std::move(writer); }
    bool HasBacklight() const { return static_cast<bool>(m_WriteRegister); }

    /// Sets the backlight brightness, 0 to 255. False when there is none or
    /// the keyboard did not answer; the level asked for is kept either way.
    bool SetBacklight(uint8_t level);

    /// Reads `count` bytes. False when the device did not answer.
    using ReadBytes = std::function<bool(uint8_t* out, size_t count)>;

    /// The LilyGO T-Deck's keyboard only: once found, it is switched to
    /// handing over its key matrix (register 0x03) and read whole each
    /// update, so a key is down from when it is pressed until it is let go.
    /// Needs keyboard firmware with that mode; with an older one the bytes are
    /// characters, not the matrix. Set before Initialize.
    void UseKeyMatrix(ReadBytes read, WriteRegister write);
    bool UsesKeyMatrix() const { return static_cast<bool>(m_ReadMatrix); }

    static constexpr uint8_t kMatrixModeRegister = 0x03;
    static constexpr int kMatrixColumns = 5;
    static constexpr int kMatrixRows = 7;
    uint8_t GetBacklight() const { return m_Backlight; }

    static constexpr uint8_t kBacklightRegister = 0x01;

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
    void UpdateMatrix();
    void ReleaseAll();

    ReadBytes m_ReadMatrix;
    WriteRegister m_WriteMatrixRegister;
    bool m_MatrixModeSet = false;
    // Keys mode of the matrix: what each held position pressed, 0 = up.
    uint32_t m_HeldKey[kMatrixColumns][kMatrixRows] = {};

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
    uint32_t m_DownKey = 0;  // awaiting its KeyUp; 0 = none

    WriteRegister m_WriteRegister;
    uint8_t m_Backlight = 127;  // the T-Deck keyboard's own start-up level
};

}  // namespace DekiInput
