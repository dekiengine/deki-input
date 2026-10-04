#pragma once

#include <stdint.h>

#include <functional>

namespace DekiInput
{

enum class InputEventType
{
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    KeyDown,
    KeyUp,
    TouchDown,
    TouchUp,
    TouchMove,
    AppQuit  // Application quit request
};

struct InputEvent
{
    InputEventType type;
    int32_t x, y;        // Position for mouse/touch events
    uint32_t key;        // Key id for keyboard events: see Keys.h
    bool pressed;        // Button/key state
    uint32_t timestamp;  // Event timestamp

    // What a KeyDown types, as a code point: 'A' for shift and the A key,
    // where `key` is Keys::A. 0 when the key types nothing (an arrow) or the
    // driver only knows keys, not text. Last and defaulted so drivers written
    // before it need no change.
    uint32_t character = 0;
};

// Input event callback function type
using InputEventCallback = std::function<void(const InputEvent& event)>;

/// What an input driver implements to work with the Deki engine: start-up,
/// events, and the state of the pointer and keys.
class IDekiInput
{
public:
    virtual ~IDekiInput() = default;

    /// Starts the driver. Returns false when it cannot.
    virtual bool Initialize() = 0;

    /// Stops the driver and frees its resources.
    virtual void Shutdown() = 0;

    /// Reads the device and sends its events; called once a frame.
    virtual void Update() = 0;

    /// Adds a callback that receives this driver's events.
    virtual void RegisterEventCallback(const InputEventCallback& callback) = 0;

    virtual bool IsInitialized() const = 0;

    /// The current mouse or touch position. Returns false when there is none.
    virtual bool GetPointerPosition(int32_t* x, int32_t* y) const = 0;

    /// True when `key` (see Keys.h) is down.
    virtual bool IsKeyPressed(uint32_t key) const = 0;
};

}  // namespace DekiInput