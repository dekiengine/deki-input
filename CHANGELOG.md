# Changelog

Notable changes to `deki-input`. Engine and editor changes are in the
[engine changelog](https://github.com/dekiengine/deki-engine/blob/master/CHANGELOG.md).

A package's `minEngine` names the engine version it needs. Before 1.0 a
breaking change bumps the minor across the editor, the engine and every
package together, so a package with no changes of its own is still released
alongside one that has them.

## Unreleased

### Added
- **`Keys.h`: the key ids, in public.** `DekiInput::Keys::Space`, `Enter`,
  `Up`, `A`..`Z`, `Num0`..`Num9`. They are the values drivers have always
  reported; the list was private to the engine, so every driver restated it
  and games wrote bare numbers. `Keys::ForCharacter` gives the key a typed
  character sits on.
- **`InputEvent::character`**: what a `KEY_DOWN` types, where `key` is the key.
  Shift and the A key is `Keys::A` typing `'A'`. 0 when the key types nothing
  or the driver only knows keys. It is the struct's last field and defaults to
  0, so existing drivers need no change.
- **`I2CKeyboardComponent`**: a boot-scene step for a keyboard that hands over
  one typed character per I2C read - the LilyGO T-Deck's (0x55), the M5Stack
  CardKB (0x5F). Each character is a `KEY_DOWN` and, on the next update, its
  `KEY_UP`. The keyboard's controller may still be starting at boot: it is
  looked for over the first five seconds, then left alone. A missing keyboard
  never stops the boot. Needs `deki-i2c`.
- **`TrackballComponent`**: a trackball on four pulse lines and a click, as the
  arrow keys and Enter (`Keys`) or as a pointer whose click is a mouse button
  (`Pointer`). Steps are counted by interrupt, so none are lost between
  frames. Needs `deki-gpio`.

Both are tested against fake devices; neither has run on hardware yet.

### Changed
- **Features name what they need, and nothing is required of every project.**
  `requires` is empty. Colliders need `deki-rendering` (the camera, for screen
  to world), the keyboard `deki-i2c`, the trackball `deki-gpio`, each declared
  on its feature. Installing this package installs none of them; a feature
  whose package is missing builds as a step that logs what to install. A
  desktop project gets this package alone.
- `InputCollider::GetBounds()` is gone. It returned a `deki-2d` type, which
  made this package include `deki-2d` without depending on it. Read the
  collider's size and padding fields instead.

### Fixed
- The trackball pointer reads the screen size while it runs. It used to take it
  at setup, before a display existed.

## 0.16.0

### Fixed
- `DekiInput_InitSystem` and `DekiInput_ShutdownSystem` stay at global scope.
  The namespace move swept up these two, but they are link-time glue: the
  editor generates a translation unit declaring them as plain externs to bring
  a static simulator or firmware build up, and that generated file cannot know
  a package's namespace. Which is why the symbol carries the package prefix
  itself, as `DekiInput_RegisterComponents` from the reflection codegen always
  has. Namespaced, they broke every simulator and firmware link.

### Changed
- **Moved into the `DekiInput` namespace.** Every component was declared at global
  scope, which made its identity a bare class name — the name a scene file
  stores and the name the registry keys on — so two packages defining one name
  collided there with nothing to tell them apart. Each component carries
  `DEKI_FORMER_NAME` with the name it was saved under before, so existing
  scenes load unchanged and are written back qualified on the next save.
  Code naming these types needs the namespace: `using namespace DekiInput;` or a
  qualified name.
- Enum properties are stored by name rather than by number, so appending to an
  enum or reordering one no longer changes what a saved scene means. Files
  written before this still read.
- `minEngine` 0.16.0. Reflection ABI 17: the package must be rebuilt.

## 0.15.0

### Added
- Unit tests for the gesture claim protocol and the hit test's world-to-bounds
  coordinate flip, with each padding edge checked separately.

### Fixed
- `DEKI_INPUT_API` has one definition. `InputDispatch.h` and
  `DekiInputPackage.h` each spelled it out and the two disagreed about
  `DEKI_ENGINE_EXPORTS`, so the linkage a symbol got depended on which header
  was included first. It now lives in `InputApi.h`, which both include.
