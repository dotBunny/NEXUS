---
name: build
description: Describes how to build the NEXUS project and its plugins. Invoke this skill whenever building, compiling, or rebuilding the project or any plugin.
---

# Building

## Build the Editor (most common)

```powershell
& "___UEROOT___\Engine\Build\BatchFiles\Build.bat" NEXUSEditor Win64 Development "___PROJECTROOT___\NEXUS.uproject" -progress
```

## Build Configurations

| Configuration | Use when |
|---|---|
| `Development` | Day-to-day iteration — optimised but with debug info |
| `DebugGame` | Stepping through game code in a debugger |
| `Shipping` | Release / packaging builds |

## Build Targets

| Target | Description |
|---|---|
| `NEXUSEditor` | Editor target — use for local iteration and running tests |
| `NEXUS` | Game target — use for cook / package / shipping |

## Cook, Stage, and Package (UAT)

```powershell
& "___UEROOT___\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="___PROJECTROOT___\NEXUS.uproject" `
    -target=NEXUS `
    -platform=Win64 `
    -clientconfig=Development `
    -cook -stage -package -build -pak `
    -archive -archivedirectory="___PROJECTROOT___\..\Staging\Build"
```

## Diagnosing Build Failures

Build output is written to the terminal; for a detailed log check `Saved/Logs/NEXUS.log`. Look for `error:` lines. Common causes:

- **Missing module dependency** — add to `PublicDependencyModuleNames` (headers in `.h`) or `PrivateDependencyModuleNames` (headers in `.cpp` only) in the plugin's `.Build.cs`.
- **Missing API macro** — tag the class/function with `NEXUS<UPPERCASEPLUGINNAME>_API`.
- **Stale generated files** — delete `TestProject/Intermediate/` and rebuild.
