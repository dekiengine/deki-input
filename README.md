# Deki Input

Documentation: https://dekiengine.github.io/deki-input/ (components and properties, generated from the code)

Input dispatch and collision detection system for the Deki Engine.

Part of the [Deki Engine](https://github.com/dekiengine/deki-engine) package ecosystem.

## Namespace

This package's types live in `DekiInput`. Scene files store the qualified
name, so a component is `DekiInput::SomeComponent` there, and code naming one
needs the namespace:

```cpp
using namespace DekiInput;
obj->AddComponent<SomeComponent>();
```

Scenes saved before 0.16.0 used bare names and still load: every component
records what it used to be called, and a save writes the current name.

## Installation

Install via the Package Manager inside the Deki Editor.

## Dependencies

| Dependency | Type |
|---|---|
| `deki-rendering` | Deki package |

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.
