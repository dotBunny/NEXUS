# Changelog

## [0.2.0] - ???

### Added

- `NEngineContentValidator` throws configurable severity when any content inside the `Engine` folder is saved.
- VLOG support for all pickers.
- [#65](https://github.com/dotBunny/NEXUS/issues/65) Option to automatically profile multiplayer test clients and standalone server.
- [#64](https://github.com/dotBunny/NEXUS/issues/64) Utilize `DefaultTransform` option on `FNActorPoolSettings` as the return location and the scale when spawning.
- [#63](https://github.com/dotBunny/NEXUS/issues/63) Add `ServerOnly` flag by default to `FNActorPoolSettings`.
- [#31](https://github.com/dotBunny/NEXUS/issues/31) Added a watermark option to `NSamplesDisplayActor`. 
- [#45](https://github.com/dotBunny/NEXUS/issues/45) Configurable default `NActorPool` settings via project settings (`Projct Settings > Nexus > Actor Pools`).
- [#47](https://github.com/dotBunny/NEXUS/issues/47) Add update channel support to the framework.
- Ability to determine if a game is running inside a multiplayer test via both utility and blueprint library methods.
- Support programatically allowing/disallow config files to be staged via `FNEditorUtils::AllowConfigFileForStaging` and `FNEditorUtils::DisallowConfigFileForStaging`.
- Add safety check when creating actor pools that we are not getting passed a `nullptr`.
- `INActorPoolItem` now has a dynamic multicast delegate (`OnActorOperationalStateChanged`) that fires when changing states.
- `UNActorPoolLibrary` supporting methods to bind to `INActorPoolItem` delegate.
- Configurable distribution inner & outer visualizer colors in `Editor Preferences > NEXUS > Core (User)`
- `External` tool section to Unreal default `Tools > Profile` menu with `NetworkProfiler` when present.
- Samples content now contains `ANSamplesHUD` and `ANSamplesPawn` to support making documentation easier.
- `UNDeveloperSubsystem` in non-release builds allowing for monitoring of object counts and providing details.

### Fixed
- `ANSamplesDisplayActor` no longer causes headless servers to crash.
- `FNActorPool` will properly timeslice their strategies.
- `FGCScopeGuard` guards like a boss during tests now.
- Crashes when trying to Pan a non-blueprint editor that uses the blueprint editor (Curves, etc).

### Changed

- [[#48](https://github.com/dotBunny/NEXUS/issues/48)] Use `HideCategories` instead of using a layout customizer pattern.  
- Corrected a comparison link at bottom of `CHANGELOG`.
- Renamed `InstanceObjects` to `DelayedEditorTask` types.
- Validators now have a combined configuration / strictness level in `Project Settings`.
- Performance testing framework can now use `GCScopeGaurd` when doing measured tests.
- Properly renamed validators with the N prefix.
- `SplineLevelReference` will only be visible when `APSD_Spline` is selected as the `Distribution` on `UNActorPoolSpawnerComponent`.
- Renamed `bDontDestroyStaticActors` / `bDontDestroyNonInterfacedActors` to `bIgnoreStaticActors` / `bIgnoreNonInterfacedActors`.
- `FNActorPoolSettings` now uses a bitmask `Flags` configuration option instead of a long list of boolean flags.
- Clarified all `int` to `int32` to standardize with Unreal platform definitions.
- Disabled `Automatically Set Usage in Editor` flag on `M_NDebug`, `M_NDebugText` and `M_NWireframe` materials.
- General spelling and grammar fixes to `CHANGELOG.md`.
- Altered `NSamplesDisplayActor` to support replication.
- `Inside or On` abbreviated to `IO`.
- Miscellaneous fixes and tweaks to support rolling out documentation.
- Optimized `FNActorPool` to use a cached `IsStubMode()` for clients using a server-only pool.

### Removed

- `bAllowCreateMoreObjects` became obsolete on `FNActorPoolSettings`.
- Extra `bRequiresDeferredConstruction` on `FNActorPool`, replaced with a flag.

### Core Redirects
```ini
+PropertyRedirects=(OldName="/Script/NexusActorPools.NActorPoolSettings.ReturnMoveLocation",NewName="/Script/NexusActorPools.NActorPoolSettings.StorageLocation")
```

## [0.1.1] - 2025-08-01

### Changed

- Resolves issues with `Shipping` build configuration (accidental reference to `FunctionalTesting` modules in `Core`).
- Resolves issues with `Shipping` build configuration for `NSamplesDisplayActor`, in shipping builds the test portions will not function.
- Replaced `NTextRenderActor` functionality with `NTextRenderComponent` functionality.

## [0.1.0] - 2025-07-31

### Added

- `NActorPools` *Generalized pooling system for Actors.*
- `NCore` *Functionality used by all NEXUS plugins in the framework.*
- `NDynamicReferences` *Method for referring to runtime Actors without knowing them.*
- `NFixers` *A collection of tools for fixing content in the Unreal Editor.*
- `NMaterialLibrary` *Library of Materials commonly used.*
- `NMultiplayer` *Functionality and tools that are useful when developing multiplayer games.*
- `NPicker` *Selection functionality for points and other items.*
- `NUI` *Components for creating a user interface based on UMG/Slate.*

[0.1.1]: https://github.com/dotBunny/NEXUS/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/dotBunny/NEXUS/releases/tag/v0.1.0
