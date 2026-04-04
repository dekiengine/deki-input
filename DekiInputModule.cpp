/**
 * @file DekiInputModule.cpp
 * @brief Module entry point for deki-input DLL
 *
 * Exports the standard Deki plugin interface so the editor
 * can load deki-input.dll and register its components (InputCollider).
 */

#include "DekiInputModule.h"
#include "interop/DekiPlugin.h"
#include "DekiModuleFeatureMeta.h"
#include "InputCollider.h"
#include "DekiInputSystem.h"
#include "DekiEngine.h"
#include "reflection/ComponentRegistry.h"
#include "reflection/ComponentFactory.h"

#ifdef DEKI_EDITOR

// Auto-generated registration helpers
extern void DekiInput_RegisterComponents();
extern int DekiInput_GetAutoComponentCount();
extern const DekiComponentMeta* DekiInput_GetAutoComponentMeta(int index);

// Track if already registered to avoid duplicates
static bool s_InputRegistered = false;
static DekiInputSystem s_InputSystem;

extern "C" {

/**
 * @brief Ensure deki-input module is loaded and components are registered
 */
DEKI_INPUT_API int DekiInput_EnsureRegistered(void)
{
    if (s_InputRegistered)
        return DekiInput_GetAutoComponentCount();
    s_InputRegistered = true;

    DekiInput_RegisterComponents();

    // Register input system on DekiEngine (accessible via GetInputSystem())
    DekiEngine::GetInstance().SetInputSystem(&s_InputSystem);

    return DekiInput_GetAutoComponentCount();
}

} // extern "C"

// =============================================================================
// Plugin metadata (for dynamic loading compatibility)
// =============================================================================

extern "C" {

#ifndef DEKI_PLUGIN_EXPORTS
DEKI_PLUGIN_API const char* DekiPlugin_GetName(void)
{
    return "Deki Input Module";
}

DEKI_PLUGIN_API const char* DekiPlugin_GetVersion(void)
{
#ifdef DEKI_MODULE_VERSION
    return DEKI_MODULE_VERSION;
#else
    return "0.0.0-dev";
#endif
}

DEKI_PLUGIN_API const char* DekiPlugin_GetReflectionJson(void)
{
    return "{}";
}

DEKI_PLUGIN_API int DekiPlugin_Init(void)
{
    return 0;
}

DEKI_PLUGIN_API void DekiPlugin_Shutdown(void)
{
    s_InputSystem.Shutdown();
    DekiEngine::GetInstance().SetInputSystem(nullptr);
    s_InputRegistered = false;
}

DEKI_PLUGIN_API int DekiPlugin_GetComponentCount(void)
{
    return DekiInput_GetAutoComponentCount();
}

DEKI_PLUGIN_API const DekiComponentMeta* DekiPlugin_GetComponentMeta(int index)
{
    return DekiInput_GetAutoComponentMeta(index);
}

DEKI_PLUGIN_API void DekiPlugin_RegisterComponents(void)
{
    DekiInput_EnsureRegistered();
}

#endif // DEKI_PLUGIN_EXPORTS (resume after feature data)

// =============================================================================
// Module Feature API
// =============================================================================

static const char* s_InputGuids[] = { InputCollider::StaticGuid };

static const DekiModuleFeatureInfo s_Features[] = {
    {"input", "Input", "Input dispatch with InputCollider hit areas", true, "DEKI_FEATURE_INPUT", s_InputGuids, 1},
};

#ifndef DEKI_PLUGIN_EXPORTS
DEKI_PLUGIN_API int DekiPlugin_GetFeatureCount(void)
{
    return sizeof(s_Features) / sizeof(s_Features[0]);
}

DEKI_PLUGIN_API const DekiModuleFeatureInfo* DekiPlugin_GetFeature(int index)
{
    if (index < 0 || index >= DekiPlugin_GetFeatureCount())
        return nullptr;
    return &s_Features[index];
}

#endif // DEKI_PLUGIN_EXPORTS

// =============================================================================
// Module-specific feature API
// =============================================================================

DEKI_INPUT_API const char* DekiInput_GetName(void)
{
    return "Input";
}

DEKI_INPUT_API int DekiInput_GetFeatureCount(void)
{
    return static_cast<int>(sizeof(s_Features) / sizeof(s_Features[0]));
}

DEKI_INPUT_API const DekiModuleFeatureInfo* DekiInput_GetFeature(int index)
{
    if (index < 0 || index >= DekiInput_GetFeatureCount())
        return nullptr;
    return &s_Features[index];
}

} // extern "C"

#endif // DEKI_EDITOR
