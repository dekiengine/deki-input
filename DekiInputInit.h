#pragma once

/**
 * @brief Initialize the input system (creates DekiInputSystem, registers with engine).
 *
 * Called from deki_init_package_systems() on firmware builds,
 * and from DekiInput_EnsureRegistered() on editor builds.
 *
 * Idempotent — safe to call multiple times.
 *
 * GLOBAL SCOPE, deliberately, and the one part of this package that is.
 * These are link-time glue: the editor generates a translation unit that
 * declares them as plain `extern void DekiInput_InitSystem();` and calls them
 * to bring a static simulator or firmware build up. That generated file cannot
 * know a package's namespace, which is why every entry point it names carries
 * a package prefix in the symbol itself. DekiInput_RegisterComponents, emitted
 * by the reflection codegen, is global for the same reason.
 *
 * 0.16.0 moved the whole package into namespace DekiInput and swept these up
 * with it, which silently broke every simulator and firmware link: the
 * generated glue looked for the global symbols and the package no longer had
 * them. Keep them here, outside the namespace.
 */
void DekiInput_InitSystem();
void DekiInput_ShutdownSystem();
