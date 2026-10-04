#pragma once

/// Starts the input system: creates DekiInputSystem and registers it with the
/// engine. Called from DekiInitPackageSystems() on firmware builds and from
/// DekiInputEnsureRegistered() on editor builds. Safe to call more than once.
///
/// At global scope on purpose, the one part of this package that is. These
/// are link-time glue: the editor generates a translation unit that declares
/// them as plain `extern void DekiInputInitSystem();` and calls them to bring
/// up a static simulator or firmware build. That file cannot know a package's
/// namespace, so each entry point carries the package prefix in its name.
/// DekiInputRegisterComponents, emitted by the reflection codegen, is global
/// for the same reason. Moving these into namespace DekiInput breaks every
/// simulator and firmware link.
void DekiInputInitSystem();
void DekiInputShutdownSystem();
