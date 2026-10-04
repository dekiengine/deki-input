#pragma once

#include <cstdint>

#include <deki/providers/IInputSystem.h>

// Forward declarations
namespace Deki
{
class Object;
}

namespace DekiRendering
{
class CameraComponent;
}

namespace DekiInput
{
struct InputEvent;

/// Sends input events to InputCollider components. The package's
/// Deki::IInputSystem, set on Deki::Engine with SetInputSystem() when the
/// package starts.
///
/// On a device it registers a callback on DekiInput and dispatches as events
/// arrive. In editor play mode, PlayViewPanel calls DispatchInput() directly.
///
/// DispatchInput takes WORLD UNITS (float). OnInputEvent converts the device
/// pixels in an InputEvent with Camera::ScreenToWorld before dispatching.
class DekiInputSystem : public Deki::IInputSystem
{
public:
    DekiInputSystem();
    ~DekiInputSystem() override;

    void Initialize() override;
    void Shutdown() override;
    void DispatchInput(Deki::Scene* scene, float x, float y, bool down, bool move, bool up) override;

    /// Injects a key state change from the host (the editor play view).
    /// Backed by an "Injected" IDekiInput driver registered on demand, so
    /// DekiInput::IsKeyPressed() counts injected keys exactly like keys from
    /// a real driver (SDL3 on the desktop simulator).
    void DispatchKey(uint32_t key, bool down) override;

    bool IsInitialized() const override { return m_Initialized; }

    void Update() override;
    bool ShouldExit() const override;

private:
    bool m_Initialized = false;

    // Camera used for screen->world, found once per scene so a mouse move
    // does not walk the whole tree. Reset when the root scene pointer changes.
    DekiRendering::CameraComponent* m_CachedCamera = nullptr;
    Deki::Scene* m_CachedCameraScene = nullptr;
    DekiRendering::CameraComponent* FindCamera(Deki::Scene* scene);

    // Callback from DekiInput: converts screen to world and dispatches.
    void OnInputEvent(const InputEvent& event);

    // Dispatches input to an object and its children, children first, so the
    // innermost (frontmost) elements take priority over their parents.
    // Returns true when a collider with consumeInput=true handled it.
    bool DispatchToObject(Deki::Object* obj, float x, float y, bool down, bool move, bool up);
};

}  // namespace DekiInput
