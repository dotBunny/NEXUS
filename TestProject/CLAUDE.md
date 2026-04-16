# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Layout

This is the **NEXUS Framework** monorepo. The working directory (`TestProject/`) is an Unreal Engine 5.7 project used to test and validate the framework.

Source code lives in the parent directories:
```
NEXUS/
├── Plugins/        # Core framework plugins (Runtime + Editor modules, the source of truth)
├── Samples/        # Sample content plugins per feature area
├── TestProject/    # UE5.7 test project (this repo root for Claude Code)
│   ├── Source/NEXUS/   # Minimal game module (just module boilerplate)
│   └── Content/        # Test maps and assets
├── .github/        # CI/CD workflows and reusable actions
└── Staging/        # Build output and test results (generated)
```

- The `TestProject` is a thin host project.
- Actual feature work lives in `../Plugins/<PluginName>/Source/`.
- The Unreal Engine source may be found in `C:\UE\UE_5.7`, `D:\UE\UE_5.7`, `E:\UE\UE_5.7`, or `D:\EGS\UE_5.7` depending on the computer. Before using any command that contains `___UEROOT___`, check each candidate path in order and use the first one that exists on disk.
- `___PROJECTROOT___` is the absolute path to the `TestProject/` directory — the current working directory. Resolve it from the working directory rather than hardcoding a path.

## Plugin Architecture

Each plugin follows this layout:
```
Plugins/<Name>/
├── Source/
│   ├── Nexus<Name>/          # Runtime module (Public/ + Private/)
│   └── Nexus<Name>Editor/    # Editor module (includes Tests/ subdirectory)
└── Nexus<Name>.uplugin
```

## Plugin Overview

Active plugins loaded by the `TestProject`:

| Plugin | Purpose |
|--------|---------|
| **NexusCore** | Shared base utilities, test macros, and helpers used by all other plugins |
| **NexusActorPools** | Generalized actor pooling system |
| **NexusBlockout** | Level blockout tools |
| **NexusDynamicRefs** | Dynamic asset reference system |
| **NexusGuardian** | Monitors UObject counts against a baseline; fires warnings/snapshots when configurable thresholds are crossed |
| **NexusMultiplayer** | Networking / multiplayer support |
| **NexusPicker** | Point distribution and picking utilities |
| **NexusProcGen** | Procedural generation (depends on UE's PCG plugin) |
| **NexusTooling** | Editor tools and fixers |
| **NexusUI** | UMG/Slate UI components (depends on CommonUI) |

`NexusCore` is a dependency of all other plugins and provides shared utilities: math helpers, collections, debug draw, actor/level libraries, and the developer-only headers in `Public/Developer/` (test utilities, object snapshots, scope timers, `NDebugActor`).

`NexusCore/Public/Macros/` contains framework-wide macros used across all plugins.

**Samples vs Tests**: Functional test content lives in `Samples/<Name>/Content/` as separate plugins (`NexusXxxSamples`). Unit tests live in `Plugins/<Name>/Source/Nexus<Name>Editor/Tests/`. The two test categories use different exec commands (`Tests.Nexus` vs `NEXUS.UnitTests`).