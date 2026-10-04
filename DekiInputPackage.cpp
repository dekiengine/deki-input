// Package entry point for the deki-input DLL. Exports the standard Deki
// plugin interface, so the editor can load deki-input.dll and register its
// components (InputCollider).

#include "DekiInputPackage.h"
#include <deki/interop/Plugin.h>
#include "InputCollider.h"
#include "DekiInputInit.h"
#include <deki/Engine.h>
#include <deki/reflection/ComponentRegistry.h>
#include <deki/reflection/ComponentFactory.h>

extern void DekiInputRegisterComponents();
extern int DekiInputGetAutoComponentCount();
extern const Deki::ComponentMeta* DekiInputGetAutoComponentMeta(int index);

namespace DekiInput
{

#ifdef DEKI_EDITOR

#ifndef DEKI_PLUGIN_EXPORTS
// Auto-generated registration helpers (standalone DLL only)

// Track if already registered to avoid duplicates
static bool s_InputRegistered = false;

// The exports below are C symbols at global scope; the package's own
// registration helpers and statics live in its namespace.
using namespace DekiInput;

extern "C"
{
    // Makes sure the deki-input package is loaded and its components are
    // registered.
    DEKI_INPUT_API int DekiInputEnsureRegistered(void)
    {
        if (s_InputRegistered)
        {
            return ::DekiInputGetAutoComponentCount();
        }
        s_InputRegistered = true;

        ::DekiInputRegisterComponents();

        // Safe to repeat: DekiInitPackageSystems() may already have started
        // it during Deki::Engine::Initialize().
        DekiInputInitSystem();

        return ::DekiInputGetAutoComponentCount();
    }

}  // extern "C"
#endif  // DEKI_PLUGIN_EXPORTS

// =============================================================================
// Plugin metadata (for dynamic loading compatibility)
// =============================================================================

extern "C"
{
#ifndef DEKI_PLUGIN_EXPORTS
    DEKI_PLUGIN_API const char* DekiPluginGetName(void)
    {
        return "Deki Input Package";
    }

    DEKI_PLUGIN_API const char* DekiPluginGetVersion(void)
    {
#ifdef DEKI_PACKAGE_VERSION
        return DEKI_PACKAGE_VERSION;
#else
        return "0.0.0-dev";
#endif
    }

    DEKI_PLUGIN_API int DekiPluginInit(void)
    {
        return 0;
    }

    DEKI_PLUGIN_API void DekiPluginShutdown(void)
    {
        DekiInputShutdownSystem();
        s_InputRegistered = false;
    }

    DEKI_PLUGIN_API int DekiPluginGetComponentCount(void)
    {
        return ::DekiInputGetAutoComponentCount();
    }

    DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPluginGetComponentMeta(int index)
    {
        return ::DekiInputGetAutoComponentMeta(index);
    }

    DEKI_PLUGIN_API void DekiPluginRegisterComponents(void)
    {
        DekiInputEnsureRegistered();
    }

#endif  // DEKI_PLUGIN_EXPORTS

    // =============================================================================
    // Package-specific feature API
    // =============================================================================

    DEKI_INPUT_API const char* DekiInputGetName(void)
    {
        return "Input";
    }

}  // extern "C"

#endif  // DEKI_EDITOR
}  // namespace DekiInput
