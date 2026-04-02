#pragma once

/**
 * @file DekiInputModule.h
 * @brief Central header for the Deki Input Module
 *
 * The input module provides:
 * - InputCollider: Hit area component with pointer event callbacks
 * - DekiInputSystem: Dispatches input events to InputCollider components
 * - InputDispatch: Gesture claim system (in deki-engine-core)
 */

// DLL export macro
#ifdef DEKI_EDITOR
    #ifdef _WIN32
        #ifdef DEKI_INPUT_EXPORTS
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

// Include all module headers when module is enabled
#ifdef DEKI_MODULE_INPUT

#include "InputCollider.h"
#include "DekiInputSystem.h"

#endif // DEKI_MODULE_INPUT
