#pragma once

#include <cstdint>

#include <deki/providers/IInputSystem.h>

// Forward declarations
namespace Deki { class Object; }
struct InputEvent;

/**
 * @brief Input dispatch system — routes input events to InputCollider components
 *
 * Concrete implementation of Deki::IInputSystem, lives in the input package DLL.
 * Registered on Deki::Engine via SetInputSystem() during package init.
 *
 * For embedded runtime: registers as a callback on DekiInput and
 * auto-dispatches when input events arrive.
 *
 * For editor play mode: call DispatchInput() directly from PlayViewPanel.
 *
 * Coordinates: DispatchInput takes WORLD UNITS (float). Raw device pixels
 * coming from InputEvent are converted via Camera::ScreenToWorld in
 * OnInputEvent before reaching dispatch.
 */
class DekiInputSystem : public Deki::IInputSystem
{
public:
    DekiInputSystem();
    ~DekiInputSystem() override;

    void Initialize() override;
    void Shutdown() override;
    void DispatchInput(Deki::Scene* scene, float x, float y,
                       bool down, bool move, bool up) override;

    /**
     * @brief Inject a key state change from the host (editor play view).
     *
     * Backed by an "Injected" IDekiInput driver registered on demand, so
     * DekiInput::IsKeyPressed() aggregates injected keys exactly like keys
     * from a real driver (SDL3 on the desktop simulator).
     */
    void DispatchKey(uint32_t key, bool down) override;

    bool IsInitialized() const override { return m_Initialized; }

    void Update() override;
    bool ShouldExit() const override;

private:
    bool m_Initialized = false;

    // Camera used for screen->world, found once per scene rather than by a
    // full tree walk (through a heap-allocated std::function) on every
    // mouse-move event. Reset when the root scene pointer changes.
    class CameraComponent* m_CachedCamera = nullptr;
    Deki::Scene* m_CachedCameraScene = nullptr;
    CameraComponent* FindCamera(Deki::Scene* scene);

    /**
     * @brief Callback from DekiInput — converts screen→world and dispatches
     */
    void OnInputEvent(const InputEvent& event);

    /**
     * @brief Recursively dispatch input to an object and its children
     *
     * Uses children-first dispatch: deepest child processes first,
     * giving inner/frontmost elements priority over parents.
     *
     * @return true if input was consumed (a collider with consumeInput=true handled it)
     */
    bool DispatchToObject(Deki::Object* obj, float x, float y,
                          bool down, bool move, bool up);
};
