#pragma once

#include <cstdint>
#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "InputApi.h"

namespace DekiInput
{

/// Boot-scene component for a keyboard on the I2C bus.
///
/// For a keyboard that answers a read with the next character typed: the
/// LilyGO T-Deck's (address 0x55) and the M5Stack CardKB (0x5F). Needs an
/// I2CBusComponent on the matching port earlier in the boot scene.
///
/// Keys arrive as KeyDown / KeyUp through DekiInput like any other
/// keyboard's: InputEvent::key is the key (Keys.h), InputEvent::character what
/// it typed.
///
/// A missing keyboard does not stop the boot. Its controller may still be
/// starting when this runs, so it is looked for over the first few seconds;
/// after that the step reports that nothing answered and stays quiet.
DEKI_CATEGORY("Input")
DEKI_DISPLAY_NAME("I2C Keyboard")
DEKI_DESCRIPTION(
    "Reads a keyboard that hands over one typed character per I2C read: the LilyGO T-Deck's, the M5Stack CardKB.")
class DEKI_INPUT_API I2CKeyboardComponent : public Deki::SetupComponent
{
public:
    DEKI_EXPORT
    DEKI_TOOLTIP("Which I2C bus the keyboard is on. Must match the I2C Bus component that set that port up.")
    DEKI_RANGE(0, 3)
    int32_t i2cPort = 0;

    DEKI_EXPORT
    DEKI_TOOLTIP(
        "The keyboard's address on the bus: 0x55 (85) for the LilyGO T-Deck, 0x5F (95) for the M5Stack CardKB.")
    DEKI_RANGE(0, 127)
    int32_t i2cAddress = 0x55;

    DEKI_EXPORT
    DEKI_TOOLTIP("The keyboard has a backlight it controls itself, set over the bus (the LilyGO T-Deck's: brightness "
                 "in register 0x01). Game code then sets it with I2CKeyboard::SetBacklight.")
    bool hasBacklight = false;

    DEKI_EXPORT
    DEKI_TOOLTIP("The LilyGO T-Deck's keyboard: read its whole key matrix, so a key is down for as long as it is held, "
                 "as a desktop keyboard's is. A game can then walk while a key is held. Needs keyboard firmware with "
                 "the matrix mode (register 0x03); off, each typed key is down for one frame.")
    bool reportHeldKeys = false;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "I2C Keyboard"; }
};

}  // namespace DekiInput
