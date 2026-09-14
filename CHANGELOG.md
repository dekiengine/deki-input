# Changelog

Notable changes to `deki-input`. Engine and editor changes are in the
[engine changelog](https://github.com/dekiengine/deki-engine/blob/master/CHANGELOG.md).

A package's `minEngine` names the engine version it needs. Before 1.0 a
breaking change bumps the minor across the editor, the engine and every
package together, so a package with no changes of its own is still released
alongside one that has them.

## 0.15.0

### Added
- Unit tests for the gesture claim protocol and the hit test's world-to-bounds
  coordinate flip, with each padding edge checked separately.

### Fixed
- `DEKI_INPUT_API` has one definition. `InputDispatch.h` and
  `DekiInputPackage.h` each spelled it out and the two disagreed about
  `DEKI_ENGINE_EXPORTS`, so the linkage a symbol got depended on which header
  was included first. It now lives in `InputApi.h`, which both include.
