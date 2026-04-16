---
name: build
description: Describes how to build the NEXUS project and its plugins. Invoke this skill whenever building, compiling, or rebuilding the project or any plugin.
---

# Building

## Build the Editor (most common)

Compiles the `NEXUSEditor` target for Win64 in Development configuration. This is the standard build used before running tests or opening the editor.

```powershell
& "___UEROOT___\Engine\Build\BatchFiles\Build.bat" NEXUSEditor Win64 Development "___PROJECTROOT___\NEXUS.uproject" -progress
```

---

## Build Configurations

| Configuration | Use when |
|---|---|
| `Development` | Day-to-day iteration — optimised but with debug info |
| `DebugGame` | Stepping through game code in a debugger |
| `Shipping` | Release / packaging builds |

---

## Build Targets

| Target | Description |
|---|---|
| `NEXUSEditor` | Editor target — use for local iteration and running tests |
| `NEXUS` | Game target — use for cook / package / shipping |

---

## Cook, Stage, and Package (UAT)

Runs the full BuildCookRun pipeline. `clientconfig` accepts any configuration from the table above.

```powershell
& "___UEROOT___\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="___PROJECTROOT___\NEXUS.uproject" `
    -target=NEXUS `
    -platform=Win64 `
    -clientconfig=Development `
    -cook -stage -package -build -pak `
    -archive -archivedirectory="___PROJECTROOT___\..\Staging\Build"
```

---

## Diagnosing Build Failures

Build output is written to the terminal. For a more detailed log, check:

```
Saved/Logs/NEXUS.log
```

Look for lines containing `error:` to identify the root cause. Common causes:

- **Missing module dependency** — a new include references a module not listed in the plugin's `.Build.cs`. Add it to `PublicDependencyModuleNames` (for headers in `.h` files) or `PrivateDependencyModuleNames` (for headers in `.cpp` files only).
- **Missing API macro** — an exported class or free function is not tagged with its module's `NEXUS<UPPERCASEPLUGINNAME>_API` macro.
- **Stale generated files** — delete `TestProject/Intermediate/` and rebuild.
