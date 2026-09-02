// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellJunctionComponent.h"

#include "NWorldAssemblyRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NLevelUtils.h"
#include "NWorldAssemblyDebugDraw.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblySubsystem.h"
#include "NWorldAssemblyUtils.h"
#include "TimerManager.h"
#include "Cell/INCellJunctionBeginPlay.h"
#include "Cell/INCellJunctionFiller.h"
#include "Cell/NCellLevelInstance.h"
#include "Collections/NWeightedIntegerArray.h"
#include "Developer/NPrimitiveFont.h"
#include "Engine/AssetManager.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Level.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceInterface.h"
#include "Math/NMersenneTwister.h"
#include "Math/NVectorUtils.h"
#include "Types/NRawMeshUtils.h"

#if WITH_EDITOR
FString UNCellJunctionComponent::GetJunctionName() const
{
	FString ReturnString = GetOwner()->GetActorLabel();

	TArray<USceneComponent*> ParentComponents;
	GetParentComponents(ParentComponents);
	for (const USceneComponent* Parent : ParentComponents)
	{
		ReturnString.Append(" > ");
		ReturnString.Append(Parent->GetName());
	}

	// Get actual name of the component
	ReturnString.Append(" > ");
	ReturnString.Append(GetName());

	return ReturnString;
}
#endif // WITH_EDITOR

FRotator UNCellJunctionComponent::GetOffsetRotator() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UNCellJunctionComponent::GetOffsetLocation() const
{
	if (ALevelInstance* Instance = LevelInstance.Get())
	{
		return Instance->GetActorLocation();
	}
	return FVector::ZeroVector;
}

float UNCellJunctionComponent::GetFillDepth() const
{
	const ENCellJunctionFillDepthMode Mode = Details.FillDepthMode;
	if (Mode == ENCellJunctionFillDepthMode::DefaultBackward ||
		Mode == ENCellJunctionFillDepthMode::DefaultCentered ||
		Mode == ENCellJunctionFillDepthMode::DefaultForward)
	{
		return UNWorldAssemblySettings::Get()->SocketDepth;
	}
	return Details.OverrideFillDepth;
}

float UNCellJunctionComponent::GetFillDepthAnchor() const
{
	return GetFillDepth() * FNCellJunctionDetails::GetFillDepthAnchorScale(Details.FillDepthMode);
}

void UNCellJunctionComponent::BeginPlay()
{
	Super::BeginPlay();

	// A cell's assembly data is replicated, and on a client it can arrive after its junctions have registered and
	// begun play. Until it does, GetCellLinkDetails has nothing to answer from and hands back a default — which
	// reads as unconnected, and is indistinguishable from a junction that genuinely resolved that way. Acting on it
	// would hide a connected junction's additional actors and spawn a filler into an opening another cell is about
	// to meet, so wait: ANCellLevelInstance::UpdateFromAssemblyData resolves this junction once the data lands.
	if (const ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(LevelInstance.Get());
		CellLevelInstance != nullptr && !CellLevelInstance->HasAssemblyData())
	{
		bAwaitingAssemblyData = true;
		return;
	}

	ResolveConnectionState();
}

void UNCellJunctionComponent::ResolveConnectionState()
{
	if (bConnectionStateResolved) return;
	bConnectionStateResolved = true;
	bAwaitingAssemblyData = false;

	// Drive both additional-actor lists to match how this junction resolved. Unconditional and first, because every
	// path out of this function below is conditional — a connector endpoint returns early, a Required or AllowEmpty
	// junction never calls Fill(), and Fill() itself returns before doing anything when the junction has no eligible
	// filler and the project sets no default. A junction routed through any of those would otherwise never have its
	// additional actors touched at all, and would show them at whatever visibility they were authored with.
	ProcessAdditionalActors(LinkDetails.bConnected);

	// Send out calls to anything linked to it
	if (OnBeginPlayTargets.Num() > 0)
	{
		for (int i = 0; i < OnBeginPlayTargets.Num(); ++i)
		{
			if ( OnBeginPlayTargets[i] == nullptr ) continue;


			AActor* Actor = OnBeginPlayTargets[i].Get();
			if (Actor == nullptr) continue;


			if (Actor->Implements<UNCellJunctionBeginPlay>())
			{
				INCellJunctionBeginPlay::Execute_OnJunctionBeginPlay(Actor, this, LinkDetails);
			}
			else
			{
				UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to invoke OnJunctionBeginPlay on %s as it does not implement the INCellJunctionBeginPlay."), *Actor->GetName())
			}
		}
	}

	// A connector-paired junction is connected but nothing occupies it yet: the actor that bridges it needs both
	// cells live, so report in and let the subsystem build it once the far end arrives.
	if (LinkDetails.bConnector)
	{
		UNWorldAssemblySubsystem::Get(GetWorld())->RegisterJunctionConnectorEndpoint(this);
		return;
	}

	if (LinkDetails.bConnected)
		return;

	switch (Details.Requirements)
	{
	case ENCellJunctionRequirements::Required:
		// Should not be here!
		break;
	case ENCellJunctionRequirements::AllowBlocking:
		// Fill immediately or register for deferred
		if (!UNWorldAssemblySettings::Get()->bAssemblySpawningDelayedJunctionSpawning || bSpawnFillerImmediately)
		{
			Fill();
		}
		else
		{
			UNWorldAssemblySubsystem::Get(GetWorld())->RegisterCellJunctionToFill(this);
		}
		break;
	case ENCellJunctionRequirements::AllowEmpty:
		break;
	}


}

void UNCellJunctionComponent::OnAssemblyDataUpdated()
{
	ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(LevelInstance.Get());
	if (CellLevelInstance == nullptr) return;

	LinkDetails = CellLevelInstance->GetCellLinkDetails(Details.InstanceIdentifier);

	// Only a junction that deferred in BeginPlay is waiting on this. One that has not begun play yet resolves for
	// itself off the data now present, and one that already resolved is deliberately left standing rather than
	// unwound — it read real data, so a later update describes the same outcome.
	if (bAwaitingAssemblyData && HasBegunPlay())
	{
		ResolveConnectionState();
	}
}

void UNCellJunctionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Cell was removed, and were still doing stuff so the filler should go too
	if (EndPlayReason == EEndPlayReason::Type::Destroyed || EndPlayReason == EEndPlayReason::Type::RemovedFromWorld )
	{
		UWorld* World = GetWorld();

		// A connector bridges two cells, so this one leaving strands it. Withdrawing tears it down and leaves the
		// pairing intact, ready to rebuild if the cell streams back in.
		if (LinkDetails.bConnector && World != nullptr)
		{
			UNWorldAssemblySubsystem::Get(World)->UnregisterJunctionConnectorEndpoint(this);
		}

		if (World != nullptr && FillerActor.Get() != nullptr)
		{
			AActor* Actor = FillerActor.Get();

			ALevelInstance* LocalLevelInstance = LevelInstance.Get();
			if (LocalLevelInstance == nullptr)
			{
				UNWorldAssemblySubsystem::Get(World)->UnregisterOperationActor(Actor);
				Actor->Destroy();
				return;
			}
			ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(LocalLevelInstance);
			if (CellLevelInstance == nullptr)
			{
				UNWorldAssemblySubsystem::Get(World)->UnregisterOperationActor(Actor);
				Actor->Destroy();
				return;
			}

			// If we still have it, be direct
			UNWorldAssemblySubsystem::Get(World)->UnregisterOperationActorByTicket(Actor,
				CellLevelInstance->GetAssemblyData().OperationTicket);
			Actor->Destroy();
		}
	}

}


void UNCellJunctionComponent::DrawDebugPDI(FPrimitiveDrawInterface* PDI, const FLinearColor& ValidColor, const FLinearColor& InvalidColor,
                                           const bool bShowDepth, const bool bIsConnected, const bool bDrawBox, const bool bDrawCornerLines,
                                           const bool bDrawFillDepth, const UNWorldAssemblySettings* Settings) const
{
	FLinearColor GizmoColor = ValidColor; // Default color
	const FVector ComponentLocation = GetComponentLocation();
	const FRotator ComponentRotation = GetComponentRotation();
	const ULevel* Level = GetComponentLevel();

	FNDrawSocketSettings SocketSettings;
	SocketSettings.SocketSize = Settings->SocketSize;
	SocketSettings.SocketType = Details.Type;

	SocketSettings.bDrawFillDepth = bDrawFillDepth;
	SocketSettings.FillDepthMode = Details.FillDepthMode;
	SocketSettings.FillDepth = GetFillDepth();

	SocketSettings.UnitSize = Details.SocketSize;
	SocketSettings.bIsConnected = bIsConnected;

	SocketSettings.bDrawBox = bDrawBox;
	SocketSettings.bDrawCornerLines = bDrawCornerLines;

	if (bShowDepth && Level != nullptr)
	{
		// Check Cell Root
		const UNCellRootComponent* CellRoot = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(Level);
		if (CellRoot == nullptr)
		{
			FNWorldAssemblyDebugDraw::DrawSocket(PDI, ComponentLocation, ComponentRotation, SocketSettings);
			return;
		}

		const FNRawMesh& Hull = CellRoot->Details.Hull;

		// Memoized: the ed mode redraws every registered junction each frame, but the sweep below only changes when
		// the hull, this junction's transform, or the socket sizing does. A hit also skips rebuilding the corner
		// points, which is two heap allocations per junction per frame on its own.
		float MaximumDepth = 0.f;
		double LowestZ = 0.0;
		GetCachedHullPenetration(Hull, Settings->SocketSize, MaximumDepth, LowestZ);

		if (MaximumDepth > Settings->AssemblyJunctionMatchingCellHullPenetration)
		{
			GizmoColor = InvalidColor;
		}

		// Draw the depth text
		if (MaximumDepth != 0)
		{
			// Always draw the readout upright in world space, directly beneath the junction. Using only the junction's
			// yaw (zero pitch/roll) keeps the glyphs world-upright no matter how the junction is oriented, so the text
			// never ends up upside down. Anchoring at the socket's lowest world-Z corner keeps it clear of the socket.
			const FVector TextPosition(ComponentLocation.X, ComponentLocation.Y, LowestZ - 4.0f);
			const FRotator TextRotation(0.0, ComponentRotation.Yaw, 0.0);

			FNPrimitiveFont::DrawPDI(PDI, FString::Printf(TEXT("%.1f"),MaximumDepth),
				TextPosition, TextRotation, GizmoColor,0.15f, 1.f, 1.f,
				false, true, SDPG_Foreground);
		}
	}

	SocketSettings.Color = GizmoColor;
	FNWorldAssemblyDebugDraw::DrawSocket(PDI, ComponentLocation, ComponentRotation, SocketSettings);
}

void UNCellJunctionComponent::OnRegister()
{
#if WITH_EDITOR
	// Ensure that undo system works
	SetFlags(RF_Transactional);
#endif

	// Is this part of a level instance?
	ILevelInstanceInterface* Interface = FNLevelUtils::GetActorComponentLevelInstance(this);
	if (Interface != nullptr)
	{
		LevelInstance = Cast<ALevelInstance>(Interface);
	}

	const ULevel* Level = GetComponentLevel();
	ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromLevel(Level);

#if WITH_EDITOR
	// Author-time validation: catch junctions placed into a level that has no cell root. Skipped for
	// junctions streamed in as part of a cell level instance, where AddToWorld can chunk component
	// registration across ticks and race this check.
	if (!LevelInstance.IsValid())
	{
		// Author-time validation: a junction forces itself to Static mobility, so a non-Static attach parent
		// produces a Static-under-Movable error at cook time. Warn early; the authoritative gate is
		// UNWorldAssemblyEditorValidator::ValidateWorldAsset, which also catches a parent whose mobility is
		// changed after this junction has registered.
		if (const USceneComponent* ParentComponent = GetAttachParent();
			ParentComponent != nullptr && ParentComponent->Mobility != EComponentMobility::Static)
		{
			UE_LOG(LogNexusWorldAssembly, Warning,
				TEXT("Junction '%s' is attached to non-Static parent '%s'; this errors during cook. Set the owning actor/component Mobility to Static."),
				*GetJunctionName(), *ParentComponent->GetName());
		}

		TWeakObjectPtr WeakJunctionComponent(this);
		Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakJunctionComponent]()
		{
			if (!WeakJunctionComponent.IsValid()) return;

			const ULevel* Level = WeakJunctionComponent.Get()->GetComponentLevel();
			const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(Level);
			if (RootComponent == nullptr)
			{
				UE_LOG(LogNexusWorldAssembly, Error, TEXT("No UNCellRootComponent found for ULevel(%s); removing added UNCellJunctionComponent next update. [registered roots=%d]"),
					*Level->GetPathName(),
					FNWorldAssemblyRegistry::GetCellRootComponents().Num());
				Level->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakJunctionComponent]()
				{
					if (!WeakJunctionComponent.IsValid()) return;
					WeakJunctionComponent.Get()->DestroyComponent();
				}));
			}
		}));
	}

	if (Actor != nullptr && !Actor->WasSpawnedFromProxy())
	{
		// Whilst in the editor we want to make sure that we uniquely identify our junctions
		if (Details.InstanceIdentifier == -1)
		{
			Actor->Modify();
			Details.InstanceIdentifier = Actor->GetCellJunctionNextIdentifier();
			Actor->SetActorDirty();
		}
		if (!Actor->CellJunctions.Contains(Details.InstanceIdentifier))
		{
			// CellJunctions is Transient, so this map is empty after every load and gets rebuilt here on
			// registration. Rebuilding transient runtime state is not an author-time change, so we must not
			// mark the actor dirty - the genuine "new junction" case is handled by the InstanceIdentifier
			// assignment above.
			Actor->CellJunctions.Add(Details.InstanceIdentifier, this);
		}
	}


#endif // WITH_EDITOR

	// Update details based on generation.
	if (ALevelInstance* Instance = LevelInstance.Get();
		Actor != nullptr && Actor->WasSpawnedFromProxy() && Instance != nullptr && Instance->IsA<ANCellLevelInstance>())
	{
		ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(Instance);
		const FNCellJunctionDetails* UpdatedDetails = CellLevelInstance->JunctionData.Find(Details.InstanceIdentifier);
		if (UpdatedDetails != nullptr)
		{
			// Copy details in-place
			Details = *UpdatedDetails;

			// Update the rotation so the thing draws nicely; this feels like a bug. The ALevelInstance is supposed
			// to rotate the UWorlds content when it gets placed and loaded. The documentation around the methods seem to
			// infer however that some of this might be editor time only. Not exactly sure what is happening here leading
			// to the world rotations needing to be updated manually to match the data-only version that we use during
			// generating our FNAssemblyGraph.

			// There is some explicit logic around WorldPartition moving everything to a flat-structure when you add ALevelInstances,
			// this logic doesn't appear to be as exercised in the old-school non-world partition way of building levels.
			// It might be related --- I don't know --- hopefully this information might be useful in the future.
			SetWorldRotation(Details.WorldRotation, false, nullptr, ETeleportType::ResetPhysics);
		}
		// Skipped when the cell's replicated assembly data has not landed yet: GetCellLinkDetails would only warn
		// and hand back a default. BeginPlay detects the same condition and defers this junction's resolve to
		// OnAssemblyDataUpdated, which is where LinkDetails is filled in for that case.
		if (CellLevelInstance->HasAssemblyData())
		{
			LinkDetails = CellLevelInstance->GetCellLinkDetails(Details.InstanceIdentifier);

#if WITH_EDITOR
			// AUTHOR-TIME PREVIEW — BeginPlay never runs in an editor world, so this is the only point at which a
			// generated junction knows how it resolved. Visibility of the additional actors is all that is driven
			// here: filling, connector registration and the OnBeginPlayTargets callbacks all route through the
			// game-only UNWorldAssemblySubsystem, which does not exist in an editor world.
			//
			// Safe this early even though the actors named in those lists may not have registered their components
			// yet — FPrimitiveSceneProxy reads IsHiddenEd() when the proxy is built, so one registering after this
			// still comes up hidden. The enclosing WasSpawnedFromProxy() test is what keeps this off a cell opened
			// for authoring, where hiding the author's own actors would be wrong.
			if (const UWorld* World = GetWorld(); World != nullptr && !World->IsGameWorld())
			{
				ProcessAdditionalActors(LinkDetails.bConnected);
			}
#endif // WITH_EDITOR
		}
	}

	FNWorldAssemblyRegistry::RegisterCellJunctionComponent(this);

	N_WORLD_ICON_ON_REGISTER("/NexusWorldAssembly/EditorResources/S_NCellJunctionComponent", 0.35f)

	Super::OnRegister();
}

void UNCellJunctionComponent::OnUnregister()
{
	ANCellActor* Actor = FNWorldAssemblyUtils::GetCellActorFromLevel(GetComponentLevel());
	if (Actor != nullptr && Actor->CellJunctions.Contains(Details.InstanceIdentifier))
	{
		Actor->CellJunctions.Remove(Details.InstanceIdentifier);
	}
	FNWorldAssemblyRegistry::UnregisterCellJunctionComponent(this);
	Super::OnUnregister();
}

void UNCellJunctionComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	N_WORLD_ICON_CLEANUP(bDestroyingHierarchy)
}


#if WITH_EDITOR
void UNCellJunctionComponent::OnTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	const UNCellRootComponent* RootComponent = FNWorldAssemblyRegistry::GetCellRootComponentFromLevel(GetComponentLevel());
	const ANCellActor* CellActor = RootComponent != nullptr ? RootComponent->GetNCellActor() : nullptr;
	if (CellActor != nullptr && !CellActor->WasSpawnedFromProxy())
	{
		bool bHasMadeChanges = false;

		// LOCATION
		const FVector ComponentLocation = GetComponentLocation();
		if (ComponentLocation != Details.WorldLocation)
		{
			// We do not try to store anything about the voxel/final location here as the bounds of the data can change
			Modify();
			Details.WorldLocation = ComponentLocation;
			bHasMadeChanges = true;
		}

		// ROTATOR
		const FRotator ComponentRotation = GetComponentRotation();
		if (ComponentRotation != Details.WorldRotation)
		{
			Modify();
			Details.WorldRotation = ComponentRotation;
			bHasMadeChanges = true;
		}

		// Have we made changes, let the people know!
		if (bHasMadeChanges)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			MarkPackageDirty();
		}
	}
}

void UNCellJunctionComponent::PostEditImport()
{
	// Forces the instance identifier
	Details.InstanceIdentifier = -1;
}

#endif // WITH_EDITOR

float UNCellJunctionComponent::ComputeMaximumHullPenetration(const FNRawMesh& Hull, const TArray<FVector>& CornerPoints)
{
	float MaximumDepth = 0.f;
	for (const FVector& Corner : CornerPoints)
	{
		// Corners outside the hull come back as the -1 sentinel, which never beats the 0 seed — so a junction
		// entirely clear of the hull reports 0 rather than a negative depth.
		const float Depth = FNRawMeshUtils::GetIntersectDepth(Hull, FVector::Zero(), FRotator::ZeroRotator, Corner);
		if (Depth > MaximumDepth)
		{
			MaximumDepth = Depth;
		}
	}
	return MaximumDepth;
}

void UNCellJunctionComponent::GetCachedHullPenetration(const FNRawMesh& Hull, const FVector2D& SettingsSocketSize,
	float& OutMaximumDepth, double& OutLowestCornerZ) const
{
	const FVector ComponentLocation = GetComponentLocation();
	const FRotator ComponentRotation = GetComponentRotation();

	// Identify the hull by its contents rather than by a version counter on the cell root: the hull is mutated from
	// several places and can be swapped wholesale by undo/redo, and a CRC over the vertex buffer stays correct
	// without each of those paths having to remember to invalidate us. It also catches the case a cheaper
	// vertex-count/bounds check would miss — a vertex dragged inward while others still define the AABB.
	const uint32 HullVertexCrc = Hull.Vertices.IsEmpty()
		? 0u
		: FCrc::MemCrc32(Hull.Vertices.GetData(), Hull.Vertices.Num() * sizeof(FVector));
	const int32 HullLoopCount = Hull.Loops.Num();

	if (CachedHullPenetration.bValid
		&& CachedHullPenetration.KeyHullVertexCrc == HullVertexCrc
		&& CachedHullPenetration.KeyHullLoopCount == HullLoopCount
		&& CachedHullPenetration.KeyLocation == ComponentLocation
		&& CachedHullPenetration.KeyRotation == ComponentRotation
		&& CachedHullPenetration.KeyUnitSocketSize == Details.SocketSize
		&& CachedHullPenetration.KeySettingSocketSize == SettingsSocketSize)
	{
		OutMaximumDepth = CachedHullPenetration.MaximumDepth;
		OutLowestCornerZ = CachedHullPenetration.LowestCornerZ;
		return;
	}

	const TArray<FVector> CornerPoints = GetWorldCornerPoints(SettingsSocketSize);
	const float MaximumDepth = ComputeMaximumHullPenetration(Hull, CornerPoints);

	// Seeded from the component location (not from the first corner) so the readout never floats above the
	// junction itself when every corner sits higher — preserving the anchor the draw path has always used.
	double LowestCornerZ = ComponentLocation.Z;
	for (const FVector& Corner : CornerPoints)
	{
		LowestCornerZ = FMath::Min(LowestCornerZ, Corner.Z);
	}

	CachedHullPenetration.KeyHullVertexCrc = HullVertexCrc;
	CachedHullPenetration.KeyHullLoopCount = HullLoopCount;
	CachedHullPenetration.KeyLocation = ComponentLocation;
	CachedHullPenetration.KeyRotation = ComponentRotation;
	CachedHullPenetration.KeyUnitSocketSize = Details.SocketSize;
	CachedHullPenetration.KeySettingSocketSize = SettingsSocketSize;
	CachedHullPenetration.MaximumDepth = MaximumDepth;
	CachedHullPenetration.LowestCornerZ = LowestCornerZ;
	CachedHullPenetration.bValid = true;

	OutMaximumDepth = MaximumDepth;
	OutLowestCornerZ = LowestCornerZ;
}

TArray<FVector> UNCellJunctionComponent::GetWorldCornerPoints(const FVector2D& SocketSize) const
{
	// Built from the live component transform rather than Details' baked one, which is why this cannot just take
	// the FNCellJunctionDetails overload.
	// TODO: Should this maybe be cached at spawning at runtime?
	return FNWorldAssemblyUtils::GetJunctionWorldCornerPoints(GetComponentLocation(), GetComponentRotation(),
		Details.SocketSize, SocketSize);
}


void UNCellJunctionComponent::Fill()
{
	// ResolveConnectionState has already shown this unconnected junction's filled actors, which is the point of
	// disabling the fill: the opening is dressed by hand instead of by a spawned filler.
	if (bDisableFill) return;

	ALevelInstance* LocalLevelInstance = LevelInstance.Get();
	if (LocalLevelInstance == nullptr) return;
	ANCellLevelInstance* CellLevelInstance = Cast<ANCellLevelInstance>(LocalLevelInstance);
	if (CellLevelInstance == nullptr) return;



	// AUTHORED FILLERS — pick an eligible, weighted entry and spawn it directly. These classes are hard-referenced by
	// the junction, so they are always resident and can spawn synchronously.
	if (Fillers.Num() > 0)
	{
		FNCellAssemblyData& AssemblyData = CellLevelInstance->GetAssemblyData();
		const FQuat FillerRotation = GetComponentRotation().Quaternion();

		// Create our spawning parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.OverrideLevel = GetComponentLevel();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;

		// Create a deterministic random based on the overall seed, the node were in, and the junction itself.
		FNMersenneTwister RandomGenerator(AssemblyData.Seed ^ AssemblyData.NodeIdentifier ^ LinkDetails.JunctionInstanceIdentifier);

		FNWeightedIntegerArray WeightedAvailableIndices = GetJunctionFillEntries(AssemblyData);

		// TwistedValue returns INDEX_NONE when every filler was gated out by its constraints; falling through to the
		// default-filler fallback below rather than indexing Fillers with -1.
		const int32 FillerIndex = WeightedAvailableIndices.TwistedValue(RandomGenerator);
		if (FillerIndex != INDEX_NONE)
		{
			// Location offset is authored in the junction's frame, so rotate it by the junction's orientation before
			// nudging; the rotation offset then spins the filler in place at that spot.
			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Fillers[FillerIndex].Actor,
				GetComponentLocation() + FillerRotation.RotateVector(Fillers[FillerIndex].Offset.GetLocation()),
				(FillerRotation * FQuat(Fillers[FillerIndex].Offset.Rotator())).Rotator(),
				SpawnParams);

			if (SpawnedActor != nullptr)
			{
				SpawnedActor->SetActorScale3D(SpawnedActor->GetActorScale3D() * Fillers[FillerIndex].Offset.GetScale3D());

				// The entry's opt-out is the one input ResolveConnectionState could not have had, so it is applied
				// here — and only once this filler is the one that actually spawned. A selected filler that fails to
				// spawn falls through to the default-filler path below with the junction's state left as it was,
				// rather than having been narrowed on behalf of a filler that never arrived.
				if (Fillers[FillerIndex].bSkipAdditionalActors)
				{
					ProcessAdditionalActors(false, true);
				}

				FinalizeFillerSpawn(SpawnedActor, CellLevelInstance);
				return;
			}
		}
	}

	// DEFAULT FILLER FALLBACK — no authored filler qualified, so fall back to the project-wide default. It is a soft
	// reference (so an early config load can't null it out), which means it may not be resident yet.
	const TSoftClassPtr<AActor>& DefaultFiller = UNWorldAssemblySettings::Get()->AssemblySpawningDefaultJunctionFiller;
	if (DefaultFiller.IsNull())
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to fill junction as no fillers were available, and no default filler was set."));
		return;
	}

	// Already resident: spawn immediately to keep the synchronous fast path.
	if (UClass* LoadedFiller = DefaultFiller.Get())
	{
		SpawnDefaultFiller(LoadedFiller, CellLevelInstance);
		return;
	}

	// Not resident: stream it in, then spawn once it lands. Weak captures bail if the junction or its cell were
	// destroyed (e.g. the cell streamed back out) while the load was in flight.
	TWeakObjectPtr<UNCellJunctionComponent> WeakThis(this);
	TWeakObjectPtr<ANCellLevelInstance> WeakCell(CellLevelInstance);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(DefaultFiller.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda([WeakThis, WeakCell, DefaultFiller]()
		{
			UNCellJunctionComponent* StrongThis = WeakThis.Get();
			ANCellLevelInstance* StrongCell = WeakCell.Get();
			if (StrongThis == nullptr || StrongCell == nullptr) return;

			UClass* LoadedFiller = DefaultFiller.Get();
			if (LoadedFiller == nullptr)
			{
				UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Default junction filler '%s' failed to load; junction left unfilled."),
					*DefaultFiller.ToString());
				return;
			}
			StrongThis->SpawnDefaultFiller(LoadedFiller, StrongCell);
		}));
}

void UNCellJunctionComponent::SpawnDefaultFiller(UClass* FillerClass, ANCellLevelInstance* CellLevelInstance)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.OverrideLevel = GetComponentLevel();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	AActor* SpawnedActor = World->SpawnActor<AActor>(FillerClass, GetComponentLocation(), GetComponentRotation(), SpawnParams);
	if (SpawnedActor != nullptr)
	{
		FinalizeFillerSpawn(SpawnedActor, CellLevelInstance);
	}
}

void UNCellJunctionComponent::FinalizeFillerSpawn(AActor* SpawnedActor, ANCellLevelInstance* CellLevelInstance)
{
	if (SpawnedActor->Implements<UNCellJunctionFiller>())
	{
		INCellJunctionFiller::Execute_OnInitializedFromJunction(SpawnedActor, CellLevelInstance, this, LinkDetails.JunctionInstanceIdentifier);
	}
	UNWorldAssemblySubsystem* System = UNWorldAssemblySubsystem::Get(GetWorld());
	System->RegisterOperationActor(SpawnedActor, CellLevelInstance->GetAssemblyData().OperationTicket);
	FillerActor = SpawnedActor;
}

namespace
{
	/**
	 * Show or hide one of a junction's additional actors.
	 *
	 * The game and editor visibility flags are separate: bHidden gates rendering only in a game world and
	 * bHiddenEdTemporary only in an editor viewport, so a junction resolved at author time has to set the editor
	 * one to show anything at all. Neither is saved, and SetIsTemporarilyHiddenInEditor neither transacts nor
	 * dirties, which suits actors that exist only inside a generated cell's level instance.
	 * @param Actor The additional actor to drive.
	 * @param bVisible Whether this junction's outcome calls for the actor to be shown.
	 * @note Author time drives visibility only. The world-collision gather skips primitives whose collision is
	 *       switched off, so toggling collision here would make a level's baked pool depend on which junctions
	 *       happened to connect, and invalidate the cache on every regeneration.
	 */
	void ApplyAdditionalActorVisibility(AActor* Actor, const bool bVisible)
	{
#if WITH_EDITOR
		if (const UWorld* World = Actor->GetWorld(); World != nullptr && !World->IsGameWorld())
		{
			Actor->SetIsTemporarilyHiddenInEditor(!bVisible);
			return;
		}
#endif // WITH_EDITOR

		Actor->SetActorEnableCollision(bVisible);
		Actor->SetActorHiddenInGame(!bVisible);
	}
}

void UNCellJunctionComponent::ProcessAdditionalActors(const bool bConnected, const bool bSkipAdditionalFilledActors)
{
	// The skip flag only ever narrows: a filler that occupies the opening on its own hides the filled actors that
	// would otherwise have been shown, and can never show ones a connected junction hides.
	const bool bShowFilledActors = !bConnected && !bSkipAdditionalFilledActors;
	for (int i = 0; i < AdditionalFilledActors.Num(); i++)
	{
		AActor* FilledActor = AdditionalFilledActors[i];
		if (FilledActor == nullptr) continue;

		ApplyAdditionalActorVisibility(FilledActor, bShowFilledActors);
	}

	// Second, so an actor named in both lists ends up in the state this one asks for.
	for (int i = 0; i < AdditionalConnectedActors.Num(); i++)
	{
		AActor* ConnectedActor = AdditionalConnectedActors[i];
		if (ConnectedActor == nullptr) continue;

		ApplyAdditionalActorVisibility(ConnectedActor, bConnected);
	}
}

FNWeightedIntegerArray UNCellJunctionComponent::GetJunctionFillEntries(const FNCellAssemblyData& AssemblyData) const
{
	FNWeightedIntegerArray Indices;

	// Rebuild the cell's final tag-counter state once up front so each filler's constraints can be evaluated
	// against it without reconstructing the map per entry. Mirrors FNVirtualOrganContext::FilterCellInputData,
	// gating each candidate against the assembly state instead of the in-flight graph state.
	const FNGameplayTagCounter TagCounter(AssemblyData.TagCounter);

	for (int32 i = 0; i < Fillers.Num(); i++)
	{
		const FNCellJunctionFillerEntry& Filler = Fillers[i];

		// REQUIRED CONTEXT TAGS — the cell's resolved context must satisfy every tag the filler requires.
		if (!Filler.RequiredContextTags.IsEmpty() && !AssemblyData.ContextTags.HasAllExact(Filler.RequiredContextTags))
		{
			continue;
		}

		// TAG COUNTER CONSTRAINTS — every constraint must pass against the cell's final counter state. A tag the
		// counter does not track compares as a count of zero (see FNGameplayTagCounterConstraint::DoesPassComparison).
		bool bGatedByTagCounter = false;
		for (const FNGameplayTagCounterConstraint& Constraint : Filler.TagCounterConstraints)
		{
			if (!Constraint.DoesPassComparison(TagCounter))
			{
				bGatedByTagCounter = true;
				break;
			}
		}
		if (bGatedByTagCounter)
		{
			continue;
		}

		// Survivor: add it weighted so selection honors the authored relative likelihood.
		Indices.Add(i, Filler.Weighting);
	}

	return Indices;
}

