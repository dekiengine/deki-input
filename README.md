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

## Install

Package Manager in the Deki Editor, or `DekiEditor --packages-add deki-input <project>`.

## Dependencies

| Dependency | Type |
|---|---|
| `deki-rendering` | Deki package |

## License

Apache 2.0. See [LICENSE](LICENSE).
