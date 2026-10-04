#pragma once

// The deki-input package's main header. It provides:
// - InputCollider: a hit area component with pointer event callbacks
// - DekiInputSystem: sends input events to InputCollider components
// - InputDispatch: lets one component claim the current gesture

// DLL export macro (own header so the package's headers avoid this aggregator)
#include "InputApi.h"

// Include all package headers when package is enabled
#ifdef DEKI_PACKAGE_INPUT

#include "InputCollider.h"
#include "DekiInputSystem.h"

#endif  // DEKI_PACKAGE_INPUT
