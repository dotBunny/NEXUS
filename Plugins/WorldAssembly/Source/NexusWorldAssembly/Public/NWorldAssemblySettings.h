// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NSettingsUtils.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NSettingsMacros.h"
#include "Types/NDirection.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPtr.h"
#include "NWorldAssemblySettings.generated.h"

/**
 * Selects how generated content is replicated to clients during a World Assembly run.
 */
UENUM(BlueprintType)
enum class ENWorldAssemblyNetworkMode : uint8
{
	ReplicatedLevelInstances UMETA(DisplayName = "Replicated Level Instances", ToolTip = "Default behavior replicating level instances based on Replicated Level Instances for all procedures."),
	AlwaysRelevantLevelInstances UMETA(DisplayName = "Always Replicate Level Instances", ToolTip="All created level instances will be flagged as always relevant, ignoring normal relevancy culling methods."),
};


/**
 * Controls which world actors are captured as collision sources during the virtual-world capture phase of an assembly run.
 */
USTRUCT(BlueprintType)
struct FNWorldAssemblyWorldCollisionSettings
{
	GENERATED_BODY()

	/** Additional tags to query for when ignoring actors from world collision detection, on top of the NWorldCollision_Ignore markup tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Actor Ignore Tags", meta=(ToolTip="Additional tags to query for when ignoring actors from world collision detection."))
	TArray<FName> ActorIgnoreTags;

	/** When true, actors with collision disabled are excluded from world collision capture. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Exclude Non-Collision Enabled Actors", meta=(ToolTip="Do not include Actors who have their collision turned off."))
	bool bExcludeNonCollisionEnabledActors = true;

	/** When true, player start positions are captured so generated content avoids them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Include Player Starts", meta=(ToolTip="Player start positions should be considered to be avoided."))
	bool bIncludePlayerStarts = true;
};



/**
 * Tuning for the junction-connector pass, which pairs junctions the graph builders left unmatched and proves a
 * collision-free swept path between each pair.
 *
 * Held both project-wide on UNWorldAssemblySettings and per-operation on FNAssemblyOperationSettings; the assembly
 * task graph reads the operation's copy, since the pass runs on a worker thread and cannot touch the settings object.
 */
USTRUCT(BlueprintType)
struct FNWorldAssemblyJunctionConnectorSettings
{
	GENERATED_BODY()

	/** When false, the junction-connector pass is skipped entirely and unmatched junctions are filled as before. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Enabled",
		meta=(ToolTip="Should unmatched junctions in close proximity be paired up and connected with geometry?"))
	bool bEnabled = true;

	/** Straight-line distance within which two unmatched junctions are considered as a candidate pair. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Maximum Range",
		meta=(ToolTip="The maximum distance a junction can be to be considered to match.", ClampMin="0", Units="cm"))
	float MaximumRange = 5000.f;

	/** Upper bound on the arc length of the connecting spline; a pair whose path exceeds this is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Maximum Spline Length",
		meta=(ToolTip="The maximum spline length allowed to connect two unmatched Junctions.", ClampMin="0", Units="cm"))
	float MaximumSplineLength = 1000.f;

	/** Radius of the coarse clearance sweep run along the center spline before the exact socket-corner test. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Spline Radius",
		meta=(ToolTip="The collision check distance around the spline.", ClampMin="0", Units="cm"))
	float SplineRadius = 200.f;

	/** Spacing between samples when a spline is flattened to a polyline for length and collision testing; smaller is more accurate and slower. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Sample Step",
		meta=(ToolTip="How far apart to sample a connector spline when measuring its length and sweeping it for collisions.", ClampMin="1", Units="cm"))
	float SampleStep = 50.f;

	/**
	 * Spline tangent magnitude at each socket, as a fraction of the straight-line distance between the two junctions.
	 * @note Larger values leave each socket more perpendicular before curving, at the cost of a longer path.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Tangent Scale",
		meta=(ToolTip="How strongly the spline leaves each socket along its facing, as a fraction of the distance between the two junctions.", ClampMin="0", ClampMax="2"))
	float TangentScale = 0.5f;

	/**
	 * Tightest turn a route may make, as a multiple of the socket's half-extent in the direction of the turn.
	 *
	 * Expressed relative to the socket rather than as a world distance because the point at which a turn becomes
	 * impossible depends on which way it bends: the connector's geometry spans the full socket, so a tall narrow
	 * opening can turn far more sharply left than it can up. One multiple covers every turn plane, and every socket
	 * size, without re-tuning.
	 * @note 1.0 is the geometric floor — below it the inside of the connector folds through itself and no geometry
	 *       can be built. Around 2.0 reads as a corridor-width turn. 0 disables the check, leaving only the
	 *       always-on fold rejection.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Minimum Turn Radius Scale",
		meta=(ToolTip="How gentle a connector's turns must be, as a multiple of the socket size. 1.0 is the tightest turn that can physically have geometry built through it; higher values keep routes walkable. 0 disables the check.", ClampMin="0"))
	float MinimumTurnRadiusScale = 2.f;

	/**
	 * Number of progressively straighter variants tried when a route turns too tightly, before the pair is abandoned.
	 * @note Each step lengthens the route, so Maximum Spline Length is what ultimately bounds this — escalation stops
	 *       as soon as a variant blows that budget, since every later one is longer still.
	 * @note Worth more than one or two steps: longer tangents open a turn up to a point and then overshoot into a
	 *       tighter one again, so the value that works often sits in the middle of the range rather than at its top.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Maximum Straightening Attempts",
		meta=(ToolTip="How many progressively straighter paths to try when a route turns too tightly.", ClampMin="0"))
	int32 MaximumStraighteningAttempts = 4;

	/** Number of detour variants tried when the natural path collides, before the pair is abandoned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Maximum Avoidance Attempts",
		meta=(ToolTip="How many alternate spline paths to try when the direct path is blocked.", ClampMin="0"))
	int32 MaximumAvoidanceAttempts = 16;

	/** Distance each successive detour variant pushes its midpoint away from the direct path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Avoidance Offset Step",
		meta=(ToolTip="How far each successive avoidance attempt pushes the spline off the direct path.", ClampMin="1", Units="cm"))
	float AvoidanceOffsetStep = 200.f;

	/**
	 * Distance from each socket over which the owning cell's own hull is excluded from collision testing.
	 * @note A socket sits on its cell's hull surface, so without this every path would collide at both endpoints.
	 *       Beyond this distance the hull is tested again, which is what rejects a path that curls back into its
	 *       own cell.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Endpoint Exclusion",
		meta=(ToolTip="How far from each socket the connected cell's own geometry is ignored when checking for collisions.", ClampMin="0", Units="cm"))
	float EndpointExclusion = 100.f;

	/**
	 * When false, two cells may hold at most one connection between them, and a candidate pair whose cells are
	 * already linked is rejected.
	 * @note "Already linked" covers both a doorway the graph builders mated and a connector this same pass accepted
	 *       earlier, so several openings facing each other across two cells produce one connector rather than a
	 *       bundle. Only a direct link between the two cells blocks — cells joined indirectly through others are
	 *       still free to connect, which is usually the interesting case.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Allow Multiple Cell Connections",
		meta=(ToolTip="Can two cells be connected more than once? When disabled, cells that are already joined - by a doorway or by an earlier connector - will not be connected again."))
	bool bAllowMultipleCellConnections = false;
};


/**
 * Project-wide settings for the NexusWorldAssembly plugin.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "World Assembly Settings", Config=NexusGame, defaultconfig)
class NEXUSWORLDASSEMBLY_API UNWorldAssemblySettings : public UDeveloperSettings
{
	GENERATED_BODY()

	UNWorldAssemblySettings()
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultProxyAsset(TEXT("/NexusWorldAssembly/M_NCellProxy.M_NCellProxy"));
		if (DefaultProxyAsset.Succeeded())
		{
			ProxyMaterial = DefaultProxyAsset.Object;
		}
	}

	N_SETTINGS_BASE(UNWorldAssemblySettings, "World Assembly", "Settings related to World Assembly.");

public:
	/** World-space size of a single voxel used when generating per-cell voxel data; smaller values cost more to compute. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Voxel Size",
		meta=(ToolTip="What is the size to use when generating voxel data around the space of a NCell. This sizing will also be used to calculate some additional meta data as a unit size. It doesn't need to be too tight; just remember the smaller the size, the greater the performance hit."))
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	/** Base unit size (per socket grid cell, in world units) for junctions and bones. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Socket Size",
		meta=(ToolTip="What is the unit base size for things like the NJunctions and NBones."))
	FVector2D SocketSize = FVector2D(50.f, 50.f);

	/** Per-side depth of a junction, used for collision detection in some (PCG) scenarios. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Socket Depth",
		meta=(ToolTip="The depth per side of a junction that is used for collision detection in some (PCG) scenarios.", Units="cm"))
	float SocketDepth = 100.f;

	/** Approximate player collider size, used when reasoning about traversable space. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Player Size",
		meta=(ToolTip="What is the size of the player's collider?"))
	FVector PlayerSize = FVector(72.f, 184.f, 72.f);

	/** How ANCellLevelInstances are replicated to clients: by relevancy (proximity) or as always-relevant. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network", DisplayName="Mode",
		meta=(ToolTip="How should ANCellLevelInstances be replicated to clients, either based on relevancy (proximity) or treated as always relevant."))
	ENWorldAssemblyNetworkMode NetworkingMode = ENWorldAssemblyNetworkMode::ReplicatedLevelInstances;

	/** Range ANWorldAssemblyRelay queries for nearby ANCellLevelInstances when deciding whether a client is loaded. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network", DisplayName="Initial Range",
	meta=(ToolTip="The range to query for nearby ANCellLevelInstances by ANWorldAssemblyRelay, used to determine if the client is considered loaded. Distance is calculated to the world position point of the ANCellLevelInstance."))
	float NetworkNearbyRange = 20000.f;

	/** When true, the subsystem polls for PlayerControllers and adds relays automatically (else call SpawnRelay manually for seamless travel). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network", DisplayName="Support Seamless Travel",
	meta=(ToolTip="This will cause the subsystem to periodically poll for PlayerControllers and add relays for them. You can avoid this method, with Seamless travel if you call SpawnRelay(PC) on the UNWorldAssemblySubsystem manually from the GameMode."))
	bool bSupportSeamlessTravel = false;

	/** Direction used to compute automatic bone placement on an organ volume. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction",
	meta=(ToolTip="The direction used to calculate the automatic bone placement on the volume."))
	ENDirection OrganAutomaticBoneDirection = ENDirection::Backward;

	/** Offset applied on top of the automatic bone direction. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction Offset",
		meta=(ToolTip="Offset value applied to the direction provided by the enumeration."))
	FVector OrganAutomaticBoneDirectionOffset = FVector::ZeroVector;

	/** Settings controlling which world actors are treated as collision sources and avoided during assembly. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly", DisplayName="World Collisions",
		meta=(ToolTip="Settings used for world collision and avoidance."))
	FNWorldAssemblyWorldCollisionSettings  WorldCollisionSettings;

	/** Maximum number of full assembly attempts before a space is considered a complete failure. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly", DisplayName="Retry Count",
		meta=(ToolTip="The maximum amount of full attempts at assembling a space before it is considered a complete failure."))
	int32 AssemblyGenerationRetryCount = 10000;

	/** Maximum hull-into-hull penetration (world units) allowed when mating two cells at a junction. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Matching", DisplayName="Cell Penetration Tolerance",
		meta=(ToolTip="The maximum depth of penetration a cell's hull can penetrate another to make a junction connection.", ClampMin="1", ClampMax="100", UIMin="1", UIMax="100", SliderExponent = 1))
	float AssemblyJunctionMatchingCellHullPenetration = 10.f;

	/** Maximum hull-into-world penetration (world units) allowed when making a junction connection. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Matching", DisplayName="World Penetration Tolerance",
		meta=(ToolTip="The maximum depth of penetration a cell's hull can penetrate world geometry to make a junction connection.", ClampMin="1", ClampMax="100", UIMin="1", UIMax="100", SliderExponent = 1))
	float AssemblyJunctionMatchingWorldPenetration = 2.f;

	/**
	 * When true, two unmatched junctions that already occupy the same opening facing opposite ways are mated as if
	 * the builder had joined them.
	 *
	 * The builders only ever grow a *new* cell off an open junction, so a graph that loops back on itself — or two
	 * organs that grow into each other — can leave two junctions sitting in exactly the same place facing opposite
	 * ways with no link between them. Both are then capped, walling off what is physically an open doorway.
	 * @note Nothing is spawned for these: the two cells are already flush, so the pairing is a plain mating rather
	 *       than a routed connector.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Matching", DisplayName="Connect Coincidences",
		meta=(ToolTip="Should two unmatched junctions that sit in the same place facing opposite directions be linked to each other? These are cells that ended up flush without the builder joining them, and would otherwise both be capped off."))
	bool bJunctionMatchingCoincidences = false;

	/** Default context tags seeded into every assembly operation. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Tagging", DisplayName="Context Tags",
		meta=(ToolTip="The default Context Tags to provide to the Assembly Operation."))
	FGameplayTagContainer AssemblyContextTags;

	/** Default starting tag-counter values seeded into every assembly operation. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Tagging", DisplayName="Starting Counters",
		meta=(ToolTip="The starting counters associated to tags."))
	TMap<FGameplayTag, int32> AssemblyTagCounters;

	/** Angular tolerance (+/- degrees) when matching a candidate's bearing against a target direction. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly", DisplayName="Direction Tolerance",
		meta=(ToolTip="How close should the range of angle be to the target direction (within this many degrees +/-)?", Units="deg"))
	float AssemblyDirectionTolerance = 15.f;


	/** Fallback connector spawned for a paired junction when neither junction nor organ names one; must implement INCellJunctionConnector. */
	UPROPERTY(Config, EditAnywhere, DisplayName="Junction Default Connector", Category="Assembly|Junction Connecting",
		meta=(MustImplement="/Script/NexusWorldAssembly.NCellJunctionConnector", ToolTip="The actor spawned to connect two junctions in close proximity. Must implement NCellJunctionConnector. Setting this Actor does not guarantee its inclusion in a build, you must take steps to include it manually."))
	TSoftClassPtr<AActor> AssemblyDefaultJunctionConnector;

	/** Tuning for the pass that pairs unmatched junctions and routes a connecting spline between them. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Connecting", DisplayName="Junction Connectors",
		meta=(ToolTip="Settings used when matching up unmatched junctions and routing geometry between them."))
	FNWorldAssemblyJunctionConnectorSettings JunctionConnectorSettings;

	/** Target per-frame time budget for spawning cells; the remainder is queued to a new task once exceeded. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Spawning", DisplayName="Cell Time Slice",
	meta=(ToolTip="The target maximum allowed frame time to spawn cells before a new task is queued with the remainder."))
	float AssemblySpawningCellProxiesTimeSlice = 1.f;

	/** Fallback filler spawned when none of a junction's authored fillers are eligible; must implement INCellJunctionFiller. */
	UPROPERTY(Config, EditAnywhere, DisplayName="Junction Default Filler", Category="Assembly|Spawning",
		meta=(MustImplement="/Script/NexusWorldAssembly.NCellJunctionFiller", ToolTip="The actor spawned to fill a junction when none of that junction's authored fillers are eligible. Must implement NCellJunctionFiller. Setting this Actor does not guarantee its inclusion in a build, you must take steps to include it manually."))
	TSoftClassPtr<AActor> AssemblySpawningDefaultJunctionFiller;



	/** When true, junction filling registers with the subsystem to be time-sliced via Junction Time Slice rather than spawning immediately. */
	UPROPERTY(Config, EditAnywhere, DisplayName="Delayed Junction Spawning", Category="Assembly|Spawning",
		meta=(ToolTip="Should junctions that are being filled register themselves with the UWorldAssemblySubsystem to be filled time-sliced via Junction Time Slice?"))
	bool bAssemblySpawningDelayedJunctionSpawning = true;

	/** Per-frame budget (ms) for spawning junction fillers; at least one filler is always spawned per tick. */
	UPROPERTY(Config, EditAnywhere, DisplayName="Junction Time Slice", Category="Assembly|Spawning",
		meta=(ToolTip="How long per frame, in milliseconds, can be spent spawning junction fillers. At least one filler is always spawned per tick."))
	float AssemblySpawningDelayedJunctionSpawningTimeSlice = 0.5f;

	/** Material applied to the dynamic proxy meshes used by ANCellProxy. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Debug", DisplayName="Proxy Material",
		meta=(ToolTip="The material to use with the DynamicMeshes as part of ANCellProxy. Setting this Material does not guarantee its inclusion in a build, you must take steps to include it manually."))
	TSoftObjectPtr<UMaterialInterface> ProxyMaterial;
};