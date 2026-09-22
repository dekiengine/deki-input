#include "I2CKeyboardComponent.h"
#include "I2CKeyboard.h"
#include "DekiInput.h"

// The keyboard needs deki-i2c, which installing deki-input does not bring: a
// project without it (a desktop one, typically) builds this step as a stub.
#if __has_include("DekiI2C.h")
#define DEKI_INPUT_HAS_I2C 1
#include "DekiI2C.h"   // from deki-i2c
#include "IDekiI2C.h"  // from deki-i2c
#endif

#include <deki/LogSystem.h>

#include <memory>

namespace DekiInput
{

#if DEKI_INPUT_HAS_I2C

void I2CKeyboardComponent::Setup(SetupCallback onComplete)
{
    // A keyboard is not what decides whether the game starts: every way this
    // can go wrong is reported and the boot carries on.
    DekiI2c::IDekiI2C* bus = DekiI2c::DekiI2C::GetBus(i2cPort);
    if (!bus)
    {
        DEKI_LOG_ERROR("I2CKeyboardComponent: no I2C bus on port %d - add DekiI2c::I2CBusComponent before "
                       "this step in the boot scene. No keyboard.",
                       (int)i2cPort);
        if (onComplete) onComplete(true);
        return;
    }

    const uint8_t address = static_cast<uint8_t>(i2cAddress);
    auto keyboard = std::make_unique<I2CKeyboard>(
        [bus, address](uint8_t& out) { return bus->ReadRaw(address, &out, 1); },
        [bus, address]() { return bus->Probe(address); });

    if (keyboard->Initialize())
    {
        ::DekiInput::DekiInput::SetInput(std::move(keyboard), "I2CKeyboard");
        DEKI_LOG_INFO("I2CKeyboardComponent: looking for a keyboard at 0x%02X on I2C port %d", (unsigned)address,
                      (int)i2cPort);
    }

    // Nothing here has to outlive the boot scene: DekiInput owns the driver,
    // and a bus, once set up, is never torn down.
    if (onComplete) onComplete(true);
}

#else

void I2CKeyboardComponent::Setup(SetupCallback onComplete)
{
    DEKI_LOG_ERROR("I2CKeyboardComponent: this build has no deki-i2c, so there is no keyboard; install deki-i2c");
    if (onComplete) onComplete(true);
}

#endif

}  // namespace DekiInput
