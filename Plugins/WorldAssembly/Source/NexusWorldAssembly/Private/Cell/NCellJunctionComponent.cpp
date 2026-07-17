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
#include "Engine/Level.h"
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

	// Early enable/disable based on connection
	if (LinkDetails.bConnected)
	{
		ProcessAdditionalActors(true, bDisableFill);
	}

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

void UNCellJunctionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Cell was removed, and were still doing stuff so the filler should go too
	if (EndPlayReason == EEndPlayReason::Type::Destroyed || EndPlayReason == EEndPlayReason::Type::RemovedFromWorld )
	{
		UWorld* World = GetWorld();
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
                                           const UNWorldAssemblySettings* Settings) const
{
	FLinearColor GizmoColor = ValidColor; // Default color
	const FVector ComponentLocation = GetComponentLocation();
	const FRotator ComponentRotation = GetComponentRotation();
	const ULevel* Level = GetComponentLevel();

	FNDrawSocketSettings SocketSettings;
	SocketSettings.SocketSize = Settings->SocketSize;
	SocketSettings.SocketType = Details.Type;

	// Do we want to make this an option?
	SocketSettings.bDrawFillDepth = true;
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

		TArray<FVector> CornerPoints = GetWorldCornerPoints(Settings->SocketSize);
		float MaximumDepth = 0;
		for (int i = 0; i < CornerPoints.Num(); i++)
		{
			const float Depth = FNRawMeshUtils::GetIntersectDepth(Hull,FVector::Zero(), FRotator::ZeroRotator,  CornerPoints[i]);
			if (Depth > MaximumDepth)
			{
				MaximumDepth = Depth;
			}
		}

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

			double LowestZ = ComponentLocation.Z;
			for (const FVector& Corner : CornerPoints)
			{
				LowestZ = FMath::Min(LowestZ, Corner.Z);
			}

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
		LinkDetails = CellLevelInstance->GetCellLinkDetails(Details.InstanceIdentifier);
	}

	FNWorldAssemblyRegistry::RegisterCellJunctionComponent(this);
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

TArray<FVector> UNCellJunctionComponent::GetWorldCornerPoints(const FVector2D& SocketSize) const
{
	const FQuat DisplayQuat = FQuat(GetComponentRotation()) * FQuat(FRotator(0.0f, 90.0f, 0.0f));
	const FRotator DisplayRotation = DisplayQuat.Rotator();

	// TODO: Should this maybe be cached at spawning at runtime?
	const FVector2D Size = FNWorldAssemblyUtils::GetWorldSize2D(Details.SocketSize, SocketSize);

	const TArray<FVector> UnrotatedCornerPoints = FNWorldAssemblyUtils::GetCenteredWorldCornerPoints2D(Size.X,Size.Y, ENAxis::Z);
	const TArray<FVector> RotatedCornerPoints = FNVectorUtils::RotateAndOffsetPoints(UnrotatedCornerPoints, DisplayRotation, GetComponentLocation());
	return RotatedCornerPoints;
}


void UNCellJunctionComponent::Fill()
{
	if (bDisableFill)
	{
		ProcessAdditionalActors(false);
		return;
	}

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
			ProcessAdditionalActors(false, Fillers[FillerIndex].bSkipAdditionalActors);

			// Location offset is authored in the junction's frame, so rotate it by the junction's orientation before
			// nudging; the rotation offset then spins the filler in place at that spot.
			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Fillers[FillerIndex].Actor,
				GetComponentLocation() + FillerRotation.RotateVector(Fillers[FillerIndex].Offset.GetLocation()),
				(FillerRotation * FQuat(Fillers[FillerIndex].Offset.Rotator())).Rotator(),
				SpawnParams);

			if (SpawnedActor != nullptr)
			{
				SpawnedActor->SetActorScale3D(SpawnedActor->GetActorScale3D() * Fillers[FillerIndex].Offset.GetScale3D());
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
		ProcessAdditionalActors(false);
		SpawnDefaultFiller(LoadedFiller, CellLevelInstance);
		return;
	}

	// Not resident: stream it in, then spawn once it lands. Weak captures bail if the junction or its cell were
	// destroyed (e.g. the cell streamed back out) while the load was in flight.
	ProcessAdditionalActors(false);
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

void UNCellJunctionComponent::ProcessAdditionalActors(const bool bConnected, const bool bSkipAdditionalFilledActors)
{
	if (!bSkipAdditionalFilledActors)
	{
		const bool bShowFilledActors = !bConnected;
		for (int i = 0; i < AdditionalFilledActors.Num(); i++)
		{
			AActor* FilledActor = AdditionalFilledActors[i];
			if (FilledActor == nullptr) continue;

			FilledActor->SetActorEnableCollision(bShowFilledActors);
			FilledActor->SetActorHiddenInGame(!bShowFilledActors);
		}
	}

	const bool bShowConnectedActors = bConnected;
	for (int i = 0; i < AdditionalConnectedActors.Num(); i++)
	{
		AActor* ConnectedActor = AdditionalConnectedActors[i];
		if (ConnectedActor == nullptr) continue;

		ConnectedActor->SetActorEnableCollision(bShowConnectedActors);
		ConnectedActor->SetActorHiddenInGame(!bShowConnectedActors);
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

