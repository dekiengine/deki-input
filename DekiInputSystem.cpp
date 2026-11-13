#include "DekiInputSystem.h"
#include "InputCollider.h"
#include <deki/Engine.h>
#include <deki/Object.h>
#include <deki/Scene.h>
#include <deki/SceneSystem.h>
// Screen to world goes through the scene's camera, which deki-rendering
// provides. Optional: without it, pointer positions are used as they come.
#if __has_include("deki-rendering/CameraComponent.h")
#define DEKI_INPUT_HAS_CAMERA 1
#include "deki-rendering/CameraComponent.h"
#endif
#include "DekiInput.h"  // this package's own
#include <deki/providers/IRenderSystem.h>

#include <map>
#include <memory>

namespace DekiInput
{

namespace
{

// Keyboard state fed by DispatchKey() rather than a physical device driver.
// Registered with DekiInput on first injected key so IsKeyPressed() aggregates
// injected keys exactly like keys from a real driver (SDL3 on the desktop
// simulator). Owned by DekiInput once registered.
class InjectedKeyboard : public IDekiInput
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override { m_Keys.clear(); }
    void Update() override {}
    void RegisterEventCallback(const InputEventCallback& callback) override { m_Callback = callback; }
    bool IsInitialized() const override { return true; }
    bool GetPointerPosition(int32_t*, int32_t*) const override { return false; }

    bool IsKeyPressed(uint32_t key) const override
    {
        auto it = m_Keys.find(key);
        return it != m_Keys.end() && it->second;
    }

    void SetKey(uint32_t key, bool down)
    {
        m_Keys[key] = down;

        if (m_Callback)
        {
            InputEvent event{};
            event.type = down ? InputEventType::KeyDown : InputEventType::KeyUp;
            event.key = key;
            event.pressed = down;
            m_Callback(event);
        }
    }

private:
    std::map<uint32_t, bool> m_Keys;
    InputEventCallback m_Callback;
};

constexpr const char* kInjectedKeyboardName = "Injected";

// A pointer fed by DispatchPointer(): the editor's Play view, standing in for
// a touch panel or a mouse. Its events go through DekiInput like a driver's.
class InjectedPointer : public IDekiInput
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override {}
    void Update() override {}
    void RegisterEventCallback(const InputEventCallback& callback) override { m_Callback = callback; }
    bool IsInitialized() const override { return true; }
    bool GetPointerPosition(int32_t* x, int32_t* y) const override
    {
        if (!m_Known || !x || !y)
        {
            return false;
        }
        *x = m_X;
        *y = m_Y;
        return true;
    }
    bool IsKeyPressed(uint32_t) const override { return false; }

    void Send(int32_t x, int32_t y, bool down, bool move, bool up)
    {
        m_X = x;
        m_Y = y;
        m_Known = true;
        if (!m_Callback)
        {
            return;
        }
        InputEvent event{};
        event.type =
            down ? InputEventType::MouseButtonDown : (up ? InputEventType::MouseButtonUp : InputEventType::MouseMove);
        event.x = x;
        event.y = y;
        event.pressed = down;
        (void)move;
        m_Callback(event);
    }

private:
    InputEventCallback m_Callback;
    int32_t m_X = 0, m_Y = 0;
    bool m_Known = false;
};

constexpr const char* kInjectedPointerName = "Injected pointer";

// Sticks fed by DispatchStick(): a host's gamepad (the editor's Play view).
class InjectedSticks : public IDekiInput
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override {}
    void Update() override {}
    void RegisterEventCallback(const InputEventCallback&) override {}
    bool IsInitialized() const override { return true; }
    bool GetPointerPosition(int32_t*, int32_t*) const override { return false; }
    bool IsKeyPressed(uint32_t) const override { return false; }

    bool GetStick(int stick, float* x, float* y) const override
    {
        if (stick < 0 || stick >= kSticks || !m_Has[stick])
        {
            return false;
        }
        *x = m_X[stick];
        *y = m_Y[stick];
        return true;
    }

    void Set(int stick, float x, float y)
    {
        if (stick >= 0 && stick < kSticks)
        {
            m_X[stick] = x;
            m_Y[stick] = y;
            m_Has[stick] = true;
        }
    }

private:
    static constexpr int kSticks = 2;
    float m_X[kSticks] = {};
    float m_Y[kSticks] = {};
    bool m_Has[kSticks] = {};
};

constexpr const char* kInjectedSticksName = "Injected sticks";

}  // namespace

DekiInputSystem::DekiInputSystem()
{
}

DekiInputSystem::~DekiInputSystem()
{
    Shutdown();
}

void DekiInputSystem::Initialize()
{
    if (m_Initialized)
    {
        return;
    }

    // Register callback on DekiInput to receive input events
    DekiInput::RegisterEventCallback([this](const InputEvent& event) { OnInputEvent(event); });

    m_Initialized = true;
}

void DekiInputSystem::Shutdown()
{
    // Clear callbacks BEFORE DLL unload — the std::function objects in
    // DekiInput::s_GlobalCallbacks hold lambdas whose code lives
    // in this DLL. After FreeLibrary, those function pointers are stale
    // and any operation on them (move, copy, destroy) will crash.
    DekiInput::ClearEventCallbacks();

    // Tear down active input drivers (mouse / keyboard / touch). Deki::Engine
    // reaches this through the interface so it doesn't need to link to the
    // static DekiInput symbol in this package DLL.
    DekiInput::Shutdown();

    m_Initialized = false;
}

void DekiInputSystem::Update()
{
    DekiInput::Update();
}

void DekiInputSystem::DispatchKey(uint32_t key, bool down)
{
    auto* driver = DekiInput::GetInput(kInjectedKeyboardName);
    if (!driver)
    {
        // Registered on demand so the driver only exists for hosts that
        // actually inject keys; DekiInput owns it from here on and clears it
        // on DekiInput::Shutdown (looked up by name each call, never cached).
        DekiInput::SetInput(std::make_unique<InjectedKeyboard>(), kInjectedKeyboardName);
        driver = DekiInput::GetInput(kInjectedKeyboardName);
        if (!driver)
        {
            return;
        }
    }

    static_cast<InjectedKeyboard*>(driver)->SetKey(key, down);
}

bool DekiInputSystem::DispatchPointer(int32_t screenX, int32_t screenY, bool down, bool move, bool up)
{
    auto* driver = DekiInput::GetInput(kInjectedPointerName);
    if (!driver)
    {
        DekiInput::SetInput(std::make_unique<InjectedPointer>(), kInjectedPointerName);
        driver = DekiInput::GetInput(kInjectedPointerName);
        if (!driver)
        {
            return false;
        }
    }
    static_cast<InjectedPointer*>(driver)->Send(screenX, screenY, down, move, up);
    return true;
}

void DekiInputSystem::DispatchStick(int stick, float x, float y)
{
    auto* driver = DekiInput::GetInput(kInjectedSticksName);
    if (!driver)
    {
        DekiInput::SetInput(std::make_unique<InjectedSticks>(), kInjectedSticksName);
        driver = DekiInput::GetInput(kInjectedSticksName);
        if (!driver)
        {
            return;
        }
    }
    static_cast<InjectedSticks*>(driver)->Set(stick, x, y);
}

bool DekiInputSystem::ShouldExit() const
{
    return DekiInput::ShouldExit();
}

void DekiInputSystem::OnInputEvent(const InputEvent& event)
{
    // A scene to send it to is all this needs: the editor runs games without
    // Engine::Initialize.
    Deki::Engine& engine = Deki::Engine::GetInstance();
    Deki::Scene* scene = engine.GetRootScene();
    if (!scene)
    {
        return;
    }

    // A Touch event is taken as the Mouse event it matches.
    bool isDown = (event.type == InputEventType::MouseButtonDown || event.type == InputEventType::TouchDown);
    bool isMove = (event.type == InputEventType::MouseMove || event.type == InputEventType::TouchMove);
    bool isUp = (event.type == InputEventType::MouseButtonUp || event.type == InputEventType::TouchUp);

    if (!isDown && !isMove && !isUp)
    {
        return;
    }

    float worldX = static_cast<float>(event.x);
    float worldY = static_cast<float>(event.y);

#if DEKI_INPUT_HAS_CAMERA
    // Camera for screen-to-world conversion, cached per scene.
    DekiRendering::CameraComponent* cam = FindCamera(scene);
    if (cam && engine.GetRenderSystem())
    {
        cam->ScreenToWorld(static_cast<float>(event.x), static_cast<float>(event.y),
                           engine.GetRenderSystem()->GetScreenWidth(), engine.GetRenderSystem()->GetScreenHeight(),
                           worldX, worldY);
    }
#endif

    DispatchInput(scene, worldX, worldY, isDown, isMove, isUp);
}

#if DEKI_INPUT_HAS_CAMERA
static DekiRendering::CameraComponent* FindCameraRecursive(Deki::Object* obj)
{
    if (DekiRendering::CameraComponent* c = obj->GetComponent<DekiRendering::CameraComponent>())
    {
        return c;
    }
    for (Deki::Object* child : obj->GetChildren())
    {
        if (DekiRendering::CameraComponent* c = FindCameraRecursive(child))
        {
            return c;
        }
    }
    return nullptr;
}

DekiRendering::CameraComponent* DekiInputSystem::FindCamera(Deki::Scene* scene)
{
    const uint32_t serial = Deki::Engine::GetInstance().GetSceneSystem().GetRootSceneSerial();
    if (m_CachedCameraSerial != serial)
    {
        m_CachedCamera = nullptr;
        m_CachedCameraSerial = serial;
    }
    if (!m_CachedCamera && scene)
    {
        for (Deki::Object* obj : scene->GetObjects())
        {
            m_CachedCamera = FindCameraRecursive(obj);
            if (m_CachedCamera)
            {
                break;
            }
        }
    }
    return m_CachedCamera;
}
#endif

void DekiInputSystem::DispatchInput(Deki::Scene* scene, float x, float y, bool down, bool move, bool up)
{
    if (!scene)
    {
        return;
    }

    for (Deki::Object* obj : scene->GetObjects())
    {
        DispatchToObject(obj, x, y, down, move, up);
    }
}

bool DekiInputSystem::DispatchToObject(Deki::Object* obj, float x, float y, bool down, bool move, bool up)
{
    // An inactive object is invisible (the renderer skips it), so it must not
    // take input either, or a hidden menu's buttons fire onClick. Parents are
    // checked on the way down, so IsActive() alone is the full test here.
    if (!obj || !obj->IsActive())
    {
        return false;
    }

    // Phase 1: Recurse to children FIRST (deepest child gets priority)
    // All siblings are dispatched so they can track hover state (pointer_exit).
    bool childConsumed = false;
    for (Deki::Object* child : obj->GetChildren())
    {
        if (DispatchToObject(child, x, y, down, move, up))
        {
            childConsumed = true;
        }
    }

    // Phase 2: Process this object's InputCollider. Left out with the
    // colliders feature, whose sources a stripped build does not compile.
#if !defined(DEKI_PACKAGE_FEATURES_CONFIGURED) || defined(DEKI_FEATURE_INPUT)
    for (Deki::Component* comp : obj->GetComponents())
    {
        if (comp->GetType() == ::Deki::TypeId<InputCollider>() ||
            comp->GetBaseType() == ::Deki::TypeId<InputCollider>())
        {
            auto* collider = static_cast<InputCollider*>(comp);
            // Process if: no child consumed, OR this is a non-consuming collider (e.g. scroll)
            if (!childConsumed || !collider->consumeInput)
            {
                bool handled = collider->ProcessInput(x, y, down, move, up);
                if (handled && collider->consumeInput)
                {
                    return true;
                }
            }
            break;
        }
    }
#endif

    return childConsumed;
}

}  // namespace DekiInput
