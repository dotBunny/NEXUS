# SonarQube Rule Reference

Companion to the `coding-style` skill. Read this when you need the full rule set, need to justify a suppression, or hit a rule not covered by the summary in `SKILL.md`.

The repo is analysed by SonarCloud as project `dotBunny_NEXUS` (org `dotbunny`). C++ runs the **stock Sonar way profile** — nothing is customised, so any active `cpp:` rule can fire. To query what is flagged *right now*, use the `sonar-issues` skill; this file is the durable rule guidance.

Rule IDs are written `cpp:S2681` in SonarCloud and `CPP_S2681` in suppression markers. For a rule's prose rationale, open the UI (the API returns an empty description body):

```
https://sonarcloud.io/organizations/dotbunny/rules?open=cpp%3AS2681&q=cpp%3AS2681
```

## `cpp:S2681` — multiline blocks should be enclosed in curly braces

Worth spelling out because it is a real bug class, not a taste rule. An unbraced body followed by a statement at the *same indentation* reads as a block but isn't:

```cpp
// Noncompliant — Deactivate() runs unconditionally
if (!PoolObject->IsRegistered())
    PoolObject->Register();
    PoolObject->Deactivate();
```

The trap in this codebase is that the **same-line body trips it too** — a one-line `if` early-out or a one-line `for`, followed by a statement at the same indent:

```cpp
// Noncompliant — flagged at the following line, in both forms
if (!ensure(CellActor != nullptr)) return;
CellActor->Destroy();

for (int32 i = Sides - 1; i >= 0; --i) Bottom.Add(i);
Mesh.Loops.Add(FNRawMeshLoop(MoveTemp(Bottom)));
```

So: **brace the body whenever any statement follows it in the same block.** A same-line unbraced body is only safe as a terminal early-out with nothing after it.

```cpp
if (!ensure(CellActor != nullptr))
{
    return;
}
CellActor->Destroy();
```

Braces also belong at the `if`'s own indent level — an over-indented brace after a multi-line condition trips this rule as well.

## Rules to write to

| Rule | What it wants |
|---|---|
| `cpp:S1121` | No assignment inside a sub-expression — hoist it out of the `if`/`while` condition |
| `cpp:S1117` | No shadowing an outer-scope variable; rename the inner one |
| `cpp:S1066` | Merge a lone nested `if` into its parent with `&&` |
| `cpp:S3358` | No nested conditional operators — use `if`/`else` |
| `cpp:S1871` | Two branches of one `if`/`switch` must not have identical bodies — collapse them |
| `cpp:S1116` | No stray empty statements (`;;`, a `;` after a block) |
| `cpp:S1481` / `cpp:S1172` | Remove unused locals and unused parameters — for a parameter kept for signature compatibility, omit the name |
| `cpp:S1854` | Remove unused assignments — a value assigned then overwritten or never read |
| `cpp:S3230` | Initialise members in-class or in the constructor initialiser list, not in the body |
| `cpp:S995` | Pointer/reference parameters not modified through must be `const` |
| `cpp:S818` | Uppercase the `L` suffix on long literals (`1024L`, not `1024l`) |
| `cpp:S5028` | Prefer a `constexpr` constant or the `NEXUS::<Plugin>::` namespace over a `#define` for a constant value |
| `cpp:S5945` | Use `TArray` / `TStaticArray` / `std::array` over a C-style array |
| `cpp:S924` | A loop with more than one `break`/`goto` wants restructuring |
| `cpp:S125` / `cpp:S1135` | No commented-out code; `TODO` tags are tracked as issues — file them instead of leaving them |
| `cpp:S3656` | Member variables should not be `protected` — prefer private with accessors |
| `cpp:S7035` | Flags `static_cast` of an enum to its underlying type, wanting C++23 `std::to_underlying`; UE5.8 does not give us it, so this one gets suppressed rather than fixed |

Sonar's naming rules (`cpp:S100`, `S101`, `S116`, `S117`) are **not** active in Sonar way, so there is no conflict with the UE naming conventions — never rename a symbol to satisfy a Sonar naming pattern.

## Rules NEXUS trades away

These four account for every suppression in the tree. Hitting one is a prompt to reconsider the design, not an automatic suppression:

| Rule | Fires on | When suppression is legitimate |
|---|---|---|
| `cpp:S107` | More than 7 parameters | Draw/debug-helper signatures where the parameters *are* the API and nearly all are defaulted (`NDrawDebugHelpers`, `NPrimitiveFont`) |
| `cpp:S3776` | Cognitive complexity over 25 | Flat edge-case handling, or a hot path where extraction would cost a call (`NRawMesh`, `NTriangleUtils`) |
| `cpp:S134` | Control flow nested more than 3 deep | Widget-tree / graph walks that are genuinely nested (`FNSlateUtils::FindDockTabWithLabel`) |
| `cpp:S5025` | Manual `new`/`delete` instead of RAII | A type that owns and frees its own node storage (`FNAssemblyGraph`) — but prefer `TUniquePtr` / `Reset()` where ownership really is single-owner |

## Suppression markers

A paired comment at **column 0** — deliberately unindented, so it reads as tooling metadata rather than code — carrying the rule ID and a reason:

```cpp
FNAssemblyGraph::~FNAssemblyGraph()
{
// #SONARQUBE-DISABLE-CPP_S5025 Wanting to own and control memory
    ...
// #SONARQUBE-ENABLE-CPP_S5025 Wanting to own and control memory
}
```

- Always name the rule (`-CPP_S5025`) and always give a reason on the same line.
- Always close the region with a matching `#SONARQUBE-ENABLE`. The bare form (no rule ID) is for whole-file exclusions such as intended macro duplication (`NActorMacros.h`, `NCardinalDirection.h`) — don't use it for a single rule.
- Scope the region as tightly as the construct allows: the function, not the file.
- These are NEXUS's own markers, not a SonarQube feature — Sonar's inline mechanism is `NOSONAR` and exclusions live in project config, so a marker documents an accepted violation rather than removing it from the dashboard. Treat it as the reviewed-and-accepted record.
