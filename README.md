# Deki Input

Docs: https://dekiengine.github.io/deki-input/ (components and properties, generated from the code)

Input dispatch and collision detection system for the Deki Engine.

Part of [Deki Engine](https://github.com/dekiengine/deki-engine).

## Namespace

Types live in `DekiInput`. Scene files store the qualified name, and so does code:

```cpp
using namespace DekiInput;
obj->AddComponent<SomeComponent>();
```

Scenes saved before 0.16.0 used bare names and still load; saving writes the current one.

## Keys

`Keys.h` names the keys every driver reports:

```cpp
#include "Keys.h"

if (DekiInput::DekiInput::IsKeyPressed(DekiInput::Keys::Space)) Jump();
```

A key event carries the key and, separately, what it typed: shift and the A key
is `key == Keys::A` with `character == 'A'`. Use `key` for controls and
`character` for text.

Drivers report through this package: `deki-sdl3-integration` for a desktop
keyboard and mouse, `deki-lovyangfx-integration` for a touch panel, and this
package's own for a keyboard or trackball wired to a board (below).

## Features

| Feature | Components | Needs |
| --- | --- | --- |
| Colliders | `InputCollider` | `deki-rendering`, for the camera that turns a screen position into a world one |
| I2C Keyboard | `I2CKeyboardComponent` | `deki-i2c` |
| Trackball | `TrackballComponent` | `deki-gpio` |

Installing this package installs none of the packages in the last column. A
feature whose package is missing builds as a step that logs what to install,
and the build says so. The board or project that uses the feature brings its
package: `deki-lilygo-boards` brings `deki-i2c` and `deki-gpio`. A desktop
project installs this package alone.

Each target's build contains only the features its scenes use.

### I2C Keyboard

For a keyboard that hands over one typed character per I2C read: the LilyGO
T-Deck's (address 0x55), the M5Stack CardKB (0x5F). Put it in the platform's
boot scene after the I2C Bus step. Such a keyboard reports presses and never
releases, so each character is a press and, on the next update, a release. Its
controller may still be starting at boot; it is looked for over the first five
seconds, and a board without one still boots.

### Trackball

Four lines that pulse once per step and a click switch, such as the T-Deck's
ball. `Keys` mode makes it the arrow keys and Enter: a frame with steps is one
press, released the next frame, so rolling repeats. `Pointer` mode makes it a
mouse, `pixelsPerStep` per step with the click as the button, so a scene built
for touch works unchanged. Steps are counted by interrupt, so a fast roll
loses none.

## Install

Package Manager in the Deki Editor, or `DekiEditor --packages-add deki-input <project>`.

## Dependencies

| Dependency | Type |
|---|---|
| `deki-rendering` | Deki package |

## License

Apache 2.0. See [LICENSE](LICENSE).
