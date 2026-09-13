#pragma once

// DLL export macro, in its own header so intra-package headers can use it
// without pulling the DekiInputPackage.h aggregator, and so the macro has one
// definition: InputDispatch.h and the aggregator each used to spell it out,
// and the two spellings disagreed about DEKI_ENGINE_EXPORTS, so the linkage a
// symbol got depended on which header was included first.
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
