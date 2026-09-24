#include "TrackballComponent.h"
#include "Trackball.h"

#include "DekiInput.h"
// The trackball needs deki-gpio, which installing deki-input does not bring: a
// project without it (a desktop one, typically) builds this step as a stub.
#if __has_include("DekiGPIO.h")
#define DEKI_INPUT_HAS_GPIO 1
#include "DekiGPIO.h"   // from deki-gpio
#endif

#include <deki/Engine.h>
#include <deki/LogSystem.h>

#include <memory>

namespace DekiInput
{

#if DEKI_INPUT_HAS_GPIO

void TrackballComponent::Setup(SetupCallback onComplete)
{
    // Like the keyboard: a control that is not there is reported, and the
    // boot carries on.
    DekiGpio::IDekiGPIO* gpio = DekiGpio::DekiGPIO::GetCurrent();
    if (!gpio)
    {
        DEKI_LOG_WARNING("TrackballComponent: this platform has no GPIO backend; no trackball");
        if (onComplete) onComplete(true);
        return;
    }

    const int pins[4] = { upPin, downPin, leftPin, rightPin };
    for (int pin : pins)
    {
        // The ball's lines rest high and pulse low, so each step is one
        // falling edge.
        if (pin >= 0 && !gpio->CountEdges(pin, DekiGpio::Edge::Falling, DekiGpio::Pull::Up))
        {
            DEKI_LOG_ERROR("TrackballComponent: cannot count edges on GPIO %d; no trackball", pin);
            if (onComplete) onComplete(true);
            return;
        }
    }
    if (clickPin >= 0)
        gpio->SetInput(clickPin, clickActiveLow ? DekiGpio::Pull::Up : DekiGpio::Pull::Down);

    const int up = upPin, down = downPin, left = leftPin, right = rightPin, click = clickPin;
    const bool activeLow = clickActiveLow;
    auto ball = std::make_unique<Trackball>(
        [gpio, up, down, left, right, click, activeLow]()
        {
            Trackball::Sample s;
            if (up >= 0) s.up = gpio->TakeEdges(up);
            if (down >= 0) s.down = gpio->TakeEdges(down);
            if (left >= 0) s.left = gpio->TakeEdges(left);
            if (right >= 0) s.right = gpio->TakeEdges(right);
            if (click >= 0) s.pressed = gpio->Read(click) != activeLow;
            return s;
        });

    ball->SetMode(mode == TrackballMode::Pointer ? Trackball::Mode::Pointer : Trackball::Mode::Keys);
    ball->SetPixelsPerStep(pixelsPerStep);
    // The screen: the framebuffer, sized once the display is set up.
    ball->SetPointerAreaSource([](int32_t& w, int32_t& h)
    {
        auto& engine = Deki::Engine::GetInstance();
        w = engine.GetScreenWidth();
        h = engine.GetScreenHeight();
    });

    if (ball->Initialize())
    {
        ::DekiInput::DekiInput::SetInput(std::move(ball), "Trackball");
        DEKI_LOG_INFO("TrackballComponent: trackball on GPIO %d/%d/%d/%d, click %d, as %s", up, down, left, right,
                      click, mode == TrackballMode::Pointer ? "a pointer" : "arrow keys");
    }

    if (onComplete) onComplete(true);
}

#else

void TrackballComponent::Setup(SetupCallback onComplete)
{
    DEKI_LOG_ERROR("TrackballComponent: this build has no deki-gpio, so there is no trackball; install deki-gpio");
    if (onComplete) onComplete(true);
}

#endif

}  // namespace DekiInput
