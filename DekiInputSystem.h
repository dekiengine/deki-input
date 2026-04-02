#pragma once

#include "providers/IDekiInputSystem.h"

// Forward declarations
class DekiObject;
struct InputEvent;

/**
 * @brief Input dispatch system — routes input events to InputCollider components
 *
 * Concrete implementation of IDekiInputSystem, lives in the input module DLL.
 * Registered on DekiEngine via SetInputSystem() during module init.
 *
 * For embedded runtime: registers as a callback on DekiInputProvider and
 * auto-dispatches when input events arrive.
 *
 * For editor play mode: call DispatchInput() directly from PlayViewPanel.
 */
class DekiInputSystem : public IDekiInputSystem
{
public:
    DekiInputSystem();
    ~DekiInputSystem() override;

    void Initialize() override;
    void Shutdown() override;
    void DispatchInput(Prefab* prefab, int32_t x, int32_t y,
                       bool down, bool move, bool up) override;
    bool IsInitialized() const override { return m_Initialized; }

private:
    bool m_Initialized = false;

    /**
     * @brief Callback from DekiInputProvider — converts screen→world and dispatches
     */
    void OnInputEvent(const InputEvent& event);

    /**
     * @brief Recursively dispatch input to an object and its children
     *
     * Uses children-first dispatch: deepest child processes first,
     * giving inner/frontmost elements priority over parents.
     *
     * @return true if input was consumed (a collider with consume_input=true handled it)
     */
    bool DispatchToObject(DekiObject* obj, int32_t x, int32_t y,
                          bool down, bool move, bool up);
};
