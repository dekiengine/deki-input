#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "IDekiInput.h"

namespace DekiInput
{

/// The registry of input sources (mouse, keyboard, touch, gamepad and so on).
/// Several can be active at once; events from every active source go into
/// one global event stream.
///
/// Drivers live in platform integration packages and register with
/// SetInput(); this package owns the interface and the dispatch logic.
class DekiInput
{
private:
    static std::map<std::string, std::unique_ptr<IDekiInput>> s_ActiveInputs;
    static std::vector<InputEventCallback> s_GlobalCallbacks;
    static bool s_Initialized;
    static bool s_ShouldExit;

public:
    /// Adds an initialized input source under `name` and takes ownership of
    /// it. A name already registered is left as it is. Returns false only for
    /// a null input.
    static bool SetInput(std::unique_ptr<IDekiInput> input, const std::string& name);

    /// Shuts down and removes every input source and callback.
    static void Shutdown();

    /// The input source registered under `name`, or nullptr.
    static IDekiInput* GetInput(const std::string& name);

    /// Adds a callback that receives events from ALL input sources.
    static void RegisterEventCallback(const InputEventCallback& callback);

    /// Removes every global event callback. Call it before freeing a DLL that
    /// registered callbacks, or the std::function objects point into freed
    /// code.
    static void ClearEventCallbacks();

    /// Updates every active input source, once a frame, which delivers their
    /// events.
    static void Update();

    /// The pointer position from the first input source that has one. Returns
    /// false when none does.
    static bool GetPointerPosition(int32_t* x, int32_t* y);

    /// True when `key` (see Keys.h) is down on any active input source.
    static bool IsKeyPressed(uint32_t key);

    /// True once any input source has sent a quit event.
    static bool ShouldExit();

    /// Sets the exit flag. Input packages call it on a quit event.
    static void SetShouldExit(bool exit);

    static bool IsInitialized() { return s_Initialized; }

    /// The names of all active input sources.
    static std::vector<std::string> GetActiveInputs();

private:
    // Passes an event from any source to the global callbacks.
    static void DistributeEvent(const InputEvent& event);
};

}  // namespace DekiInput
