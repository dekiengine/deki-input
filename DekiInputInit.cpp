#include "DekiInputInit.h"
#include "DekiInputSystem.h"
#include <deki/Engine.h>
#include <deki/LogSystem.h>

static DekiInputSystem* s_InputSystem = nullptr;

void DekiInput_InitSystem()
{
    if (s_InputSystem)
        return;

    s_InputSystem = new DekiInputSystem();
    s_InputSystem->Initialize();
    Deki::Engine::GetInstance().SetInputSystem(s_InputSystem);

    DEKI_LOG_INTERNAL("DekiInput: Input system initialized");
}

void DekiInput_ShutdownSystem()
{
    if (!s_InputSystem)
        return;

    Deki::Engine::GetInstance().SetInputSystem(nullptr);
    s_InputSystem->Shutdown();
    delete s_InputSystem;
    s_InputSystem = nullptr;

    DEKI_LOG_INTERNAL("DekiInput: Input system shutdown");
}
