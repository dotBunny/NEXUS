// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Contexts/NAssemblyOperationContext.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "StaticMeshResources.h"
#include "Cell/NTissue.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/NBoundsUtils.h"
#include "Organ/NOrganComponent.h"

FNAssemblyOperationContext::FNAssemblyOperationContext(const int32 NewOperationTicket)
{
	OperationTicket = NewOperationTicket;
}

void FNAssemblyOperationContext::ResetContext()
{
	OrganData.Empty();
	BoneData.Empty();
	Bounds.Empty();
	ComponentBoneMap.Empty();
	GenerationOrder.Empty();
	InputComponents.Empty();
	TargetWorld = nullptr;
	
	bIsLocked = false;
}

void FNAssemblyOperationContext::SetOperationSettings(const FNAssemblyOperationSettings& InSettings)
{
	
	if (IsLocked())
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Unable to copy settings on FNOrganGenerationContext when it has already been locked."));
	}
	else
	{
		OperationSettings = InSettings; 
	}
}

bool FNAssemblyOperationContext::AddOrganComponent(UNOrganComponent* Component)
{
	if (IsLocked())
	{
		UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Attempted to add additional context from a UNOrganComponent when the FNOrganGenerationContext has already been locked."));
		return false;
	}
	
	// We've already added this component
	if (OrganData.Contains(Component))
	{
		return true;
	}
	
	InputComponents.Add(Component);
	OrganData.Add(Component, FNWorldOrganData());
	FNWorldOrganData* WorkingOrganData = OrganData.Find(Component);
	WorkingOrganData->SourceComponent = Component;
	WorkingOrganData->Origin = Component->GetOwner()->GetActorLocation();

	// We're going to capture all the other components in the level
	TArray<UNOrganComponent*> LevelComponents = FNWorldAssemblyUtils::GetOrganComponentsFromLevel(Component->GetComponentLevel());

	// We've got ourselves a volume and it has dimensions
	if (Component->IsVolumeBased() && !Component->bUnbounded)
	{
		// Assign Bounds
		const AVolume* ComponentVolume = Cast<AVolume>(Component->GetOwner());
		const FBoxSphereBounds ComponentBounds = ComponentVolume->GetBounds();
		WorkingOrganData->Bounds = ComponentBounds;

		// Stage relationships locally; the recursive AddOrganComponent calls below mutate
		// OrganContext and can rehash its storage, which would dangle WorkingContext.
		TArray<UNOrganComponent*> Intersects;
		TArray<UNOrganComponent*> Contains;

		for (UNOrganComponent* OtherComponent : LevelComponents)
		{
			if (OtherComponent == Component)
			{
				continue;
			}

			const AVolume* OtherComponentVolume = Cast<AVolume>(OtherComponent->GetOwner());
			FBoxSphereBounds OtherVolumeBounds = OtherComponentVolume->GetBounds();

			// Check for intersection of any type
			if (FBoxSphereBounds::BoxesIntersect(ComponentBounds, OtherVolumeBounds))
			{
				AddOrganComponent(OtherComponent);
				Intersects.AddUnique(OtherComponent);
			}

			// Check for full containment
			if (FNBoundsUtils::IsBoundsContainedInBounds(OtherVolumeBounds, ComponentBounds))
			{
				AddOrganComponent(OtherComponent);
				Contains.AddUnique(OtherComponent);
			}
		}

		// Re-pin after possible rehash and commit the staged relationships.
		WorkingOrganData = OrganData.Find(Component);
		WorkingOrganData->IntersectComponents.Append(Intersects);
		WorkingOrganData->ContainedComponents.Append(Contains);
	}
	else
	{
		// An unbounded component intersects with everything, no good way about this, they will generate first
		WorkingOrganData->Bounds = FBox(FVector(MIN_dbl, MIN_dbl, MIN_dbl), FVector(MAX_dbl, MAX_dbl, MAX_dbl));
		for (UNOrganComponent* OtherComponent : LevelComponents)
		{
			if (OtherComponent == Component) continue;
			WorkingOrganData->IntersectComponents.AddUnique(OtherComponent);
		}
	}
	return true;
}

void FNAssemblyOperationContext::LockAndPreprocess(UWorld* World)
{
	bIsLocked = true;
	const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
	
	// This is the world where generation ultimately takes place
	TargetWorld = World;
	
	// Gather our bone components so we can add them to a specific organs context later
	TArray<UNBoneComponent*> BoneComponents = FNWorldAssemblyRegistry::GetBoneComponentsFromLevel(TargetWorld->GetCurrentLevel());
	BoneData.Empty();
	BoneData.Reserve(BoneComponents.Num());
	for (const auto& BoneComponent : BoneComponents)
	{
		BoneData.Add(BoneComponent, FNWorldBoneData());
		FNWorldBoneData* WorkingContext = BoneData.Find(BoneComponent);
		WorkingContext->SourceComponent = BoneComponent;
		WorkingContext->CornerPoints = BoneComponent->GetCornerPoints(Settings->SocketSize);
	}
	
	// Create a separate list of components that we will operate on and clear out.
	int32 GenerationOrderIndex = 0;
	GenerationOrder.Empty();
	GenerationOrder.Add(TArray<TObjectPtr<UNOrganComponent>>());
	TArray<UNOrganComponent*> UnboundComponents;
	TArray<UNOrganComponent*> PossibleComponents;
	TArray<UNOrganComponent*> ProcessedComponents;
	for (auto& Pair : OrganData)
	{
		// Assign values while we are iterating for defaults
		Pair.Value.MaximumRetryCount = Settings->AssemblyGenerationRetryCount;
		Pair.Value.CellHullPenetration = Settings->AssemblyJunctionMatchingCellHullPenetration;
		Pair.Value.WorldHullPenetration = Settings->AssemblyJunctionMatchingWorldPenetration;
		Pair.Value.AssemblyDirectionTolerance = Settings->AssemblyDirectionTolerance;
		Pair.Value.VoxelSize = Settings->VoxelSize;
		
		if (Pair.Key->bUnbounded)
		{
			// These will get generated last
			UnboundComponents.AddUnique(Pair.Key);
		}
		else
		{
			PossibleComponents.AddUnique(Pair.Key);	
		}
	}

	// Step 1 - Find components who have no contained "sub" organs, as they are defined and parallelizable work
	for (auto& Pair : OrganData)
	{
		// Again dont look at Unbounded here
		if (Pair.Key->bUnbounded) continue;
		
		// Handle "easy" work parallelization classification
		if (FNWorldOrganData& ComponentContext = Pair.Value; 
			ComponentContext.ContainedComponents.IsEmpty())
		{
			PossibleComponents.Remove(Pair.Key);
			ProcessedComponents.Add(Pair.Key);
			GenerationOrder[0].Add(Pair.Key);
		}
	}
	
	// Evaluate if we should bump the generation order
	if (GenerationOrder[0].Num() > 0 && PossibleComponents.Num() > 0)
	{
		GenerationOrder.Add(TArray<TObjectPtr<UNOrganComponent>>());
		GenerationOrderIndex = 1;
	}
	
	// Theres a possibility that someone creates a circular dependence with the organs, so we are going to protect against that, just incase.
	bool bMadeProgress = false;
	while (PossibleComponents.Num() > 0)
	{
		// Reset flag
		bMadeProgress = false;
		
		// We need to track the processed components per generation iteration, so if we process this loop, it needs
		// to ensure that anything that required it bumps to the next iteration.
		TArray<TObjectPtr<UNOrganComponent>> PhaseProcessed;
		
		for (int32 i = PossibleComponents.Num() - 1; i >= 0; i--)
		{
			UNOrganComponent* Component = PossibleComponents[i];
			FNWorldOrganData* TargetOrganData = OrganData.Find(Component);
			
			// Ensure all contained organs are processed / not this iteration
			bool bAllContainsProcessed = true;
			for (auto ContainedComponent : TargetOrganData->ContainedComponents)
			{
				if (!ProcessedComponents.Contains(ContainedComponent) || 
					PhaseProcessed.Contains(ContainedComponent))
				{
					bAllContainsProcessed = false;
				}
			}
			
			if (bAllContainsProcessed)
			{
				PhaseProcessed.Add(Component);
				GenerationOrder[GenerationOrderIndex].Add(Component);
				PossibleComponents.Remove(Component);
				ProcessedComponents.Add(Component);
				bMadeProgress = true;
			}
		}
		
		// Evaluate if we should bump the generation order
		if (GenerationOrder[GenerationOrderIndex].Num() > 0 && PossibleComponents.Num() > 0)
		{
			GenerationOrder.Add(TArray<TObjectPtr<UNOrganComponent>>());
			GenerationOrderIndex++;
		}
		
		if (!bMadeProgress)
		{
			UE_LOG(LogNexusWorldAssembly, Error,
				TEXT("Organ containment graph has a circular cycle; generation order cannot be determined."));
			break;
		}
	}
	
	// Handle specific-case where your generating just an unbounded organ, it will just remove the empty pass to save us later.
	if (GenerationOrder[0].IsEmpty())
	{
		GenerationOrder.RemoveAt(0);
	}

	// Handle the unbound organs last and make them each their own pass in order of the add
	const bool bHaveUnboundedBounds = UnboundComponents.Num() > 0;
	if (bHaveUnboundedBounds)
	{
		for (int32 i = 0; i < UnboundComponents.Num(); i++)
		{
			// Add to our generation order last the unbound	
			GenerationOrder.Add(TArray<TObjectPtr<UNOrganComponent>>());
			TArray<TObjectPtr<UNOrganComponent>>& NextPhase = GenerationOrder.Last();
			NextPhase.Add(UnboundComponents[i]);
		}
	}
	
	// Now that we have the generation order, we now are going to assign bones to the 'first' impacted organ.
	// This step is going to remove things from BoneComponents as they are used so DO NOT use it after this point.
	int32 BoneCount = BoneComponents.Num();
	for (const auto& Phase : GenerationOrder)
	{
		for (auto& Component : Phase)
		{
			FNWorldOrganData* TargetOrganData = OrganData.Find(Component);
			
			if (!Component->IsVolumeBased())
			{
				UE_LOG(LogNexusWorldAssembly, Warning, TEXT("Component %s is not volume based, unable to determine if UNBoneComponents are contained."), *Component->GetName());
				continue;
			}
			
			const AVolume* Volume = Component->GetVolume();
			Bounds.Add(Volume->GetBounds());
			
			for (int32 i = BoneCount - 1; i >= 0; i--)
			{
				 FNWorldBoneData* Context = BoneData.Find(BoneComponents[i]);
				if (Volume->EncompassesPoint(Context->CornerPoints[0]) || 
					Volume->EncompassesPoint(Context->CornerPoints[1]) ||
					Volume->EncompassesPoint(Context->CornerPoints[2]) ||
					Volume->EncompassesPoint(Context->CornerPoints[3]))
				{
					TargetOrganData->ContainedBones.Add(Context);
					BoneComponents.RemoveAt(i);
					BoneCount--;
				}
			}
		}
	}
	
	// If we have an unbounded volume, we need touch the entire world, unfortunately.
	if (bHaveUnboundedBounds)
	{
		Bounds.Empty();
	}
}

#if !UE_BUILD_SHIPPING
void FNAssemblyOperationContext::AddToReport(FNReport* Report, const bool bBuildTissues)
{
	const int32 OperationContextContentTicket = Report->CreateContentBlock();
	FNReportContentBlock* OperationContextContentBlock = Report->GetContentBlock(OperationContextContentTicket);
	if (!OperationSettings.DisplayName.IsEmpty())
	{
		OperationContextContentBlock->SetHeading(*OperationSettings.DisplayName.ToString());
	}
	else
	{
		OperationContextContentBlock->SetHeading("Unnamed Operation");
	}
	
	const int32 OverviewTableTicket = Report->CreateTableBlock(OperationContextContentTicket);
	FNReportTableBlock* OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->Initialize({ "Ticket", "Lock Status", "Runtime"});
	OverviewTable->AddRow({FString::FromInt(OperationTicket), bIsLocked ? "Yes" : "No", "{{RUNTIME}}"});
	
	Report->AddReplaceToken("{{RUNTIME}}", ""); // We'll fill this out later
	
	// Components
	const int32 ComponentTableTicket = Report->CreateTableBlock(OperationContextContentTicket);
	FNReportTableBlock* ComponentTableBlock = Report->GetTableBlock(ComponentTableTicket);
	ComponentTableBlock->SetHeading(FString::Printf(TEXT("Components (%i)"), OrganData.Num()));
	ComponentTableBlock->Initialize({ "Component", "Intersections", "Contains", "Bones", "Tissues" });
	
	FStringBuilderBase Builder;
	
	for (auto Data : OrganData)
	{
		// Intersections
		Builder.Reset();
		for (const auto IntersectComponent : Data.Value.IntersectComponents)
		{
			Builder.Append(IntersectComponent->GetDebugLabel());
			Builder.Append(", ");
		}
		if (Builder.Len() > 2)
		{
			Builder.RemoveSuffix(2);
		}
		FString Intersections = Builder.ToString();
		
		// Fully Contains
		Builder.Reset();
		for (const auto ContainedComponent : Data.Value.ContainedComponents)
		{
			Builder.Append(ContainedComponent->GetDebugLabel());
			Builder.Append(", ");
		}
		if (Builder.Len() > 2)
		{
			Builder.RemoveSuffix(2);
		}
		FString Contains = Builder.ToString();
		
		// Bones
		Builder.Reset();
		for (const auto ContainedBone : Data.Value.ContainedBones)
		{
			Builder.Append(ContainedBone->SourceComponent->GetDebugLabel());
			Builder.Append(", ");
		}
		if (Builder.Len() > 2)
		{
			Builder.RemoveSuffix(2);
		}
		FString Bones = Builder.ToString();
		
		// Optionally list tissue buildout
		FString TissueList = TEXT("");
		if (bBuildTissues)
		{
			TMap<TObjectPtr<UNCell>, FNTissueEntry> BuildTissue;
			FNTissueTagGroups TagGroups; // TODO: Add to report?
			Data.Value.SourceComponent->GetTissueMap(BuildTissue, TagGroups);
			
			Builder.Reset();
			for (const auto& TissuePair : BuildTissue)
			{
				Builder.Append(FString::Printf(TEXT("%s (%i), "), *TissuePair.Value.Cell.GetAssetName(), TissuePair.Value.Weighting));
			}
			if (Builder.Len() > 2)
			{
				Builder.RemoveSuffix(2);
			}
			TissueList = Builder.ToString();
		}
		
		ComponentTableBlock->AddRow({ *Data.Value.SourceComponent->GetDebugLabel(), *Intersections, *Contains, *Bones, *TissueList });
	}
	
	
	const int32 GenerationOrderContentTicket = Report->CreateTableBlock(OperationContextContentTicket);
	FNReportTableBlock* GenerationOrderTableBlock = Report->GetTableBlock(GenerationOrderContentTicket);
	GenerationOrderTableBlock->SetHeading(FString::Printf(TEXT("Generation Order (%i)"), GenerationOrder.Num()));
	GenerationOrderTableBlock->Initialize( { "Phase", "Organs" } );
	for (int32 i = 0; i < GenerationOrder.Num(); i++)
	{
		Builder.Reset();
		for (const auto Component : GenerationOrder[i])
		{
			Builder.Append(*Component->GetDebugLabel());
			Builder.Append(", ");
		}
		if (Builder.Len() > 2)
		{
			Builder.RemoveSuffix(2);
		}
		GenerationOrderTableBlock->AddRow({ FString::FromInt(i), Builder.ToString() });
	}
}
#endif // !UE_BUILD_SHIPPING

