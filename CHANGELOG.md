# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***"Initial Release"***
> Put some stakes in the ground and decided this was enough to make the repo public. The `dev` branch has stuff that is not ready yet, targetting next releease.

## [0.1.1] - 2025-08-01

### Fixes

- Resolves issues with `Shipping` build configuration (accidental reference to `FunctionalTesting` modules in `Core`)

### Known Issues

- The `NSamplesDisplayActor` is still based off of the `AFunctionalTest` which is part of `FunctionalTesting`, thus any usage of it will prevent a shipping build from completing.

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

[0.1.1]: https://github.com/olivierlacan/keep-a-changelog/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/dotBunny/NEXUS/releases/tag/v0.1.0