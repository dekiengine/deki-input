#pragma once

#include <cstdint>
#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "InputApi.h"

namespace DekiInput
{

enum class TrackballMode : uint8_t
{
    Keys = 0,     // arrows and Enter
    Pointer = 1   // a mouse
};

/**
 * @brief Boot-scene component for a trackball on GPIO pins
 *
 * Four lines that pulse once per step, one per direction, and a switch for
 * the click. The LilyGO T-Deck's ball is one. Steps are counted by interrupt
 * through deki-gpio, so none are lost between frames.
 *
 * In Keys mode the ball is the arrow keys and the click is Enter; in Pointer
 * mode it moves a pointer across the screen and the click is a mouse button,
 * so a scene built for touch works unchanged.
 *
 * A platform with no GPIO backend (the desktop) logs and boots on.
 */
DEKI_CATEGORY("Input")
DEKI_DISPLAY_NAME("Trackball")
DEKI_DESCRIPTION("A trackball on GPIO pins, as arrow keys and Enter or as a pointer.")
class DEKI_INPUT_API TrackballComponent : public Deki::SetupComponent
{
public:

    DEKI_EXPORT
    DEKI_TOOLTIP("Keys: rolling presses the arrow keys and the click is Enter. Pointer: rolling moves a pointer and the click is a mouse button.")
    TrackballMode mode = TrackballMode::Keys;

    DEKI_EXPORT
    DEKI_TOOLTIP("Pointer mode: how many pixels one step of the ball moves the pointer")
    DEKI_RANGE(1, 64)
    int32_t pixelsPerStep = 4;

    DEKI_GROUP("Pins")
    DEKI_EXPORT
    DEKI_TOOLTIP("The line that pulses when the ball rolls up")
    DEKI_RANGE(-1, 48)
    int32_t upPin = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The line that pulses when the ball rolls down")
    DEKI_RANGE(-1, 48)
    int32_t downPin = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The line that pulses when the ball rolls left")
    DEKI_RANGE(-1, 48)
    int32_t leftPin = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The line that pulses when the ball rolls right")
    DEKI_RANGE(-1, 48)
    int32_t rightPin = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The click switch (-1 = none)")
    DEKI_RANGE(-1, 48)
    int32_t clickPin = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The click pulls its pin low when pressed (on), or high (off)")
    bool clickActiveLow = true;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "Trackball"; }
};

// Generated property metadata

}  // namespace DekiInput
