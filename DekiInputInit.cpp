#include "DekiInputInit.h"
#include "DekiInputSystem.h"
#include <deki/Engine.h>
#include <deki/LogSystem.h>

// The package's own state stays in its namespace; the two entry points below
// are at global scope because the editor's generated glue declares them that
// way. See DekiInputInit.h.
namespace DekiInput
{
static DekiInputSystem* s_InputSystem = nullptr;
}

using namespace DekiInput;

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
