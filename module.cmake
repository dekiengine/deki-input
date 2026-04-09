# Module descriptor for deki-engine auto-discovery
set(MODULE_DISPLAY_NAME "Input")
set(MODULE_PREFIX "DekiInput")
set(MODULE_UPPER "INPUT")
set(MODULE_TARGET "deki-input")
set(MODULE_FILE_PREFIX "Input")
set(MODULE_HAS_SYSTEM_INIT ON)
set(MODULE_SOURCES
    InputDispatch.cpp
    InputCollider.cpp
    DekiInputSystem.cpp
    DekiInputInit.cpp
)
set(MODULE_ENTRY DekiInputModule.cpp)
set(MODULE_LINK_DEPS deki-rendering)
