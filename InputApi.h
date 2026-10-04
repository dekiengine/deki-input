#pragma once

// DLL export macro, in its own header so the package's headers can use it
// without the DekiInputPackage.h aggregator. Define it here only: two
// definitions that disagree give a symbol a linkage that depends on which
// header was included first.
#ifdef DEKI_EDITOR
#ifdef _WIN32
#if defined(DEKI_INPUT_EXPORTS) || defined(DEKI_ENGINE_EXPORTS) || defined(DEKI_PLUGIN_EXPORTS)
#define DEKI_INPUT_API __declspec(dllexport)
#else
#define DEKI_INPUT_API __declspec(dllimport)
#endif
#else
#define DEKI_INPUT_API
#endif
#else
#define DEKI_INPUT_API
#endif
