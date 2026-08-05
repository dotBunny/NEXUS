# Changelog

## [0.3.5] - ?

### Added

- Junctions the graph builders leave unmatched are now paired up and bridged with geometry. A new assembly stage (`FNConnectJunctionsTask`, between pass collection and spawn-list creation) matches each open junction to the nearest other cell's open junction of the same socket size within `Maximum Range`, routes a spline between the two sockets, and proves the swept volume clear of world geometry, placed cells, and every connector accepted before it. Accepted pairings link their junctions — so neither is capped off with a filler — and their route is cached for the connector actor to loft geometry through.
- `INCellJunctionConnector` actors are now spawned for those pairings. Because a connector spans two cells and cells stream in asynchronously, `FNSpawnJunctionConnectorsTask` hands each pairing to `UNWorldAssemblySubsystem`, whose junctions report in at `BeginPlay`; the connector is built once both ends are live, time-sliced against `Junction Time Slice`, and torn down and rebuilt if either cell streams out and back in. `OnConnectJunctions` gained a final `FNCellJunctionConnectorPath` parameter carrying the cleared route: the center curve plus the four socket-corner curves that bound it, corner-paired end to end so geometry can be built straight across them.
- `FNCellJunctionConnectorEntry` connector lists on `UNCellJunctionComponent` and `UNOrganComponent`, mirroring `FNCellJunctionFillerEntry` (actor, junction-relative `Offset`, `Required Context Tags`, `Tag Counter Constraints`, `Weighting`) and selected the same way. The priority chain is junction, then organ, then the project-wide `Junction Default Connector`; where both ends of a pairing author a list, the deterministic start end wins.
- `World Assembly` project settings `Junction Connectors` (`FNWorldAssemblyJunctionConnectorSettings`), mirrored per-operation onto `FNAssemblyOperationSettings`: `Enabled`, `Maximum Range`, `Maximum Spline Length`, `Spline Radius`, `Sample Step`, `Tangent Scale`, `Maximum Avoidance Attempts`, `Avoidance Offset Step`, `Endpoint Exclusion` and `Allow Multiple Cell Connections`. When a route is blocked, a bounded, seed-free set of detour variants is tried in nearest-first order, so the accepted route is the least-deviating one that clears.
- `Allow Multiple Cell Connections` (default off) caps two cells at one connection between them. With it off, a candidate pair is rejected when its two cells are already linked — whether the graph builders mated them at a doorway or the connector pass itself paired them earlier in the same run — so several openings facing each other across two cells produce one connector rather than a bundle. Only a direct link blocks; cells joined indirectly through others stay connectable, which is usually the interesting case. The connector report breaks these out as `Rejected (Existing Connection)`.
- Connector routes are now checked for how tightly they turn, which is what stops the pass producing hairpins that cannot be walked through — or, past a point, cannot have geometry built through at all. Two checks: a route whose inner wall folds back through itself is always rejected (a validity failure, since the connector's own geometry would self-intersect), and above that a configurable `Minimum Turn Radius Scale` sets a navigability floor. A route rejected as too tight is retried with progressively longer spline tangents, bounded by `Maximum Straightening Attempts`, before the pair is abandoned. Reported as `Rejected (Turn Radius)`, `Rejected (Folded)` and `Straightening Successes`.
- `Minimum Turn Radius Scale` is a multiple of the socket's half-extent *in the direction of the turn*, not a world distance — because how sharply a connector can bend depends on which way it bends. With the default 2×4 socket, a sideways turn only has to clear 50cm while the same turn taken vertically has to clear 100cm, so a single absolute radius would either over-restrict one or permit folded geometry in the other. `1.0` is exactly the fold point, `2.0` (the default) reads as a corridor-width turn, and `0` disables the navigability floor while leaving the fold rejection in place.
- `Connect Inverse` (`Assembly|Junction Matching`, default off) mates two unmatched junctions that already occupy the same opening facing opposite ways. The graph builders only ever grow a *new* cell off an open junction, so a graph that loops back on itself — or two organs that grow into each other — can leave two junctions sitting in exactly the same place with no link between them, and both get capped, walling off what is physically an open doorway. These are linked as a plain cell mating rather than as a connector pairing: the two cells are already flush, so nothing is routed and nothing is spawned. Because it produces a mating rather than a connector, it runs whether or not the connector pass itself is enabled, and a junction with `Disable Connecting` set is still eligible. The node-level graph edge is wired too, so hot paths route through the opening. Reported as `Inverse Matched`.
- `Disable Connecting` on a junction (`FNCellJunctionDetails::bDisableConnector`) excludes it from connector pairing entirely. Unlike `Disable Filling`, which lives on `UNCellJunctionComponent`, this sits on the junction details so it reaches the side-car the connector pass actually reads — that pass runs on a worker thread against the cell's generated junction data and never sees the component. An opted-out junction still mates normally during graph building, is still eligible for `Connect Inverse`, and is still filled per its `Requirements` if it ends up unconnected. Counted in the connector report as `Disabled Junctions`, a subset of `Open Junctions`.
- The World Assembly edit mode now draws accepted connector routes. `UNWorldAssemblyEditorSubsystem` retains each completed operation's pairings (the task-graph context that produced them is destroyed with the operation), and the ed mode draws the center curve in the `Valid` junction color with the four socket-corner curves in a new `Connector Corners` palette entry. Junctions paired by a connector also tint distinctly. Toggled by `World Assembly (User)` > `Draw Junction Connectors`. Works in the default proxy-only preview, where there are no junction components at all.
- `Draw Point Line` on `UNDeveloperLibrary` (and `FNDrawDebugHelpers::DrawPointLine`), connecting an ordered array of points with debug lines, with an optional closing segment back to the first point. The polyline counterpart to `DrawDebugLine`, for the many places a path already exists as an array of points.
- `Draw Junction Connector Path` on `UNWorldAssemblyLibrary`, drawing a `FNCellJunctionConnectorPath` — the route handed to a connector through `OnConnectJunctions`. Draws the path's stored samples, which are the exact points swept for collisions, so what appears is the geometry that was proven clear. A `Subdivisions` pin re-evaluates the center curve from its stored control points and tangents to smooth out the faceting; because it evaluates at a whole multiple of the stored step, the smoothed line still passes through every collision-tested point.
- `FNCellLinkDetails` gained `bConnector` and `ConnectorIdentifier`. Both replicate with the cell, and the identifier — not `ConnectedNodeIdentifier`, which is only unique within one assembly graph — is what rejoins the two ends of a pairing at runtime, since a pairing can span graphs.
- `FNRawMeshUtils::MakeConvexPrism` (`NexusCore`): the swept-volume sibling of `MakeBoxHull`, spanning two arbitrary quadrilateral caps with the same production-shaped topology (8 vertices, 6 quad `FaceLoops`, 12 triangle `Loops`, populated bounds and eagerly validated flags). Cap winding does not matter — a cap wound against the sweep is detected and both are flipped together.
- `FNWorldAssemblyUtils::GetJunctionOutwardDirection` and `GetJunctionWorldCornerPoints`, which state the junction facing and socket-corner conventions in one place. `UNCellJunctionComponent::GetWorldCornerPoints` and `FNWorldAssemblyDebugDraw::DrawSocket` now derive their corners from the latter rather than each rebuilding it.
- Candidate junction pairs are now gated on how the two openings are oriented with respect to each other, not just on how far apart they are. The graph builders already gate the cells they place on `FNRotationConstraints`, but the connector pass runs over cells that are already down — nothing is being rotated, so what is left to judge is the world-space relationship between two fixed openings. `FNWorldAssemblyUtils::AreJunctionsWithinConnectionAngles` settles it on three angles, and a pair has to clear all three: `Maximum Facing Angle` (how far from directly facing each other the two sockets may be), `Maximum Approach Angle` (how far off its own facing a junction's partner may sit, tested at both ends) and `Maximum Elevation Difference` (how far the two may differ in how steeply they face up or down). Evaluated before any routing, so the pass gets cheaper rather than more expensive, and reported as `Rejected (Angle)`.
- `Maximum Elevation Difference` (default 45°) is what separates the two cases a facing angle alone cannot. A ceiling hatch joined to a wall door and a right-angle corridor bend are both exactly 90° of facing, so any facing limit tight enough to reject the first rejects the second with it. Elevation is a socket's angle above or below horizontal — 90° for a ceiling hatch, 0° for every wall opening whichever way it points — so taking the difference leaves horizontal bends at 0° and puts hatch-to-wall at the full 90°. This is why `Maximum Facing Angle` and `Maximum Approach Angle` both default to a loose 90°: the elevation limit does the work, and 90° of approach then means only that a partner must be in front of the opening rather than behind it, which is the case where a route has to leave the socket and loop straight back around the cell it came from.
- `Connection Constraints` on a junction (`FNCellJunctionConnectionConstraints`) optionally replaces the operation's three angle limits with its own for any pairing that junction takes part in. Both ends are consulted and the stricter limit wins, mirroring the cell-and-junction dual veto the placement gate already uses — so an override can only ever narrow what a junction accepts, and deliberately exempting a pairing takes an override on both ends. Distinct from the `Rotation Constraints` beside it, which govern how the owning *cell* may be rotated when the generator places it.

### Changed

- World Assembly sample blueprints have been consolidated to the `Extras` folder.
- Hot paths now route through junction connectors. An accepted pairing wires the node-level graph edge as well as the junction link, so `FNAssemblyGraph::FlagHotPath` treats a connector as the traversable route it is rather than routing around it. Because a pairing can span two organs, a layout that previously produced two independent hot paths may now see them merge.
- `FNCreateSpawnsTask` resolves every graph's hot path before generating any link details, rather than interleaving the two per graph. A connector link can reach a cell in another graph, and the previous ordering would bake in that neighbor's hot-path flags before they had been computed.
- Junction pairs whose sockets open *away* from each other are now rejected outright by the orientation gate, before a route is ever built. They were previously caught downstream by the fold check — meeting both sockets square on forces the route to double back out of each opening, and no tangent scale opens that turn wide enough to clear the socket — which held only while the two sockets sat close enough to force a tight turn. Given enough distance the same pairing curves gently enough to clear every shape limit, so the facing and approach limits now settle it on the orientations alone.
- The edit mode's mated-junction drawing elects a drawer with `>=` rather than `>`. Only one end of a co-located pair draws the socket rectangle, elected by node identifier — but identifiers restart per assembly graph, so a `Connect Inverse` mating that spans two graphs can hold the same one on both ends and the strict comparison elected neither, leaving a visible hole. A tie now draws twice, which on two identical co-located rectangles is indistinguishable from drawing once.
- The three existing `Junction Default Connection` project settings moved into the new `Junction Connectors` struct under `Assembly|Junction Connecting`, alongside `Junction Default Connector`. Their `DefaultNexusGame.ini` keys change as a result; they shipped with defaults only, so nothing authored is lost.

### Removed

## [0.3.2] - 2026-07-30

>  It's IMPORTANT that you rebake any `UNCell` data (use the commandlet) as calculations and existing data needs to be rebuilt.

### Added

- `UNOrganComponent` now have both `Context Tag` and `Tag Counter` requirements.
- `UNOrganComponent` `Direction Mode` (`ENOrganDirectionConstraintMode`) selecting the reference point a cell's directional constraint measures candidate bearings from, per-organ: `Start Bone` (default, the original behavior), `Organ Center` (the volume's geometric center, falling back to the start bone when unbounded), or `Dynamic Centroid` (the running centroid of already-placed cells, falling back to the start bone before the first cell is placed).
- `Quick Assembly` workflow on the World Assembly editor-mode toolbar: select a target `UNOrganComponent` from a dropdown and start/cancel an assembly operation in place, with an optional `Auto Assembly` mode that re-runs on a configurable timer. Adds `World Assembly (User)` settings `Load Level Instances`, `Auto Assembly`, and `Auto Assembly Timer`.
- `NEXUS.WorldAssembly.Flag.Hotpath` assembly flag for creating pathing feedback post assembly in both the `ANCellLevelInstance` as well as junctions.
- Unconnected junctions now fill themselves. `UNCellJunctionComponent` gained a `Fillers` array of `FNCellJunctionFillerEntry`, each naming an actor class, a junction-relative `Offset` transform, `Required Context Tags`, `Tag Counter Constraints` and a selection `Weighting`. A junction left unconnected after generation gates its entries against the generated cell's resolved assembly state (`FNCellAssemblyData`) and picks one weighted-at-random from the survivors; when every entry is gated out it falls back to the project-wide `Junction Default Filler`. Per-junction `Disable Filling` opts a junction out of filling entirely, and `Spawn Filler Immediately` bypasses time-slicing for that one junction.
- `World Assembly` project settings `Delayed Junction Spawning` (default on) and `Junction Time Slice` (default `0.5`ms): junction filling registers with `UNWorldAssemblySubsystem` and is spread across frames instead of spawning every filler inside `BeginPlay`. At least one filler is spawned per tick regardless of the remaining budget.
- `INCellJunctionFiller` interface for filler objects to implement to get callbacks when used to fill empty junctions.
- `INCellJunctionBeginPlay` interface, paired with the junction's `OnBeginPlay Targets` list. Each assigned actor implementing the interface is notified through `OnJunctionBeginPlay` during the junction's `BeginPlay`, receiving the resolved `FNCellLinkDetails` — so gameplay actors can react to whether the junction connected, and to which cell and junction.
- `UNCellJunctionComponent` fillers now honor `ENCellJunctionFillDepthMode`: `Forward` (extends along the junction facing, the prior behavior), `Backward` (opposite), and `Centered` (straddles the socket plane) — each with a `Default` variant driven by the project `Socket Depth` and an `Override` variant driven by the junction's `Override Fill Depth`. `UNWorldAssemblyLibrary::GetJunctionFillDepthAnchor` (signed distance along the junction forward axis) and `GetJunctionFillDepthOffset` (that distance as a world-space vector) expose it to fillers, and the junction visualizer draws a grey depth-volume box reflecting the selected mode.
- The junction `Filler Visualizer` preview respawns when `Fill Depth Mode` or `Override Fill Depth` is committed so the spawned preview re-reads the depth through `OnInitializedFromJunction` (interactive slider drags are ignored until released).
- `UNPickerSettings` project settings (Project Settings > NEXUS > Picker, stored in `DefaultNexusGame.ini`) exposing point-projection configuration: `Trace Complex`, `Nav Query Extent`, `Nav Agent Radius`, and `Nav Agent Height`.
- `World Assembly` project settings `World Collisions` (`FNWorldAssemblyWorldCollisionSettings`), controlling what the virtual-world capture treats as collision: `Actor Ignore Tags` (queried on top of the `NWorldCollision_Ignore` markup tag), `Exclude Non-Collision Enabled Actors`, and `Include Player Starts`.
- `World Assembly` project setting `Support Seamless Travel`, which makes the subsystem periodically poll for `PlayerController`s and spawn relays for them. Left off, call `UNWorldAssemblySubsystem::SpawnRelay(PC)` from the GameMode instead.
- `World Assembly (User)` `Color Palette` settings for the editor-mode draw pass: bones (`Valid`/`Invalid`), cells (`Bounds`/`Hull`), and junctions (`Unfilled`/`Valid`/`Invalid`). Changes re-cache onto the edit mode immediately, including across undo/redo.
- `World Assembly (User)` settings `Draw Unfilled Junctions` (debug markers for junctions left unconnected during an operation) and `Toast Editor Assembly Operations` (a toast notification when an editor-triggered operation completes, plus Quick Assembly run summaries).
- `UNUpdateCellDataCommandlet`, exposed in the editor under Tools > Commandlets > `Update Cell Data`. It loads each `UNCell`'s referenced world, regenerates the cell data and re-saves the side-car, reporting drift as it goes. `-ErrorOnChanges` logs out-of-date cells as errors and returns non-zero instead of updating them (validation/CI gating), and `-CommitChanges` submits the updated assets through the project's source control provider.
- `Collect Junction Components` command on the World Assembly editor-mode junction menu, gathering the `UNCellJunctionComponent` on the selected actor.
- `UNOrganComponent` `Required` toggle. When false, the organ may produce no results and the assembly operation is still considered a success.
- `FNWorldUtils::IsStreaming`, reporting whether a world still has level streaming in flight.
- Blueprint accessors on `UNWorldAssemblyLibrary` for the new hot-path flag (`Is HotPath`, `Is HotPath (Shortest)`, `Is HotPath (Sequential)`, each with an exec-branching `?` variant) and for mutating a placed cell's generated state (`Has Context Tag(s)`, `Append Context Tags`, `Remove Context Tags`, `Add Tag Counter`, `Subtract Tag Counter`), alongside junction geometry helpers (`Get Junction World Size`, `Get Junction World Size (Shifted)`, `Get Junction World Corner Points`).
- Blueprint nodes, types and settings across the framework now carry `DocsURL` metadata, so the right-click *View Documentation* action opens the matching page on `nexus-framework.com`.
- `UNCell` validator checks for `UNCellJunctionComponent` that are attached to non-static movability Actors/components.
- `UNCellJunctionComponent` now has additional filling options related to toggling cell/level specific actors based on its status.
- Dedicated test modules across the framework. Unit and performance suites previously lived in a `Tests/` folder inside each plugin's `Editor` module; every plugin with coverage now ships a `Nexus<Name>Tests` (`DeveloperTool`) module for tests that exercise only runtime/shippable code, plus a `Nexus<Name>EditorTests` (`Editor`) module where suites reference editor-only symbols (`NexusCore`, `NexusActorPools`, `NexusUI`, `NexusWorldAssembly` have both; `NexusDynamicRefs`, `NexusGuardian` and `NexusPicker` need only the former). Game targets no longer compile editor-only tests they can never execute, and runtime coverage no longer requires the editor module to be loaded.
- `FNBoundsBVH` (`NexusCore`): an immutable bounding-volume hierarchy over an indexed set of `FBox` bounds — the object-level counterpart to `FNMeshBVH`, answering "which of these N objects could overlap this box" without touching the other N-1. Entries with invalid bounds are dropped at build time while survivors keep their original index, so results index straight back into the caller's array. `QueryOverlaps` writes into a caller-owned array (reset, not reallocated) and is templated on its allocator, so a caller in a hot loop can pass a `TInlineAllocator` array and stay off the heap entirely.
- `FNMeshBVH` (`NexusCore`): an immutable bounding-volume hierarchy over an `FNRawMesh`'s triangles, turning the brute-force point-containment and nearest-surface-distance sweeps in `FNRawMeshUtils::ComputePointDepthInside` from O(triangles) into roughly O(log triangles). Built for author-time diagnostics that sample a large static non-convex mesh at a handful of points per frame — World Assembly's merged world-collision mesh as read by the bone visualizer being the motivating case. Non-triangle loops are skipped at build time.
- `FNMersenneTwisterState` plus `FNMersenneTwister::SaveState`/`RestoreState`: a two-number snapshot (originating seed and exact engine draw count) that restores an engine to a precise point in its sequence by re-seeding and replaying. Portable across platforms and compilers because both the engine and `discard()` are fully standard-defined; `RestoreState` is O(draw count) and refuses snapshots beyond `MaxRestoreDrawCount` so a corrupt or hand-edited count cannot hang the replay. `FNMersenneTwisterFriendlyState` is the Blueprint-exposable form, storing the seed as a hex string and the draw count as an `int64` so a snapshot can ride UE serialization or be shown to a user, with `ToNative`/`FromNative` at the boundary.
- `UNOrganComponent` records the random-stream snapshot its most recent successful build finished from (`GetLastRandomState`/`SetLastRandomState`), so a completed organ's generation can be resumed or reproduced.
- `FNAssemblyGraph::QueryCellNodesByBounds`, serving cell-node bounds queries from a spatial index rather than a walk of every node.
- `FNRawMesh::SetVertex`, which moves a single vertex and invalidates everything cached from it. `Vertices` is public, so callers could assign into it directly — but a direct write bypasses every mutator and leaves the convexity / non-tri / bounds flags and the face-plane cache describing the geometry as it was, which is a silent wrong answer rather than a slow one. Per-vertex edits should go through this.

### Changed

- The framework now targets **Unreal Engine 5.8** (`BuildSettingsVersion.V7`, `EngineIncludeOrderVersion.Unreal5_8`). 5.7 is no longer supported.
- Determinism is now explicit rather than ambient, and this is the release's largest source of breakage. The framework-wide shared twister behind `FNRandom::GetDeterministic()` is gone; a deterministic stream is now an object you own and pass. `UNMersenneTwisterObject` holds one for use across a wider scope, Blueprint-exposed under the `NEXUS|Core|Random` category (`Seed`, `Random Bool`, `Random Bools`, `Random Integer In Range`). Every picker's `Twisted` method was consolidated onto the `Next` name, and the old parameterless `Next(OutLocations, Params)` — which drew from the shared stream — no longer exists: `Next` now requires an `FNMersenneTwister&`. **The Blueprint `<Shape>: Next Point` nodes gained a required `Twister Object` pin**, so existing graphs using them need that pin wired to a `UNMersenneTwisterObject` before they will run. `Random Point`, `Tracked Point` and `One-Shot Point` are unaffected.
- `UNWorldAssemblySubsystem::IsReady()` now  `UNWorldAssemblySubsystem::IsReady(bool bWaitOnStreaming = true)` defaulting to waiting for level streaming to complete.
- `ANCellProxy` now hold and pass to `ANCellLevelInstance` information about `UNCellJunctionComponent` and their linkage.
- `UNOrganComponent::bUnbounded` renamed `bUnbound` (redirect below), and its properties were regrouped into `Inputs`, `Requirements` and `Operation` categories.
- `UNOrganComponent::MinimumCellCount` / `MaximumCellCount` now express "no bound" as `0` rather than `-1`, and their `ClampMin` moved to `0` so the old sentinel can no longer be authored. Both gates test `> 0`, so organs already saved with `-1` keep behaving as unbounded — nothing needs re-authoring, the property just reads as `0` once touched.
- `World Assembly` project setting `Cell Time Slice` default lowered from `2`ms to `1`ms.
- `World Assembly (User)` setting `Display Viewport Messages` moved from the `Cell` category to `Notifications` and its backing property renamed `bCellDisplayViewportMessages` to `bNotificationsDisplayViewportMessages`. It is a per-user editor setting, so the only effect of the rename is that the preference resets to its default once.
- `UNWorldAssemblyLibrary::GetContextTags`, `GetContextTagsAdded` and `GetAssemblyTags` return an `FGameplayTagContainer` by value instead of a mutable reference; mutation now goes through the new `Append Context Tags` / `Remove Context Tags` / `Add Tag Counter` / `Subtract Tag Counter` nodes, which write to the placed cell's live state. The separate "state" copy those accessors used to expose is gone (see Removed).
- `UNCellJunctionComponent::DrawDebugPDI` takes explicit valid/invalid colors plus `bIsConnected`, `bDrawBox` and `bDrawCornerLines`, replacing the single `DefaultColor` argument, so the draw path can honor the new user color palette and distinguish connected from unfilled junctions.
- `FNMersenneTwister::GetCallCounter` returns `uint64` instead of `uint32`, and is now a true draw count — rejection-sampled and array draws each count individually, where previously some paths bypassed the counter. That exactness is what makes it usable as the replay offset for `SaveState`/`RestoreState`, but it also means the value no longer matches what the same sequence reported in 0.3.1.
- `FNRandom::GetNonDeterministic()` is now backed by a function-local static rather than a static member, and is documented as game-thread only: it hands back a reference to one shared `FRandomStream`, which is not internally synchronized.
- `FNMersenneTwister` Integer minimum default values now properly negative by default, and `IntegerRange` widens to `int64` before computing its span so the full default range samples correctly. The Double/Float defaults changed sign but not magnitude: `MIN_dbl`/`MIN_flt` are the smallest *positive* normalized values, so `-MIN_dbl`/`-MIN_flt` sit at roughly `-2.2e-308`/`-1.2e-38` — negative, but effectively zero rather than the full negative span. They are deliberately not widened to `-MAX_dbl`/`-MAX_flt`, because `FloatRange`/`DoubleRange` evaluate `(MaximumValue - MinimumValue)`, which overflows to infinity across the full representable span. Author both bounds when sampling a float or double range.
- `FNMultiplayerUtils::GetPlayerIdentifier` now returns `-1` when the `PlayerState` is not available.
- `ANPooledActor` renamed `ANPooledActorBase`.
- Clients with `BeginPlay` flagged `UNOrganComponents` will not generate without authority.
- `FNGameplayTagCounterConstraint` defaults the comparison value to `0` if the tag is not found.
- `FNGameplayTagCounter` no longer floors counts at zero; the mutation API (`Add`/`Subtract`/`Increment`/`Decrement`) now produces signed values, keeping operations losslessly reversible and consistent with `GetDifference`/`Combine`.
- `UNIntegerRangeLibrary::RandomValueFromSeed` / `RandomValueInSubRangeFromSeed` and `UNDoubleRangeLibrary::RandomValueFromSeed` dropped their `FromSeed` suffix to become `RandomValue` / `RandomValueInSubRange`, matching `UNFloatRangeLibrary` which already used the correct names. None of the three ever accepted a seed — each calls the non-deterministic `RandomValue`/`RandomValueInSubRange` on the underlying range, which reads the shared `FNRandom::GetNonDeterministic()` stream — so the suffix promised determinism the functions cannot provide. Blueprint node names are unchanged and the redirects below keep existing graphs resolving; native callers must update.
- `UNFloatRangeLibrary::RandomOneShotValue` and `UNDoubleRangeLibrary::RandomOneShotValue` now take `const int32 Seed` instead of `float`/`double`. `N_RANGE_BASE` only offers `RandomOneShotValue(FRandomStream&)` and `RandomOneShotValue(int32)`, so a float seed was narrowing to `int32` on the way in — every seed between `0.0` and `0.999…` collapsed to `0`, and `1.5` and `1.9` both produced `1`. The pin advertised precision the function discarded. Both sub-range variants and the whole Integer library already took `int32`, so this is the last of the family to line up. **Function redirects cannot remap a parameter type**: a Blueprint node wired to the old real-typed `Seed` pin needs that pin reconnected after upgrading.
- `One Shot` is now `One-Shot` in every Blueprint `DisplayName` that used it: `Random One-Shot Value` and `Random One-Shot Value In Sub-Range` across the Integer/Float/Double range libraries, plus `Random One-Shot Value` and `Random One-Shot Value And Remove` on `UNCollectionsLibrary`. The hyphen is correct for a compound modifier, matches the doxygen prose these labels sit beside, and matches the Picker libraries which already read `Arc: One-Shot Point`. `UNFloatRangeLibrary`'s full-span variant was additionally reordered from `Random Value One Shot (Float)` to match its siblings. Display-only — no function names, parameters, or pins changed.
- Cell junction `Requirements` now respected, weights for junctions are automatically doubled when they are `Required`.
- `INCellJunctionFiller` implementers must add `UNWorldAssemblyLibrary::GetJunctionFillDepthOffset` to their placement inside `OnInitializedFromJunction` (before extruding forward by the fill depth) to respect the `Backward`/`Centered` fill-depth modes; a filler that ignores it always fills `Forward` regardless of the junction's `Fill Depth Mode`. The junction's spawn transform is unchanged, so existing forward-growing fillers keep their behavior for `Forward` modes.
- `UNTextRenderComponent` now warns about replication instead of setting it, with a toggle.
- `UNMultiplayerLibrary::KickPlayer` is now `BlueprintAuthorityOnly`; the Blueprint node only executes on the authority.
- `Collect Junction Components` now bails out with a warning when the target actor is location-locked (such as an `ANCellActor`), since Unreal disables transform editing on components owned by a location-locked actor, which would otherwise leave the collected junctions unmovable.
- Saving a level now writes the `UNCell` side-car to disk via `PostSaveWorldWithContext` instead of synchronously inside the world pre-save broadcast: the cell data is still synced in-memory during pre-save (so it is captured by the level save), but the side-car's own `SavePackage` is deferred until after the world has been written, avoiding a re-entrant package save.
- `FNAssemblyGraphCellNode` builds its junction data (`WorldJunctions`, `FreeJunctionKeys`) on first access rather than in the constructor. None of the placement tests read it — only a node that survives them and is linked into the graph does — so every rejected candidate was filling a `TMap` with a quaternion composition per junction and immediately discarding it. Roughly 29% off the per-candidate construction cost. The lazy fill needs no locking: a candidate node is owned by one builder thread for its entire life and is never published to another.
- `FNOrganGraphBuilderTask::CheckNodeBounds` now queries `FNAssemblyGraph`'s spatial index instead of walking every node in the graph and testing its type. The graph grows while it is queried, so the index covers a prefix with a short linearly-scanned tail and is rebuilt once that tail passes a threshold; node removal invalidates it rather than patching it. Roughly 95% off that check at a thousand placed cells, where it was the builder's single largest per-candidate cost and grew quadratically over a build.
- `FNOrganGraphBuilderTask::DoesWorldCollide` and `DoesExistingNodeWorldCollide` query an `FNBoundsBVH` broadphase instead of scanning every world-collision mesh and every previously-placed cell hull per candidate. The result is identical to the scan it replaces: everything the broadphase excludes has a non-overlapping AABB, which is exactly what `FNRawMeshUtils::GetIntersectDepth` already rejected with its `-1` early-out; meshes whose bounds are invalid get no AABB rejection inside that function and so are tracked separately and always tested; and a candidate whose own hull bounds are invalid falls back to the full scan. Both functions return a bool and draw no RNG, so generation stays byte-identical for a given seed. Note this only pays off above roughly 200 collision primitives inside the organ bounds — below that the scan was never the dominant per-candidate cost.
- `UNCellJunctionComponent::DrawDebugPDI` memoizes its socket-corner cell-hull penetration instead of rebuilding the corner points and re-sweeping the hull on every viewport redraw. The World Assembly editor mode redraws every registered junction each frame, so an idle viewport in a cell-authoring level previously repeated the whole sweep per junction per frame. The memo is keyed on the hull's contents (a CRC over its vertex buffer) rather than a version counter, so it stays correct across undo/redo and the hull-vertex drag path without every hull mutator having to remember to invalidate it.
- `FNOrganGraphBuilderTask::DoesWorldCollide`, `DoesExistingNodeWorldCollide` and `CheckNodeBounds` are now public (`NEXUSWORLDASSEMBLY_API`) so the performance suites exercise the real implementations rather than a copy of their loops.
- `FNAssemblyGraphCellNode::GetWorldBounds` / `GetHullBounds` expose the node's world AABB and its baked hull's AABB; the placed-cell snapshot count moved from `FNOrganGraphBuilderTask` onto `FNVirtualOrganContext` alongside the broadphase that indexes it, since the two must always describe the same set.
- `FNVirtualOrganContext::FilterCellInputData` no longer walks the whole tissue cell pool per call. The pool is indexed into buckets keyed by the socket sizes each cell's junctions expose, and only the bucket for the requested size is visited — cells that could never host the junction are skipped before any gate runs instead of being rejected at the end. Buckets are built in ascending cell order so candidates still reach the weighted output array in the same order, which matters because the builder draws from it with the deterministic RNG. The gain scales with how mixed a tissue's socket sizes are (~3.5x when a quarter of the pool matches); a tissue using one socket size throughout sees no change.
- `FilterCellInputData` also stopped copying `FNCellJunctionDetails` by value in its per-candidate junction loop (a ~150-byte struct, for a loop reading two fields), hoisted its matching-junction scratch array out of the loop, and memoizes the required junction rotation per distinct junction orientation — that composition is a quaternion product plus an `asin` and two `atan2`, and was recomputed per junction per cell despite being a pure function of the junction's inverse quat once the source facing is fixed for the call. Together roughly 2.3x off the call.
- `UNDynamicRefSubsystem::GetObjectsByAllTags` now hashes each non-smallest bucket once and probes it in constant time, instead of running `TArray::Contains` (a linear scan) per object per bucket. The intersection was O(smallest x others x bucket size) — ~11x faster on three 250-object buckets, and the reason it had been running an order of magnitude slower than `GetObjectsByAnyTags` over the same data. Result contents and ordering are unchanged.
- Perf-test duration gates across `NexusDynamicRefs` and `NexusPicker` retuned. The DynamicRefs add/remove suites are quadratic in object count by construction (`Add` is an `AddUnique` scan and the write path folds in a `Compact`) and their gates left no headroom for slower or loaded hardware; the Picker gates were bounding sub-0.1ms regions, which cannot be gated tightly regardless.

### Fixed

- Crash when displaying collision visualizer twice in a level.
- `ANWorldAssemblyRelay` now properly responds to `IsReady` (`ANCellLevelInstances` now register appropriately when on clients)
- Macros for PostEngineInit for modules now function properly during teardown.
- Bounds and Center on `FNRawMesh` now initialized at creation (no more garbage memory comparisons).
- `FNMultiplayerUtils` player-lookup helpers now guard against a null `AGameStateBase`, and `HasWorldAuthority(const UWorld*)` guards against a null `UWorld`, preventing crashes during world startup/teardown and on clients before initial replication.
- `FNHashUtils::djb2` now hashes the string's UTF-8 encoding instead of truncating each `TCHAR` to its low byte, so non-ASCII characters that share a low byte (e.g. `Ω` and `©`) no longer collide and `FNSeedGenerator::SeedFromString` gains full character information from non-Latin seed phrases. Pure-ASCII input hashes are unchanged.
- `CellLinkDetails.bConnected` will only be true connecting to another Cell.
- `UNMultiplayerTestSubsystem` now always removes its `BeginStandaloneLocalPlay` delegate binding when stopping, including on the editor teardown path where the previous guard skipped cleanup.
- `UNBlueprintValidator` now guards against a null `Blueprint` cast and null `Node` entries while iterating graph nodes, matching `UNLevelBlueprintValidator`.
- Ramps in blockout now have simple collision.
- Editing a cell hull vertex left the hull's derived state describing geometry it no longer held. `FNCellRootComponentVisualizer::HandleInputDelta` wrote `FNRawMesh::Vertices` directly, which bypasses every mutator, with two consequences. A move rejected for making the hull non-convex was reverted without re-validating, so a genuinely convex hull kept reporting `IsConvex() == false` — with `Center` and `Bounds` still describing the discarded position — until the next `CalculateHull` or reload, and every later penetration query took the slower non-convex path. Separately, no drag invalidated the face-plane cache, so once the junction visualizer's convex depth path had built it, the penetration depths drawn in the viewport were measured against planes from before the edit. Both writes now go through `FNRawMesh::SetVertex`. Covered by tests that fail against the previous behaviour.
- Data race between concurrent organ builders over placed-cell hulls. `FNProcessPassTask` appends `FNAssemblyGraphCellNode::GetHullCopy()` results into the shared `FNVirtualWorldContext::NodeCollisionMeshes`, and `FNRawMesh`'s copy constructor deliberately drops the face-plane cache and leaves the validation flags dirty — so each hull arrived cold. Every organ builder in the following passes reads those same entries concurrently, and the first query on a cold hull triggered unsynchronized lazy writes: `EnsureCachedFacePlanes` does `SetNumUninitialized` on three arrays and `EnsureValidated` writes four flags. Two threads first-touching the same hull could double-allocate, free out from under each other, or read torn state. Both caches are now warmed on the appending thread, the same treatment `FNProcessVirtualWorldTask` already applied to the world-collision meshes.
- `FNDoubleRange` and `FNFloatRange` returned whole numbers from every `Random*` method. `FRandomStream` splits its samplers by name rather than by overload — `RandRange` takes `int32` only, and the floating-point equivalent is separately named `FRandRange` — so the eight `N_RANGE_BASE` call sites that read `RandomStream.RandRange(Minimum, Maximum)` compiled silently while narrowing both bounds to `int32`. A `0..1` double range produced only `0` or `1`, and a range wider than `int32` (including the defaults) made the conversion undefined rather than merely coarse. All eight now dispatch through `FNRangeSampler::Sample`, which picks `FRandRange` for float/double ranges and leaves `FNIntegerRange` on the identical integral path it always used. The `Next*` methods were never affected — they sample through `FNMersenneTwister`, which does overload `RandRange` per scalar type, which is why only the deterministic half of the API behaved. Note the floating-point samplers are now half-open `[Minimum, Maximum)`, matching `FMath::FRandRange`.
- `FNIntegerRange::Minimum` and the `MinimumValue` defaults on `FNMersenneTwister::IntegerRange`/`RandRange` used `-MIN_int32`, which is signed-overflow undefined behavior (negating `INT_MIN`); all four are now `MIN_int32` directly, the value the expression was folding to in practice. The `FNDoubleRange`/`FNFloatRange` defaults are unchanged but documented accurately: `MIN_dbl`/`MIN_flt` are the smallest *positive* normalized values, so `-MIN_dbl` is effectively zero rather than the "entire representable range" the comments claimed. Widening them to `-MAX_dbl`/`-MAX_flt` would make sampling a default-constructed range evaluate `(Maximum - Minimum)` past infinity, so the bounds stay as they are and the docs now say what they do.
- Non-editor targets no longer fail to compile. Reflected test-helper types (`ANTestPooledActor`, `UNInterfaceMacrosTestSubject`) were wrapped in `#if WITH_EDITORONLY_DATA`, but UHT writes every reflected type into its module's package registration table *unguarded* — so in a Development or Debug **game** target, where `DeveloperTool` test modules still build but `WITH_EDITORONLY_DATA` is `0`, the generated code referenced types the preprocessor had stripped. Reflected test types are now unguarded; only test bodies carry `#if WITH_TESTS`. Editor targets never reproduced this, so it only surfaced when packaging.

### Removed

- Junctions no longer store if they are inside of a hull.
- `ANPooledActor` from the placement pallete.
- `ANCellLevelInstance` no longer maintains state before the choice to place it in the graph.
- `FNPickerUtils` static projection configuration (`CollisionQueryParams`, `NavQueryExtent`, `NavAgentProperties`) replaced by `UNPickerSettings`.
- `FNRandom::GetDeterministic()` and the shared framework-wide `FNMersenneTwister` behind it. Deterministic streams are now owned and passed explicitly (see the determinism entry under Changed).
- `UNWorldAssemblyLibrary::GetContextTagsState`, `GetTagCounterState` and `GetTagCounterStateArray`. The generated cell's tags and counters are no longer split into an authored set and a mutable "state" copy — the getters read the live state and the new mutator nodes write it.
- `FNActorPoolUtils`, whose only member was a `ToString` for `ENActorOperationalState`.
- `UNCellJunctionComponent::GetCornerPoints` and `UpdateHullDerivedData`. Corner points are available in world space through `GetWorldCornerPoints`, and hull-derived data is now computed on demand and memoized by `GetCachedHullPenetration`.

### Core Redirects

```ini
[CoreRedirects]
+PropertyRedirects=(OldName="/Script/NexusWorldAssembly.NOrganComponent.bUnbounded",NewName="/Script/NexusWorldAssembly.NOrganComponent.bUnbound")
+PropertyRedirects=(OldName="/Script/NexusWorldAssembly.NWorldAssemblySettings.DefaultFillerActor",NewName="/Script/NexusWorldAssembly.NWorldAssemblySettings.AssemblySpawningDefaultJunctionFiller")
+PropertyRedirects=(OldName="/Script/NexusWorldAssembly.NCellJunctionComponent.OnBeginPlayCallback",NewName="/Script/NexusWorldAssembly.NCellJunctionComponent.OnBeginPlayTargets")
+PropertyRedirects=(OldName="/Script/NexusWorldAssembly.NCellActor.InitializeCallbackActors",NewName="/Script/NexusWorldAssembly.NCellActor.CellInitializedTargets")
+ClassRedirects=(OldName="/Script/NexusActorPools.NPooledActor",NewName="/Script/NexusActorPools.NPooledActorBase")
+FunctionRedirects=(OldName="NIntegerRangeLibrary.RandomValueFromSeed",NewName="NIntegerRangeLibrary.RandomValue")
+FunctionRedirects=(OldName="NIntegerRangeLibrary.RandomValueInSubRangeFromSeed",NewName="NIntegerRangeLibrary.RandomValueInSubRange")
+FunctionRedirects=(OldName="NDoubleRangeLibrary.RandomValueFromSeed",NewName="NDoubleRangeLibrary.RandomValue")
```

## [0.3.1] - 2026-06-07

### Added

- `FNAssemblyGraphNodes` now have a deterministic seed stored along side their generation, accesible at `INCellInitialized` time.
- `Bad Neighbors` tag grouping for `UNTissue`, preventing adjacent cell placement.
- `NEXUS.WorldAssembly.Flag.AlwaysRelevant` behavioral tag.
- `TagCounters` constraints and operations to `UNTissue`.
- Required `ContextTags` option on `UNTissue` entries.
- Both `FNActorPool` and `UNActorPoolSubsystem` gained the ability to `ReturnAll`/`ReturnAllActors`.
- `FNActorPoolSettings` now has a `SupportFlags` section, including `ReturnAll`.
- Added accessors to a global mutable copy of the `Tag Counters` and the `Context Tags` of a generation, with supporting Blueprint functionality.
- Added `Maximum Node Depth` and `Directional Constraints` to `UNTissue` definitions.
- `Direction Tolerance` setting (default 15-degrees, project-wide and per-operation) controlling how close a candidate's bearing must be to a `UNTissue` `Directional Constraint` to be placed.
- `UNActorPoolSubsystem::GetActorPoolStats()` returns Spawned/Available details for a target `FNActorPool`.
- `FNReportListBlock` now available in reports.
- Option in `World Assembly` project settings to enable output of status messages from operations to the log, normally only visible in the developer overlay.
- `FNActorPool` supports `OnDeferredConstruction` slow-invoking with flag.
- Quick `Has Tag ?` and `Has Exact Tag ?` helper nodes for `FGameplayTags`.

### Changed

- `FNRawMesh` collision meshes used inside of assembly operation tasks bake out their transforms.
- `ANCellActor::AuthorTimeActors` are no longer factored into spatial calculations for `UNCells`.
- GameplayTags renamed `NEXUS.WorldAssembly.BuiltIn.*` to `NEXUS.WorldAssembly.Behavior.*`.
- `OutputTags` was moved to a more encompassing `ContextTags`.
- `FNGameplayTagCounter` does not allow for negative counter values.
- `FNWorldAssemblyEdMode::RenderCellJunctionPenetrationDistance` now always renders the distance underneath the `UNCellJunctionComponent`.
- `WorldAssemblyEdMode` Collision Visualizer now tracks for changes in the level, accounting for its offset.
- Reworked `FNEditorInputProcessor` to not compare pointers, instead look at screen position and button presses to determine if the same.
- Cleaned up `IsPointInsideOrOn` for `FNOrientedBoxPicker` to be less-math.
- `UNDynamicRefSubsystem::GetCount(NDR_None)` will return `0` instead of `-1`.
- `UNDynamicRefSubsystem` now holds registered objects as weak references; it no longer keeps a `UObject` alive simply by referencing it, and entries are pruned lazily as objects are destroyed.

### Fixed

- `ANCellActor` is no longer dirtied simply being opened.
- `UNOrganComponent::Seed` properly overrides Organs' assembly operation seed when value is not `-1`.
- Side-car `NCell` asset is not loaded during cook process, restoring determinism.
- `UNOrganComponent` custom-visualizer `Generate` button now refreshes accordingly after operations have finished.
- `PktJitter` setting now properly applied to clients in `Multiplayer Test`.
- `HasSupportFlag_ReturnAll` now properly queries `SupportFlag`.
- Protect against `NSpawnActorBlueprintAsyncAction` occuring after `UWorld` teardown.
- `FNRectanglePicker` no longer crashes in hollow mode when `MinimumDimensions` equals `MaximumDimensions`; `FNRectanglePickerParams::GetValidRanges` now falls back to the full rectangle instead of returning an empty range that was indexed out of bounds.
- `UNDynamicRefSubsystem` no longer broadcasts `OnAdded`/`OnAddedByName` when re-adding an object already present (a no-op `AddUnique`), keeping the add/remove delegate pairing symmetric.
- DynamicRefs developer overlay no longer leaks `UNDynamicRefObject` list-view wrappers for objects destroyed without removal; `UNDynamicRefObject` now prunes stale entries so emptied wrappers are cleaned up.
- `UNGetActorBlueprintAsyncAction` now guards against a null `UNActorPoolSubsystem` when unbinding its pool-created callback, matching `UNSpawnActorBlueprintAsyncAction`.
- `UNGetActorBlueprintAsyncAction` and `UNSpawnActorBlueprintAsyncAction` now complete (with a null result) and tear down when their soft class fails to load, instead of leaving the Blueprint latent node hung.

### Removed

- `FNActorPool::HasFlag_ShouldFinishSpawning`, was allowing `AActor` to be left in bad states when disabled.

### DefaultGameplayTags Redirects

```ini
[/Script/GameplayTags.GameplayTagsSettings]
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.Starter", NewTagName="NEXUS.WorldAssembly.Behavior.Starter")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.StarterOnly", NewTagName="NEXUS.WorldAssembly.Behavior.StarterOnly")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.NotStarter", NewTagName="NEXUS.WorldAssembly.Behavior.NotStarter")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.Finisher", NewTagName="NEXUS.WorldAssembly.Behavior.Finisher")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.FinisherOnly", NewTagName="NEXUS.WorldAssembly.Behavior.FinisherOnly")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.NotFinisher", NewTagName="NEXUS.WorldAssembly.Behavior.NotFinisher")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.Unique", NewTagName="NEXUS.WorldAssembly.Behavior.Unique")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.RequiredAny", NewTagName="NEXUS.WorldAssembly.Behavior.RequiredAny")
+GameplayTagRedirects=(OldTagName="NEXUS.WorldAssembly.BuiltIn.BadNeighbors", NewTagName="NEXUS.WorldAssembly.Behavior.BadNeighbors")
```

## [0.3.0] - 2026-05-31

### Added

- `FNDeveloperUtils` and `UNDeveloperLibrary` now have `IsBuildDemo()` methods to support `IS_BUILD_DEMO` definitions from build targets.
- Pickers all now have a `Twisted` method to support passing a `FNMersenneTwister`.
- `ANDebugPointActor` for quick point debugging.
- `NexusCore`, `NDynamicRefs`, `NGuardian`, `NUI` all have Unit and Performance-based testing.
- `NValidationMacros` to support providing easy to use validation for Blueprint-facing methods.
- `NGuardian` now supports automatically setting its baseline after a defined period (default 15 seconds).
- `NCell` side-car data file now has an action button, that when clicked in the Content Browser will select the source world/level. 
- `FGameplayTag` support to `NDynamicRefs`.
- `Generate` and `Cleanup` console commands for `UNWordAssemblySubsystem`.ing on

### Changed

- Performance tests now use automation latent commands to attempt to stabalize the world prior to running tests.
- Block meshes now end in `_0` suffix to make auto-incrementing cleaner in map placement.
- `ANSamplesPawn` now replicates movement.
- Synthetic graph navigation has been expanded to being a synthetic-ish event and now works with every asset editor.
- `NActorPool` tests now run in PIE worlds.
- Forces `ANPooledActor` to be `abstract`.
- Updated project-wide to enforcing `American English` for spelling.
- `FNActorPool::DoesSupportInterface` renamed to `FNActorPool::ImplementsPoolItemInterface`
- `FNActorPool::HasInvokeUFunctionFlag` renamed to `FNActorPool::ShouldInvokeUFunctions`
- `FNActorPool::GetInCount` renamed to `FNActorPool::GetAvailableCount`
- `FNActorPool::GetOutCount` renamed to `FNActorPool::GetSpawnedCount`
- `UNActorPoolObject::DoesSupportInterface` renamed to `UNActorPoolObject::ImplementsPoolItemInterface`
- `UNActorPoolObject::HasInvokeUFunctionFlag` renamed to `UNActorPoolObject::ShouldInvokeUFunctions`
- `UNActorPoolObject::GetInCount` renamed to `UNActorPoolObject::GetAvailableCount`
- `UNActorPoolObject::GetOutCount` renamed to `UNActorPoolObject::GetSpawnedCount`
- All `int` now replaced with `int32` to be explicit.
- `NGuardianSubsystem` now does writing of reports using async tasks.
- `FNActorPool::OnDestroyedByActorPool` to `FNActorPool::OnReleasedFromActorPool` to better reflect actual doing
- `UNKillZoneComponent::UnknownBehaviour` to define behavior when an AActor is not known or able to be processed.
- `FNActorPool` preallocates more appropriately for its In/Out arrays.
- Started using pattern in Widgets where `BindWidget` members are validated in `NativeConstructor` only.
- Moved `Multiplayer Test` into `NexusTooling`.
- Renamed `NEXUS: ProcGen` plugin to `NEXUS: World Assembly` (`NexusProcGen` → `NexusWorldAssembly`, `NexusProcGenSamples` → `NexusWorldAssemblySamples`, `NProcGen*` class prefix → `NWorldAssembly*`). `CoreRedirects` in `DefaultEngine.ini` handle package + class renames so existing assets continue to load.
- `FNLevelUtils::DetermineLevelBounds` now by default ignores transient actors, but has an option to include them.
- DynamicRef Overlay now just holds WeakPtrs to the objects.
- NGuardian system now has a tick rate configurable in settings (defaults to 1s).
- When a thumbnail is captured for the NCell in the level editor, it now propogates to the NCell side-car data file as well (but with data visible in the thumbnail)
- `FNObjectSnapshotUtils::Diff` now uses a consumed model to calculate changes (no more copying of entries around)
- `FNOrganGraphBuilderTask` now uses a frontier model to create a node-based graph.
- `NPoseAssetFixer` now prompts with a total number of Pose Assets its about to operate on allowing for cancelling; will not prompt in commandlet.
- `NEXUS.WorldAssembly.BuiltIn.Unique` and `NEXUS.WorldAssembly.BuiltIn.MustHave` tags automatically applied to respective groups.

### Fixed

- `UNEditorUtilityWidget::DelayedConstructTask` no longer divides by a zero geometry size when the widget hasn't been laid out yet, which previously left `UnitScale` as NaN/inf.
- `FNDrawDebugHelpers::DrawString`, `FNPrimitiveFont::DrawPDI`, and `FNPrimitiveFont::DrawBatchString` now take the string as `const FString&`, allowing literals and rvalues to be passed.
- `FNPoseAssetFixer::OutOfDateAnimationSource` will no longer allow operation on `/All`, select a subfolder.
- `UNSamplesLevelActor` and `UNSamplesDisplayActor` now properly build at runtime.
- Fixed crash in `FNWeightedIntegerArray` where removing could end up out of bounds.
- Fixed `FNSpherePicker` scenarios where points could be ever-so-slightly out of the `MaximumRadius`.
- `FNRawMesh::CreateDynamicMesh` no longer crashes on non-triangle based meshes, now returns default and errors.
- NDynamicRefs events fired when removing object from the fast-path when it was not actually previously added.
- `FNEditorUtils::DisallowConfigFileFromStaging` and `FNEditorUtils::AllowConfigFileForStaging` now properly use the project name.
- `FNActorPool` now properly marks created `AActor` as root-object.
- `UNActorPoolSubsystem` will no longer crash returning null actors, and returns false when the actor is destroyed, as it doesnt get returned to the pool.
- `UNMultiplayerTestSubsystem` now concretely shutdown process handles, not relying on natural causes.
- `UNActorPoolSubsystem` crash where a tickable `FNActorPool` gets invoked during world teardown.
- `FNActorPool` crash when applying settings where `UWorld` does not exist.
- `FNActorPool` crash when dealing with MaximumActorCount=0, now minimum of 1.
- `FNActorPool::Return` now rejects actors whose class does not match the pool's `Template`, and rejects actors already present in the pool (non-Shipping builds only) — previously these would silently pollute `InActors` or be handed back twice from subsequent `Get()`/`Spawn()` calls.
- `FNRawMesh` crash when calculating center with no verticies.
- `FNActorPool` protect against the odd change that an actor is requsted and needs to be created when the pools world is being torn down.
- `FNActorPool` spawning with designated rotation now properly applies.
- `INActorPoolItem` now handles invocation during teardown gracefully.
- `FNMersenneTwister::VectorNormalized` now provides `-1` to `1` component values.
- `UNTextRenderComponent` now guards away from visuals on Dedicated Servers, but still replicates data to clients.
- Deferred deletion of sidecar `NCell` data asset to next tick when renaming, speculative crash fix.
- `FNMersenneTwister` now is cross-platform guaranteed, regardless of compiler to produce same results.

### Removed

- `Server_*` variant methods on `NTextRenderComponent` streamlining method calls; use `Set*` methods directly.

### Core Redirects

```ini
[CoreRedirects]                                                  
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorOperationalState",ValueChanges=(("Destroyed","Released")))                                                                              
+EnumRedirects=(OldName="/Script/NexusActorPools.ENActorPoolFlags",ValueChanges=(("BroadcastDestroy","BroadcastRelease")))
+ClassRedirects=(OldName="/Script/NexusMultiplayer.UNMultiplayerLibrary",NewName="/Script/NexusCore.UNMultiplayerLibrary")
+ClassRedirects=(OldName="/Script/NexusMultiplayer.UNTextRenderComponent",NewName="/Script/NexusUI.UNTextRenderComponent")
```

## [0.2.7] - 2026-02-22

### Added

- `ANSamplesLevelActor` now has `bShowLevelName` and `bShowLogo` properties to control whether the level name and logo are shown.
- `ANSamplesDisplayActor` title settings now has `bShowTitleHorizontalRule` allowing for the title to be drawn with or without a horizontal rule.
- `ANSamplesDisplayActor` title settings now has `TitleTextPadding` allowing for the title to have some padding from the edges.
- `Window > Log > Clean Logs Folder` will remove much of the chaff in the projects saved/logs folder.
- `NexusPicker` distributions now support finding the nearest point on a NavMesh (V1) via projection mode, with `FNPickerUtils::NavQueryExtent` controlling the distance from the NavMesh volume projectable.
- `FNOrientedBoxPicker` for generating points in a box with rotation.
- `FNEditorSlateUtils` and `FNSlateUtils`.
- [#117](https://github.com/dotBunny/NEXUS/issues/117)`Collision Visualizer` added to `Tools -> Debug`.
- [#120](https://github.com/dotBunny/NEXUS/issues/120) `FNArcPicker` for generating points in an arc distribution.
- [#70](https://github.com/dotBunny/NEXUS/issues/70) Add `Create Actor Pool Set` button to `UNActorPoolsEditorUtilityWidget` to create an `UNActorPoolSet` based on the currently found pools in **PIE**. 
- `Invoke UFunctions` flag to `UNActorPoolSettings` to enable `UFUNCTION` invoking on non-interfaced `AActors`, specifically invoking `OnCreatedByActorPool`, `OnSpawnedFromActorPool`, `OnReturnToActorPool`, and `OnDestroyedByActorPool` (when enabled) named `UFUNCTION` on the `AActor` if it does not implement the `INActorPoolItem` interface.
- `UNActorPoolsDeveloperOverlay` now has tooltips with detailed information about the behavior of the associated `FNActorPool`.

### Changed

- `NEXUS: Fixers` has been collapsed into a more generalized `NEXUS: Tooling` plugin, this is where _all your base_ Unreal Editor tooling has been shifted as well (from the editor side of `NEXUS: Core`).
- Menu entries were streamlined and repositioned to better fit into the UE ecosystem.
- `NexusDynamicReferences` module renamed `NexusDynamicRefs` avoid unnecessarily long names, this change includes all associated classes and methods.
- `UNDeveloperOverlayWidget` has been renamed to `UNDeveloperOverlay` and moved to the `NEXUS: UI` plugin.
- All `NexusPicker` functionality has been changed to take variants of `FNPickerParams` structs to consolidate options; this is a **breaking** change without any direct upgrade path.
- [#122](https://github.com/dotBunny/NEXUS/issues/122)  `NexusBlockout` now contain simplified collision primitives and UVs.
- `ENActorPoolFlags::ReturnToStorageLocation` renamed to `ENActorPoolFlags::ReturnToStorage` to match expansion to full `FTransfrom` set.
- `UNDeveloperSubsystem` renamed to `UNGuardianSubsystem` and moved to `NexusGuardian` plugin.
- `ANPooledActor` now has `OnCreatedByActorPoolEvent`, `OnSpawnedFromActorPoolEvent`, `OnReturnToActorPoolEvent`, and `OnDestroyedByActorPoolEvent` bindable events.
- `UNActorPoolsDeveloperOverlay` now prefixes pool bars with a color representing the `AActor` interface and invoke state.
- `UNActorPoolsDeveloperOverlay` now responds to `N.ActorPools.DeveloperOverlay.UpdateRate` console variable.

### Fixed

- Ensure that a `UNDelayedEditorTask` removes itself from `UAsyncEditorDelay` complete event when released.
- Added exception for `UDataValidationChangelist` to `UNEngineContentValidator` as when submitting with the Revision Control window the changelist object would prevent validation.
- `FNCirclePicker::IsPointInsideOrOn` and `UNCirclePickerLibrary::IsPointInsideOrOn` now correctly return `true` when the point is inside a rotated circle.
- `FNBoxPicker` and `FNRectanglePicker` no longer have regions unintentionally excluded in the distribution.
- `ANSamplesLevelActor` now properly flags the `SM_SkySphere` not to have any collisions, stopping a warning when building NavMesh.
- [#125](https://github.com/dotBunny/NEXUS/issues/125) Issue with `NPoseAssetFixer` not working on external plugin content.
- `UNActorPoolSubsystem` now properly places `UNActorPoolSet` generated actors in the correct world, no longer leaving some in limbo.

### Removed

- `UNWidgetEditorUtilityWidget` no longer used to wrap content widgets, individuals should extend off `UNEditorUtilityWidget` instead.
- `FNActorPool` half-height calculations and offset (use `FActorPoolSettings::SpawnTransform`).

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
- The `Is Host` and `? Is Host` Blueprint methods have had their display names changed to `Is Server` and `? Is Server` respectively.
- `N_SETTINGS_BASE` now offers a `GetMutable` non-const accessor, copying `N_EDITOR_SETTINGS_BASE`.
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
- External documentation now supports comma-delimited multiple entries.


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
- `NEXUS: ProcGen` has some initial structural work done, but is still not functional.


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

[0.3.2]: https://github.com/dotBunny/NEXUS/compare/v0.3.1...v0.3.2
[0.3.1]: https://github.com/dotBunny/NEXUS/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/dotBunny/NEXUS/compare/v0.2.7...v0.3.0
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
