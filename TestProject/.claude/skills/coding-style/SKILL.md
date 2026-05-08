---
name: coding-style
description: Describes coding style and naming conventions. Invoke this skill whenever writing code for any NEXUS plugin.
---

# Coding Style

## File Structure

Every file starts with:
```cpp
// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.
```

Every `.h` also adds `#pragma once` immediately after the copyright.

## Formatting

Style is enforced via `.editorconfig`. Key rules:
- Tabs, width 4; max line length 150
- Inline brace style — no brace-on-new-line for namespaces, types, or functions
- Pointer/reference alignment: left (`int* Ptr`, not `int *Ptr`)

## Naming Conventions

Standard UE prefixes apply, with NEXUS adding `N` after the type prefix:

| Prefix | Type | NEXUS example |
|---|---|---|
| `A` | AActor subclass | `ANDebugActor` |
| `U` | UObject subclass | `UNDynamicRef` |
| `F` | Struct | `FNActorPool` |
| `E` | Enum | `ENActorComponentLifecycleEnd` |
| `T` | Template | |
| `S` | SWidget | |
| `I` | Interface | `INActorPoolItem` |
| `b` | bool field | |

## Module API Macro

Every exported class and free function must be tagged with its module's API macro:

```cpp
class NEXUSACTORPOOLS_API UNActorPoolSubsystem : public UTickableWorldSubsystem { ... };
```

The macro is `NEXUS<UPPERCASEPLUGINNAME>_API`.

## Includes & Forward Declarations

- Include the module's `Nexus<Name>Minimal.h` rather than the full module header.
- Prefer forward declarations in headers over includes where only a pointer/reference is needed:

```cpp
class UNActorPoolObject;
class UNActorPoolSet;
class UNActorPoolSpawnerComponent;
```

## Namespaces

Constants and helper data live in `namespace NEXUS::<PluginName>::<SubCategory>`:

```cpp
namespace NEXUS::ActorPools::InvokeMethods
{
    inline FName OnSpawned = TEXT("OnSpawnedFromActorPool");
}
```

Do not put types or function definitions in these namespaces — they are for constants and inline data only.

## UObject Member Variables

Use `TObjectPtr<>` for all UObject references held as member variables:

```cpp
TArray<TObjectPtr<AActor>> InActors;
TObjectPtr<UNActorPoolObject> PoolObject;
```

## UCLASS / USTRUCT Specifiers

```cpp
UCLASS(BlueprintType, ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pool Subsystem")
class NEXUSACTORPOOLS_API UNActorPoolSubsystem : public UTickableWorldSubsystem
```

- `ClassGroup` is always `"NEXUS"`
- `DisplayName` format is `"NEXUS | <Human Readable Name>"`

## UFUNCTION Specifiers

```cpp
UFUNCTION(BlueprintCallable, DisplayName="Get Actor", Category = "NEXUS|Actor Pools",
    meta=(DocsURL="https://nexus-framework.com/docs/..."))
```

- `Category` format is `"NEXUS|<PluginName>"` (no spaces around `|`)
- `DisplayName` is human-readable, title case, no "N" prefix

## Editor-Only Members

Wrap editor-only fields and methods in `#if WITH_EDITOR`:

```cpp
#if WITH_EDITOR
    FString Name;
    static int32 ActorPoolTicket;
#endif // WITH_EDITOR
```

## Interface Implementation Section Markers

Use `//~ClassName` and `//End ClassName` to delimit interface implementation blocks:

```cpp
//~UTickableWorldSubsystem
virtual void OnWorldBeginPlay(UWorld& InWorld) override;
virtual void Tick(float DeltaTime) final override;
//End UTickableWorldSubsystem
```

## Doc Comments

Use Doxygen-style `/** */` for all public API:

```cpp
/**
 * Brief one-line description.
 * @param ParamName Description of the parameter.
 * @return Description of the return value.
 * @note Thread-safety, usage constraints, or behavioural caveats.
 * @remark Intended audience or manual-use warnings.
 * @see <a href="https://nexus-framework.com/docs/...">TypeName</a>
 */
```

- `@note` — thread safety, must-be-on-game-thread, does-not-check-bounds caveats
- `@remark` — "native code only", "manual add requires manual remove" style warnings
- `@see` — links to the docs page for the type or method
- Top-level class/struct/enum/namespace docs use the expanded three-line `/** */` format; methods and members use the collapsed single-line `/** comment */` format.
- Use American English spelling (color, behavior, etc.)

## Module Dependencies (.Build.cs)

When adding an include from a module that isn't already a dependency, declare it in the plugin's `.Build.cs`:

- `PublicDependencyModuleNames` for headers included in `.h` files
- `PrivateDependencyModuleNames` for headers included only in `.cpp` files

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "NexusCore" });
PrivateDependencyModuleNames.AddRange(new string[] { "NexusActorPools" });
```

The `.Build.cs` file lives at `Plugins/<Name>/Source/Nexus<Name>/Nexus<Name>.Build.cs` (runtime) or `Plugins/<Name>/Source/Nexus<Name>Editor/Nexus<Name>Editor.Build.cs` (editor).

## Assumptions

Treat the following as non-null — do not add null checks or early-out guards:

- `GEditor` and `GEngine`.
- Pointer parameters tagged `meta=(WorldContext=...)`, and Reference parameters (`Foo&`, `const Foo&`, `const TArray<Foo>&`, `UPARAM(ref)`) on `BlueprintCallable` functions.