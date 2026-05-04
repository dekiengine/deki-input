#include "DekiInputSystem.h"
#include "InputCollider.h"
#include "DekiEngine.h"
#include "DekiObject.h"
#include "Prefab.h"
#include "deki-rendering/CameraComponent.h"
#include "providers/DekiInputProvider.h"
#include "providers/IDekiRenderSystem.h"

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
        return;

    // Register callback on DekiInputProvider to receive input events
    DekiInputProvider::RegisterEventCallback([this](const InputEvent& event) {
        OnInputEvent(event);
    });

    m_Initialized = true;
}

void DekiInputSystem::Shutdown()
{
    // Clear callbacks BEFORE DLL unload — the std::function objects in
    // DekiInputProvider::global_callbacks hold lambdas whose code lives
    // in this DLL. After FreeLibrary, those function pointers are stale
    // and any operation on them (move, copy, destroy) will crash.
    DekiInputProvider::ClearEventCallbacks();
    m_Initialized = false;
}

void DekiInputSystem::OnInputEvent(const InputEvent& event)
{
    DekiEngine& engine = DekiEngine::GetInstance();
    if (!engine.IsInitialized())
        return;

    Prefab* prefab = engine.GetRootPrefab();
    if (!prefab)
        return;

    bool isDown = (event.type == InputEventType::MOUSE_BUTTON_DOWN);
    bool isMove = (event.type == InputEventType::MOUSE_MOVE);
    bool isUp   = (event.type == InputEventType::MOUSE_BUTTON_UP);

    if (!isDown && !isMove && !isUp)
        return;

    // Find camera for screen-to-world coordinate conversion (search recursively)
    CameraComponent* cam = nullptr;
    std::function<CameraComponent*(DekiObject*)> findCamera = [&](DekiObject* obj) -> CameraComponent* {
        CameraComponent* c = obj->GetComponent<CameraComponent>();
        if (c) return c;
        for (auto* child : obj->GetChildren())
        {
            c = findCamera(child);
            if (c) return c;
        }
        return nullptr;
    };
    for (DekiObject* obj : prefab->GetObjects())
    {
        cam = findCamera(obj);
        if (cam) break;
    }

    float worldX = static_cast<float>(event.x);
    float worldY = static_cast<float>(event.y);

    if (cam && engine.GetRenderSystem())
    {
        cam->ScreenToWorld(static_cast<float>(event.x), static_cast<float>(event.y),
                           engine.GetRenderSystem()->GetScreenWidth(),
                           engine.GetRenderSystem()->GetScreenHeight(),
                           worldX, worldY);
    }

    DispatchInput(prefab, worldX, worldY, isDown, isMove, isUp);
}

void DekiInputSystem::DispatchInput(Prefab* prefab, float x, float y,
                                     bool down, bool move, bool up)
{
    if (!prefab)
        return;

    for (DekiObject* obj : prefab->GetObjects())
    {
        DispatchToObject(obj, x, y, down, move, up);
    }
}

bool DekiInputSystem::DispatchToObject(DekiObject* obj, float x, float y,
                                        bool down, bool move, bool up)
{
    if (!obj)
        return false;

    // Phase 1: Recurse to children FIRST (deepest child gets priority)
    // All siblings are dispatched so they can track hover state (pointer_exit).
    bool childConsumed = false;
    for (DekiObject* child : obj->GetChildren())
    {
        if (DispatchToObject(child, x, y, down, move, up))
            childConsumed = true;
    }

    // Phase 2: Process this object's InputCollider
    for (DekiComponent* comp : obj->GetComponents())
    {
        if (comp->getType() == InputCollider::StaticType ||
            comp->getBaseType() == InputCollider::StaticType)
        {
            auto* collider = static_cast<InputCollider*>(comp);
            // Process if: no child consumed, OR this is a non-consuming collider (e.g. scroll)
            if (!childConsumed || !collider->consume_input)
            {
                bool handled = collider->ProcessInput(x, y, down, move, up);
                if (handled && collider->consume_input)
                    return true;
            }
            break;
        }
    }

    return childConsumed;
}
