#pragma once

#include <cstdint>
#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "InputApi.h"

namespace DekiInput
{

/**
 * @brief Boot-scene component for a keyboard on the I2C bus
 *
 * For a keyboard that answers a read with the next character typed: the
 * LilyGO T-Deck's (address 0x55) and the M5Stack CardKB (0x5F). Needs an
 * I2CBusComponent on the matching port earlier in the boot scene.
 *
 * Keys arrive as KEY_DOWN / KEY_UP through DekiInput like any other
 * keyboard's: InputEvent::key is the key (Keys.h), InputEvent::character what
 * it typed.
 *
 * A keyboard that is not there does not stop the boot. Its controller may
 * still be starting when this runs, so it is looked for over the first few
 * seconds, and after that the step says nothing answered and stays quiet.
 */
DEKI_CATEGORY("Input")
DEKI_DISPLAY_NAME("I2C Keyboard")
DEKI_DESCRIPTION("Reads a keyboard that hands over one typed character per I2C read: the LilyGO T-Deck's, the M5Stack CardKB.")
class DEKI_INPUT_API I2CKeyboardComponent : public Deki::SetupComponent
{
public:

    DEKI_EXPORT
    DEKI_TOOLTIP("Which I2C bus the keyboard is on. Must match the I2C Bus component that set that port up.")
    DEKI_RANGE(0, 3)
    int32_t i2cPort = 0;

    DEKI_EXPORT
    DEKI_TOOLTIP("The keyboard's address on the bus: 0x55 (85) for the LilyGO T-Deck, 0x5F (95) for the M5Stack CardKB.")
    DEKI_RANGE(0, 127)
    int32_t i2cAddress = 0x55;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "I2C Keyboard"; }
};

// Generated property metadata

}  // namespace DekiInput
