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

Conventions are declared in `Plugins/.editorconfig` (applies to everything under `Plugins/`). This is editor config, not a gate — no clang-format, pre-commit hook, or CI check enforces it, so existing files may not conform and nothing will fix yours. Match it by hand.

Key rules (see `.editorconfig` for the full set):
- Tabs, width 4; max line length 150
- Trailing whitespace trimmed
- Inline brace style — no brace-on-new-line for namespaces, types, or functions
- Pointer/reference alignment: left (`int* Ptr`, not `int *Ptr`)

## Static Analysis (SonarQube)

Unlike `.editorconfig` this **is** a gate — SonarCloud scores the repo and the `README.md` badges come off it. C++ runs the stock Sonar way profile, so any `cpp:` rule can fire. Highest-value rules to write to first time:

- **Brace every conditional/loop body when a statement follows it** (`cpp:S2681`). The same-line form trips it too: `if (!ensure(X)) return;` followed by more code at the same indent is a violation.
- Assignments out of conditions (`S1121`); no shadowing (`S1117`); no nested ternaries (`S3358`); merge lone nested `if`s (`S1066`).
- `TArray` over C-style arrays (`S5945`); `constexpr`/namespace constants over `#define` (`S5028`); no `protected` members (`S3656`); `const` on unmodified pointer/ref params (`S995`).
- Members initialised in-class or in the initialiser list (`S3230`); no unused locals, params, or assignments (`S1481`, `S1172`, `S1854`).
- No commented-out code, no stray `TODO` (`S125`, `S1135`) — file an issue instead.

Sonar's naming rules are not active, so they never override the UE conventions below.

Full rule set, the four rules NEXUS deliberately trades away, and the `#SONARQUBE-DISABLE-CPP_S<id>` suppression-marker convention: **`sonarqube.md`** in this skill directory. For what is flagged right now, use the `sonar-issues` skill.

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
- Prefer a forward declaration (`class UNActorPoolObject;`) in headers over an include where only a pointer/reference is needed.

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

## Blueprint Event Hooks (`K2_` prefix)

`BlueprintImplementableEvent` / `BlueprintNativeEvent` functions **declared as `UCLASS` members** — where C++ calls *into* Blueprint — take a `K2_` prefix plus a `DisplayName` that drops it, keeping the Blueprint-facing symbol from colliding with the native function or delegate for the same concept (mirrors Epic's `K2_DestroyActor`):

```cpp
/** Blueprint hook fired once the pool has handed this actor out. */
UFUNCTION(BlueprintImplementableEvent, Category = "NEXUS|Actor Pools", DisplayName = "On Spawned From Pool")
void K2_OnSpawnedFromPool();
```

- **Event hooks only.** `BlueprintCallable` / `BlueprintPure` stay unprefixed (`GetActor`, `IsRegistered`).
- **`DisplayName` is mandatory** — UE's auto-prettifier does not strip the prefix, so the node would read "K2 On Spawned From Pool".
- **`Receive*` is the accepted equivalent for `AActor` lifecycle hooks** (Epic uses `ReceiveBeginPlay`, `ReceiveTick`), on the same `DisplayName` terms. Don't mix both in one type — `ANSamplesDisplayActor` (`Samples/Shared/`) is the established `Receive*` type. Prefer `K2_` for new code unless the hook mirrors an `AActor` lifecycle event.
- **`UINTERFACE` events are exempt** and stay unprefixed — UHT's generated `Execute_<Name>` / `<Name>_Implementation` already qualify the symbol, and prefixing would push `K2_` into every call site and consumer override. `INCellJunctionFiller`, `INCellInitialized`, `INListViewEntry` follow this.
- **Renaming an existing event orphans Blueprint nodes** (Blueprint binds by `UFunction` name). Before renaming, `grep -rla "<OldName>" Samples/ TestProject/Content/ --include="*.uasset" --include="*.umap"`; if anything hits, add a `+FunctionRedirects=` entry under `[CoreRedirects]` in `TestProject/Config/DefaultEngine.ini` and note it in `CHANGELOG.md`.

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