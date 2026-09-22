# Package descriptor for deki-engine auto-discovery
set(PACKAGE_DISPLAY_NAME "Input")
set(PACKAGE_PREFIX "DekiInput")
set(PACKAGE_UPPER "INPUT")
set(PACKAGE_TARGET "deki-input")
set(PACKAGE_FILE_PREFIX "Input")
set(PACKAGE_HAS_SYSTEM_INIT ON)
set(PACKAGE_SOURCES
    InputDispatch.cpp
    InputCollider.cpp
    DekiInputSystem.cpp
    DekiInputInit.cpp
    DekiInput.cpp
    I2CKeyboard.cpp
    I2CKeyboardComponent.cpp
    Trackball.cpp
    TrackballComponent.cpp
)
set(PACKAGE_ENTRY DekiInputPackage.cpp)
# Each links only when installed: see the features' "requires".
set(PACKAGE_LINK_DEPS deki-rendering deki-i2c deki-gpio)
