# Changelog

## [0.2.7]

### Added

- `ANSamplesLevelActor` now has `bShowLevelName` and `bShowLogo` properties to control whether the level name and logo are shown.
- `Window > Log > Clean Logs Folder` will remove much of the chaffe in the projects saved/logs folder.
- All pickers now support finding the nearest point on a NavMesh (V1) via projection mode.

### Changed

- `NexusDynamicReferences` module renamed `NexusDynamicRefs` avoid unnecessarily long names, this change includes all associated classes and methods.
- `FNSlateUtils::FindSDocTab()` a robust search mechanism to find a tab for a widget.
- `UNDeveloperOverlayWidget` has been renamed to `UNDeveloperOverlay` and moved to the `NEXUS: UI` plugin.
- All `NexusPicker` functionality has been changed to take variants of `FNPickerParams` structs to consolidate options; this is a breaking change without any direct upgrade path.

### Fixed

- Ensure that a `UNDelayedEditorTask` removes itself from `UAsyncEditorDelay` complete event when released.
- Added exception for `UDataValidationChangelist` to `UNEngineContentValidator` as when submitting with the Revision Control window the changelist object would prevent validation.
- `FNCirclePicker::IsPointInsideOrOn` and `UNCirclePickerLibrary::IsPointInsideOrOn` now correctly return `true` when the point is inside a rotated circle.
- Stopped `NActorPoolSubsystem` from ticking when it was not initialized.

### Removed

- `UNWidgetEditorUtilityWidget` no longer used to wrap content widgets, individuals should extend off `UNEditorUtilityWidget` instead.

## [0.2.6] - 2026-01-20

### Added

- `Windows > NEXUS > Actor Pools` a tab for monitoring the pressure on the `UNActorPoolSubsystem` in realtime. 
- `FNDeveloperUtils::GetRootSetObjects()` / `FNDeveloperUtils::DumpRootSetToLog()` to help with debugging world tear down issues.
- `FNMultiplayerUtils::GetPing()`
- `UNTextRenderComponent` icon in editor.
- `UNEditorUtilityWidget` persistent flag can be used to automatically restore state across map changes (as the widgets are recreated).
- `FNWidgetUtils` to support converting from serialized to slate space.

### Changed

- Many enumerations have been modernized to `enum class`, leaving only a few `enum` where necessary.
- Console command `NEXUS.ActorPools.TrackStats` renamed to `N.ActorPools.TrackStats`.
- Extracted test logic from `ANSamplesDisplayActor` into its own test object.
- `UNActorPoolSubsystem` now respects `UnknownBehaviour` in `UNActorPoolsSettings` to determine what to do when an `AActor` is returned to the system when a `FNActorPool` does not exist for it.
- `UNTextRenderComponent` will now ensure that its owning `AActor` on `BeginPlay` is replicated.

### Core Redirects

```ini
[CoreRedirects]
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("AOS_Undefined","Undefined")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("AOS_Created","Created")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("AOS_Enabled","Enabled")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("AOS_Disabled","Disabled")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("AOS_Destroyed","Destroyed")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_Create","Create")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_CreateLimited","CreateLimited")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_CreateRecycleFirst","CreateRecycleFirst")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_CreateRecycleLast","CreateRecycleLast")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_Fixed","Fixed")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_FixedRecycleFirst","FixedRecycleFirst")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolStrategy",ValueChanges=(("APS_FixedRecycleLast","FixedRecycleLast")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_None","None")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_SweepBeforeSettingLocation","SweepBeforeSettingLocation")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_ReturnToStorageLocation","ReturnToStorageLocation")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_DeferConstruction","DeferConstruction")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_ShouldFinishSpawning","ShouldFinishSpawning")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_ServerOnly","ServerOnly")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_BroadcastDestroy","BroadcastDestroy")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_SetNetDormancy","SetNetDormancy")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("APF_DefaultSettings","DefaultSettings")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolSpawnerDistribution",ValueChanges=(("APSD_Point","Point")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolSpawnerDistribution",ValueChanges=(("APSD_Radius","Radius")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolSpawnerDistribution",ValueChanges=(("APSD_Sphere","Sphere")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolSpawnerDistribution",ValueChanges=(("APSD_Box","Box")))
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolSpawnerDistribution",ValueChanges=(("APSD_Spline","Spline")))
+EnumRedirects=(OldName="/Script/NexusFixersEditor.ENValidatorSeverity",ValueChanges=(("NVS_Disable","Disable")))
+EnumRedirects=(OldName="/Script/NexusFixersEditor.ENValidatorSeverity",ValueChanges=(("NVS_Warning","Warning")))
+EnumRedirects=(OldName="/Script/NexusFixersEditor.ENValidatorSeverity",ValueChanges=(("NVS_WarningButValid","WarningButValid")))
+EnumRedirects=(OldName="/Script/NexusFixersEditor.ENValidatorSeverity",ValueChanges=(("NVS_Error","Error")))
+EnumRedirects=(OldName="/Script/NexusFixersEditor.ENValidatorSeverity",ValueChanges=(("NVS_Message","Message")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellJunctionType",ValueChanges=(("CJT_TwoWaySocket","TwoWaySocket")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellJunctionType",ValueChanges=(("CJT_OneWaySocket","OneWaySocket")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellJunctionRequirements",ValueChanges=(("CJR_Required","Required")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellJunctionRequirements",ValueChanges=(("CJR_AllowBlocking","AllowBlocking")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellJunctionRequirements",ValueChanges=(("CJR_AllowEmpty","AllowEmpty")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellVoxel",ValueChanges=(("CVD_Empty","Empty")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENCellVoxel",ValueChanges=(("CVD_Occupied","Occupied")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENOrganGenerationTrigger",ValueChanges=(("OGT_GenerateOnLoad","GenerateOnLoad")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENOrganGenerationTrigger",ValueChanges=(("OGT_GenerateOnDemand","GenerateOnDemand")))
+EnumRedirects=(OldName="/Script/NexusProcGen.ENOrganGenerationTrigger",ValueChanges=(("OGT_GenerateAtRuntime","GenerateAtRuntime")))
+EnumRedirects=(OldName="/Script/NexusProcGen.EBuildMethod",NewName="/Script/NexusProcGen.ENullBuildMethod")
+EnumRedirects=(OldName="/Script/NexusCore.ENActorComponentLifecycleStart",ValueChanges=(("ACLS_BeginPlay","BeginPlay")))
+EnumRedirects=(OldName="/Script/NexusCore.ENActorComponentLifecycleStart",ValueChanges=(("ACLS_InitializeComponent","InitializeComponent")))
+EnumRedirects=(OldName="/Script/NexusCore.ENActorComponentLifecycleEnd",ValueChanges=(("ACLE_UninitializeComponent","UninitializeComponent")))
+EnumRedirects=(OldName="/Script/NexusCore.ENActorComponentLifecycleEnd",ValueChanges=(("ACLE_EndPlay","EndPlay")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_North","North")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_NorthNorthEast","NorthNorthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_NorthEast","NorthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_EastNorthEast","EastNorthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_East","East")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_EastSouthEast","EastSouthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_SouthEast","SouthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_SouthSouthEast","SouthSouthEast")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_South","South")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_SouthSouthWest","SouthSouthWest")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_SouthWest","SouthWest")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_WestSouthWest","WestSouthWest")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_West","West")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_WestNorthWest","WestNorthWest")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_NorthWest","NorthWest")))
+EnumRedirects=(OldName="/Script/NexusCore.ENCardinalDirection",ValueChanges=(("CD_NorthNorthWest","NorthNorthWest")))
+EnumRedirects=(OldName="/Script/NexusCoreEditor.ENUpdatesChannel",ValueChanges=(("NUC_GithubRelease","GithubRelease")))
+EnumRedirects=(OldName="/Script/NexusCoreEditor.ENUpdatesChannel",ValueChanges=(("NUC_GithubMain","GithubMain")))
+EnumRedirects=(OldName="/Script/NexusCoreEditor.ENUpdatesChannel",ValueChanges=(("NUC_Custom","Custom")))
```

## [0.2.5] - 2026-01-11

### Changed

- The `main` branch of the GitHub repository is now the base point of development; tags are still created for specific release versions, with those points in time being commited to the `release` branch.
- Code cleanup is driven by [SonarQube](https://sonarcloud.io/summary/overall?id=dotBunny_NEXUS&branch=main) static analysis and suggestions.
- Status check reports for GitHub actions are now marked on individual commits.

### Fixed

- Add proper editor menu cleanup for the `NexusFixers` plugin.

## [0.2.4] - 2026-01-06

### Added

- `FNTestUtils::PrePerformanceTest()` and `FNTestUtils::PostPerformanceTest()` for reusable performance testing steps.

### Changed

- Update detection now bumps the ignored version to the installed version if it is greater than the saved version.
- Exposed all log categories for external reference.

### Fixed

- Accuracy of `FNMethodScopeTimer` improved, as well as fixed TEXT() macro not being closed.

### Removed

- Unnecessary TODOs in non-experimental plugins.

## [0.2.3] - 2025-12-21

### Added

- Automatic `UNActorPoolSet` loading configurable in project `Actor Pool Settings`
- Glyph based `FNPrimitiveFont` supporting rendering `FString` through `PDI` (`FNPrimitiveDrawingUtils::DrawString`) and other drawing interfaces (`FNDrawDebugHelpers::DrawDebugString`).
- Support for `FNDrawDebugHelpers::DrawDebugString` via `UNDeveloperLibrary::DrawDebugString`.
- `UNMultiplayerLibrary` exposing `FNMultiplayerUtils` support for `PlayerIdentifier`.

### Changed

- Altered GitHub actions to dispatch tests based on the initial Unit test, as well as fixed up additional warnings thrown by EGS UE.
- Allow adding multiple `UNCellJunctionComponent` to a single parent.
- `UNCellJunctionComponent` will now draw indicators for the unit size as well as their actual size.
- The `NCellActor` will rename itself to reflect the map name that it creates data for.
- The `Is Host` and `? Is Host` Blueprint methods have had their display names changed to `Is Server` and `? Is Server` resepectively.
- `N_IMPLEMENT_SETTINGS` now offers a `GetMutable` non-const accessor, copying `N_IMPLEMENT_EDITOR_SETTINGS`.
- Renamed `FNSeedGenerator::SeedFromText` to ` FNSeedGenerator::SeedFromString`.
- Lowered warning level to `Log` when creating a new `FNActorPool` that already exists in a nested `UNActorPoolSet`.

### Fixed

- Selecting the `NCellActor` no longer adds it to the current selection, instead clearing and making it the sole selected.
- Tickable subsystems with conditional ticks now properly respect their conditional tick settings.

### Removed

- `N_LOG` and `NE_LOG` macros were removed in favor of using the base `UE_LOG` macro, providing better integration IDE's UE tooling.

## [0.2.2] - 2025-09-24

### Added
- `FNActorPoolSettings::Flags` now has a defaulted `APF_SetNetDormancy` flag.

### Changed

- Optimized `FNActorPool` actor creation to quarter the costs in Editor and Development builds.
- `FNActorPool::CreateActor` now takes an optional count to support bulk creation.

## [0.2.1] - 2025-09-23

### Added

- Native calls to fill arrays with results of `NMersenneTwister` methods.
- Additional blockout shapes for torus' and some larger cuboids.

### Changed

- Exposed `FNMethodScopeTimer` for external linking/usage.
- Optimized some `NMersenneTwister` distribution calls.
- Explicit `ManualStop` option to both `FNTestScopeTimer` and `FNTestScopeTimer`.
- Clarified flags for Actor Pools created during tests.

### Removed

- Internal distribution resets in the `NMersenneTwister`.

## [0.2.0] - 2025-09-19

### Added

- `NEngineContentValidator` throws configurable severity when any content inside the `Engine` folder is saved.
- VLOG support for all pickers.
- [#65](https://github.com/dotBunny/NEXUS/issues/65) Option to automatically profile multiplayer test clients and standalone server.
- [#64](https://github.com/dotBunny/NEXUS/issues/64) Utilize `DefaultTransform` option on `FNActorPoolSettings` as the return location and the scale when spawning.
- [#63](https://github.com/dotBunny/NEXUS/issues/63) Add `ServerOnly` flag by default to `FNActorPoolSettings`.
- [#62](https://github.com/dotBunny/NEXUS/issues/62) Added support for nesting `UNActorPoolSet`.
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
- `stats NActorPools` command to show stats (if tracking enabled).
- Manual `AddReference` / `RemoveReference` access on `UNDynamicReferencesSubsystem`.
- Custom `cpp.hint` file in the plugin folder to mitigate Visual Studio's parser from incorrectly flagging UE macros as not found.

### Fixed
- `ANSamplesDisplayActor` no longer causes headless servers to crash.
- `FNActorPool` will properly time-slice their strategies.
- `FGCScopeGuard` guards like a boss during tests now.
- Crashes when trying to Pan a non-blueprint editor that uses the blueprint editor (Curves, etc.).
- `UNActorPoolSpawnerComponent` will properly now allow BP editing of some variables.

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
- Moved `Leak Check` duration setting to user level preference.
- Clearly defined usage of `FNMetaUtils` static methods in helper macros.
- `NEXUS: Material Library` renamed to `NEXUS: Blockout`.
- External documentation now supports comma delimited multiple entries.


### Removed

- `bAllowCreateMoreObjects` became obsolete on `FNActorPoolSettings`.
- Extra `bRequiresDeferredConstruction` on `FNActorPool`, replaced with a flag.

### Core Redirects
```ini
+PropertyRedirects=(OldName="/Script/NexusActorPools.NActorPoolSettings.ReturnMoveLocation",NewName="/Script/NexusActorPools.NActorPoolSettings.StorageLocation")
+PackageRedirects=(OldName="/NexusMaterialLibrary/",NewName="/NexusBlockout/MaterialLibrary/",MatchSubstring=true)
```

### Experimental

- `NEXUS: Developer Console` renamed to `NEXUS: Developer Menu` still not functioning stub.
- `NEXUS: ProcGen` has some intial structural work done, but is still not functioanl.


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

[0.2.7]: https://github.com/dotBunny/NEXUS/compare/v0.2.6...v0.2.7
[0.2.6]: https://github.com/dotBunny/NEXUS/compare/v0.2.5...v0.2.6
[0.2.5]: https://github.com/dotBunny/NEXUS/compare/v0.2.4...v0.2.5
[0.2.4]: https://github.com/dotBunny/NEXUS/compare/v0.2.3...v0.2.4
[0.2.3]: https://github.com/dotBunny/NEXUS/compare/v0.2.2...v0.2.3
[0.2.2]: https://github.com/dotBunny/NEXUS/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/dotBunny/NEXUS/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/dotBunny/NEXUS/compare/v0.1.1...v0.2.0
[0.1.1]: https://github.com/dotBunny/NEXUS/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/dotBunny/NEXUS/releases/tag/v0.1.0
