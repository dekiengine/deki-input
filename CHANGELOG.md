# Changelog

Notable changes to `deki-input`. Engine and editor changes are in the
[engine changelog](https://github.com/dekiengine/deki-engine/blob/master/CHANGELOG.md).

A package's `minEngine` names the engine version it needs. Before 1.0 a
breaking change bumps the minor across the editor, the engine and every
package together, so a package with no changes of its own is still released
alongside one that has them.

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
