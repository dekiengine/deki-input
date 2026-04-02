# Module descriptor for deki-engine auto-discovery
set(MODULE_DISPLAY_NAME "Input")
set(MODULE_PREFIX "DekiInput")
set(MODULE_UPPER "INPUT")
set(MODULE_TARGET "deki-input")
set(MODULE_FILE_PREFIX "Input")
set(MODULE_CORE_SOURCES
    InputDispatch.cpp
)
set(MODULE_SOURCES
    InputCollider.cpp
    DekiInputSystem.cpp
)
set(MODULE_ENTRY DekiInputModule.cpp)
