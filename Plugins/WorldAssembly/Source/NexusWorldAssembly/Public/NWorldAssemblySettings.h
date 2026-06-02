// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NSettingsUtils.h"
#include "Collections/NGameplayTagCounter.h"
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
	// AlwaysRelevantCellProxies UMETA(DisplayName = "Always Replicate Cell Proxies", ToolTip="Enables replication of NCellProxy actors, making them always relevant, whilst leaving LevelInstances to be replicated based on relevancy."),
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

	N_IMPLEMENT_SETTINGS(UNWorldAssemblySettings, "World Assembly", "Settings related to World Assembly.");

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Voxel Size",
		meta=(ToolTip="What is the size to use when generating voxel data around the space of a NCell. This sizing will also be used to calculate some additional meta data as a unit size. It doesn't need to be too tight; just remember the smaller the size, the greater the performance hit."))
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Socket Size",
		meta=(ToolTip="What is the unit base size for things like the NJunctions and NBones."))
	FVector2D SocketSize = FVector2D(50.f, 50.f);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Socket Depth",
		meta=(ToolTip="The depth per side of a junction that is used for collision detection in some (PCG) scenarios."))
	float SocketDepth = 100.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Player Size",
		meta=(ToolTip="What is the size of the player's collider?"))
	FVector PlayerSize = FVector(72.f, 184.f, 72.f);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network", DisplayName="Mode",
		meta=(ToolTip="How should ANCellLevelInstances be replicated to clients, either based on relevancy (proximity) or treated as always relevant."))
	ENWorldAssemblyNetworkMode NetworkingMode = ENWorldAssemblyNetworkMode::ReplicatedLevelInstances;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network", DisplayName="Initial Range",
	meta=(ToolTip="The range to query for nearby ANCellLevelInstances by ANWorldAssemblyRelay, used to determine if the client is considered loaded. Distance is calculated to the world position point of the ANCellLevelInstance."))
	float NetworkNearbyRange = 20000.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction",
	meta=(ToolTip="The direction used to calculate the automatic bone placement on the volume."))
	ENDirection OrganAutomaticBoneDirection = ENDirection::Backward;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction Offset",
		meta=(ToolTip="Offset value applied to the direction provided by the enumeration."))
	FVector OrganAutomaticBoneDirectionOffset = FVector::ZeroVector;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly", DisplayName="Bad Start Limit",
		meta=(ToolTip="The maximum amount of bad starts that can occur before an assembly is considered a failure."))
	int32 AssemblyBadStartLimit = 1000;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly", DisplayName="Retry Count",
		meta=(ToolTip="The maximum amount of full attempts at assembling a space before it is considered a complete failure."))
	int32 AssemblyGenerationRetryCount = 10000;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Matching", DisplayName="Maximum Cell Hull Penetration",
		meta=(ToolTip="The maximum depth of penetration a cell's convex hull can penetrate another to make a junction connection.", ClampMin="1", ClampMax="100", UIMin="1", UIMax="100", SliderExponent = 1))
	float AssemblyJunctionMatchingCellHullPenetration = 10.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Junction Matching", DisplayName="Maximum World Penetration",
		meta=(ToolTip="The maximum depth of penetration a cell's convex hull can penetrate world geometry to make a junction connection.", ClampMin="1", ClampMax="100", UIMin="1", UIMax="100", SliderExponent = 1))
	float AssemblyJunctionMatchingWorldPenetration = 2.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Tagging", DisplayName="Context Tags",
		meta=(ToolTip="The default Context Tags to provide to the Assembly Operation."))
	FGameplayTagContainer AssemblyContextTags;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Tagging", DisplayName="Starting Counters",
		meta=(ToolTip="The starting counters associated to tags."))
	TMap<FGameplayTag, int32> AssemblyTagCounters;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Assembly|Spawning", DisplayName="Cell Time Slice",
	meta=(ToolTip="The target maxium allowed frame time to spawn cells before a new task is queued with the remainder."))
	float AssemblySpawningCellProxiesTimeSlice = 2.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Debug", DisplayName="Proxy Material",
		meta=(ToolTip="The material to use with the DynamicMeshes as part of ANCellProxy."))
	TSoftObjectPtr<UMaterialInterface> ProxyMaterial;
};