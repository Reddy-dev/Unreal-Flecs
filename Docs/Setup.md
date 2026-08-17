# Unreal-Flecs setup

This guide configures the source version of Unreal-Flecs in an Unreal Engine 5.8 project. The plugin is experimental and is currently tested primarily with the UE 5.8 MSVC toolchain; Clang support is partial.

The plugin contains the Flecs source it uses. A separate Flecs installation is not required.

## Configure the world settings class

Unreal-Flecs creates a `UFlecsWorldSubsystem` for supported game worlds. The subsystem expects the world's settings actor to be `AFlecsWorldSettings`.

Set the project world-settings class in `Config/DefaultEngine.ini`:

```ini
[/Script/Engine.Engine]
WorldSettingsClassName=/Script/UnrealFlecs.FlecsWorldSettings
```

## World Setup
- To setup your world to use Unreal-Flecs, you need to make a Flecs World Settings Asset, then assign this asset as the default world in the map's World Settings.
- Make sure to also have `Use Flecs World` setting enabled in the World Settings.

