# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - ???

### Added

- `NEngineContentValidator` throws a configurable severity when any content inside of the `Engine` folder is saved.
- VLOG support for all pickers.
- [[#31](https://github.com/dotBunny/NEXUS/issues/31)] Added watermark option to `NSamplesDisplayActor`. 
- [[#45](https://github.com/dotBunny/NEXUS/issues/45)] Configurable default `NActorPool` settings via project settings (`Projct Settings > Nexus > Actor Pools`).
- [[#47](https://github.com/dotBunny/NEXUS/issues/47)] Add update channel support to the framework.
- Ability to determine if game is running inside of a multiplayer test via both utility and blueprint library methods.
- Support to programatically allow/disallow config files to be staged via `FNEditorUtils::AllowConfigFileForStaging` and `FNEditorUtils::DisallowConfigFileForStaging`.
  
### Changed

- [[#48](https://github.com/dotBunny/NEXUS/issues/48)] Use `HideCategories` instead of using a layout customizer pattern.  
- Corrected comparison link at bottom of `CHANGELOG`.
- Renamed `InstanceObjects` to `DelayedEditorTask` types.
- Validators now have a combined configuration / strictness level in `Project Settings`.
- Performance testing framework now has an option to use `GCScopeGaurd` when doing measured tests.
- Properly renamed validators with N prefix.
- `SplineLevelReference` will only be visible when `APSD_Spline` is selected as the `Distribution` on `UNActorPoolSpawnerComponent`.
- `ReturnMoveLocation` will only be editable on `FNActorPoolSettings` when `bReturnMoveToLocation` is flagged.

### Removed

- `bAllowCreateMoreObjects` became obselete on `FNActorPoolSettings`.

## [0.1.1] - 2025-08-01

### Changed

- Resolves issues with `Shipping` build configuration (accidental reference to `FunctionalTesting` modules in `Core`).
- Resolves issues with `Shipping` build configuration for `NSamplesDisplayActor`, in shipping builds the test portions will not function.

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
