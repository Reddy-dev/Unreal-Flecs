# Flecs C++ test conversion

`ConvertTests.py` is the plugin-local entry point. It parses upstream and fork
C++ tests with libclang, copies AST-selected helpers/types, and writes generated
Unreal Automation sources, a generated test inventory, and
`flecs_conversion_manifest.json` under
`Source/FlecsLibrary/Tests/Generated`. The generated directory is disposable;
fork sources and this metadata remain in `Plugins/Unreal-Flecs/Scripts`.

The tool requires the Python `clang` bindings and a matching libclang runtime
(`python -m pip install clang`; pass `--libclang` when it is not discoverable).

The upstream convention is a free function named `<Suite>_<case>` (the Flecs
`project.json` inventory can be supplied with `--inventory`). Fork tests must
also carry explicit metadata, either beside the function:

```cpp
// flecs-test: id=ForkComponent_manual_registration category=ForkComponent tags=fork
void ForkComponent_manual_registration(void) { /* ... */ }
```

or in a JSON file based on `flecs_test_metadata.example.json`. Registration
hooks are mandatory for reflected or fork-specific types; native raw Flecs
types are emitted as explicit `World.component<Type>()` calls.

When present, `Scripts/Upstream` and `Scripts/ForkTests` are used as the
default mirror and overlay roots; command-line roots take precedence.

Example:

```powershell
python .\Plugins\Unreal-Flecs\Scripts\ConvertTests.py `
  --upstream-root .\test\cpp\src `
  --fork-root .\Plugins\Unreal-Flecs\Scripts\ForkTests `
  --inventory .\test\cpp\project.json `
  --output .\Plugins\Unreal-Flecs\Source\FlecsLibrary\Tests\Generated `
  --validate
```

`VerifyFlecsTestConversion.py --static-only` validates manifest parity and
source hashes. Without `--static-only` it also builds and runs the filtered
`FlecsLibrary.Generated` Automation category.
