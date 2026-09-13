#pragma once

/**
 * @file DekiInputPackage.h
 * @brief Central header for the Deki Input Package
 *
 * The input package provides:
 * - InputCollider: Hit area component with pointer event callbacks
 * - DekiInputSystem: Dispatches input events to InputCollider components
 * - InputDispatch: Gesture claim system (in deki-engine-core)
 */

// DLL export macro (own header so intra-package headers avoid this aggregator)
#include "InputApi.h"

// Include all package headers when package is enabled
#ifdef DEKI_PACKAGE_INPUT

#include "InputCollider.h"
#include "DekiInputSystem.h"

#endif // DEKI_PACKAGE_INPUT
