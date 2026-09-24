#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "IDekiInput.h"
#include "InputApi.h"

namespace DekiInput
{

/**
 * @brief A trackball: four pulse lines and a click
 *
 * The LilyGO T-Deck's ball pulses one line per step in each direction and
 * closes a switch when pressed. What a game makes of that is a choice, so
 * the driver has two:
 *
 *   Keys     steps are the arrow keys and the click is Enter. A frame with
 *            steps in a direction is one press of that arrow, released the
 *            next frame, so rolling repeats. A menu or a grid game.
 *   Pointer  the ball moves a pointer across the screen, `pixelsPerStep` per
 *            step, and the click is a mouse button. Anything that already
 *            works with touch.
 *
 * Reading the lines is handed in, so the behaviour is testable without pins.
 * TrackballComponent supplies the GPIO one.
 */
class DEKI_INPUT_API Trackball : public IDekiInput
{
public:
    enum class Mode { Keys, Pointer };

    /// Steps since the last read, per direction, and whether the ball is
    /// pressed right now.
    struct Sample
    {
        uint32_t up = 0, down = 0, left = 0, right = 0;
        bool pressed = false;
    };
    using Read = std::function<Sample()>;
    using Clock = std::function<uint32_t()>;

    /// `clock` defaults to the engine's.
    explicit Trackball(Read read, Clock clock = nullptr);

    void SetMode(Mode mode) { m_Mode = mode; }
    /// Pointer mode: the area the pointer moves in, and how far one step is.
    void SetPointerArea(int32_t width, int32_t height);
    /// Pointer mode: where to read the area from, asked on every update. The
    /// screen's size is only known once a display is set, which is after this
    /// is set up; until it reports a size the pointer does not move, and the
    /// first size it reports centres it.
    using AreaSource = std::function<void(int32_t& width, int32_t& height)>;
    void SetPointerAreaSource(AreaSource source) { m_AreaSource = std::move(source); }
    void SetPixelsPerStep(int32_t pixels) { m_PixelsPerStep = pixels > 0 ? pixels : 1; }

    bool Initialize() override;
    void Shutdown() override;
    void Update() override;
    void RegisterEventCallback(const InputEventCallback& callback) override;
    bool IsInitialized() const override { return m_Initialized; }
    bool GetPointerPosition(int32_t* x, int32_t* y) const override;
    bool IsKeyPressed(uint32_t key) const override;

private:
    void Emit(InputEventType type, uint32_t key, int32_t x, int32_t y, bool pressed);
    void UpdateKeys(const Sample& s);
    void UpdatePointer(const Sample& s);

    Read m_Read;
    Clock m_Clock;
    AreaSource m_AreaSource;
    bool m_AreaKnown = false;  // with a source: whether it has reported a size yet
    std::vector<InputEventCallback> m_Callbacks;

    bool m_Initialized = false;
    Mode m_Mode = Mode::Keys;
    int32_t m_PixelsPerStep = 4;
    int32_t m_Width = 320, m_Height = 240;
    int32_t m_X = 160, m_Y = 120;

    bool m_Pressed = false;
    // Keys mode: the arrows put down last update, to be released this one.
    std::vector<uint32_t> m_DownKeys;
};

}  // namespace DekiInput
