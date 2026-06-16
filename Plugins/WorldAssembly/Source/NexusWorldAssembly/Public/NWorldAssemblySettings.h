// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "Types/NDirection.h"
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
		meta=(ToolTip="The depth per side of a junction that is used for collision detection in some (PCG) scenarios."))
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