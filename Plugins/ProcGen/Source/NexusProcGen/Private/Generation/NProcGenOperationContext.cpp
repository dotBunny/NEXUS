// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationContext.h"

#include "NProcGenMinimal.h"
#include "NProcGenRegistry.h"
#include "NProcGenSettings.h"
#include "NProcGenUtils.h"
#include "Cell/NTissue.h"
#include "Math/NBoundsUtils.h"
#include "Organ/NOrganComponent.h"

void FNProcGenOperationContext::ResetContext()
{
	OrganContext.Empty();
	BoneContext.Empty();
	bIsLocked = false;
}

void FNProcGenOperationContext::SetOperationSettings(const FNProcGenOperationSettings& InSettings)
{
	
	if (IsLocked())
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to copy settings on FNOrganGenerationContext when it has already been locked."));
	}
	else
	{
		OperationSettings = InSettings; 
	}
}

bool FNProcGenOperationContext::AddOrganComponent(UNOrganComponent* Component)
{
	if (IsLocked())
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Attempted to add additional context from a UNOrganComponent when the FNOrganGenerationContext has already been locked."))
		return false;
	}
	
	// We've already added this component
	if (OrganContext.Contains(Component))
	{
		return true;
	}
	
	InputComponents.Add(Component);
	OrganContext.Add(Component, FNProcGenOperationOrganContext());
	FNProcGenOperationOrganContext* WorkingContext = OrganContext.Find(Component);
	WorkingContext->SourceComponent = Component;

	// We're going to capture all the other components in the level
	TArray<UNOrganComponent*> LevelComponents = FNProcGenUtils::GetOrganComponentsFromLevel(Component->GetComponentLevel());

	// We will handle ordering when we lock the context of the generation
	if (Component->IsVolumeBased())
	{
		// Look at volume, search for sub volumes to add context too? order matters?
		const AVolume* ComponentVolume = Cast<AVolume>(Component->GetOwner());
		const FBoxSphereBounds ComponentVolumeBounds = ComponentVolume->GetBounds();
		for (UNOrganComponent* OtherComponent : LevelComponents)
		{
			if (OtherComponent == Component)
			{
				continue;
			}

			const AVolume* OtherComponentVolume = Cast<AVolume>(OtherComponent->GetOwner());
			FBoxSphereBounds OtherVolumeBounds = OtherComponentVolume->GetBounds();
			
			// Check for intersection of any type
			if (FBoxSphereBounds::BoxesIntersect(ComponentVolumeBounds, OtherVolumeBounds))
			{
				AddOrganComponent(OtherComponent);
				WorkingContext->IntersectComponents.AddUnique(OtherComponent);
			}
			
			// Check for full containment
			if (FNBoundsUtils::IsBoundsContainedInBounds(OtherVolumeBounds, ComponentVolumeBounds))
			{
				AddOrganComponent(OtherComponent);
				WorkingContext->ContainedComponents.AddUnique(OtherComponent);
			}
		}
	}
	
	return true;
}

void FNProcGenOperationContext::LockAndPreprocess(UWorld* World)
{
	bIsLocked = true;
	const UNProcGenSettings* Settings = UNProcGenSettings::Get();
	
	// This is the world where generation ultimately takes place
	TargetWorld = World;
	
	// Gather our bone components so we can add them to a specific organs context later
	TArray<UNBoneComponent*> BoneComponents = FNProcGenRegistry::GetBoneComponentsFromLevel(TargetWorld->GetCurrentLevel());
	BoneContext.Empty();
	BoneContext.Reserve(BoneComponents.Num());
	for (const auto& BoneComponent : BoneComponents)
	{
		BoneContext.Add(BoneComponent, FNProcGenOperationBoneContext());
		FNProcGenOperationBoneContext* WorkingContext = BoneContext.Find(BoneComponent);
		WorkingContext->SourceComponent = BoneComponent;
		WorkingContext->CornerPoints = BoneComponent->GetCornerPoints(Settings->SocketSize);
	}
	
	// Create a separate list of components that we will operate on and clear out.
	int GenerationOrderIndex = 0;
	GenerationOrder.Empty();
	GenerationOrder.Add(TArray<UNOrganComponent*>());
	TArray<UNOrganComponent*> PossibleComponents;
	TArray<UNOrganComponent*> ProcessedComponents;
	for (auto& Pair : OrganContext)
	{
		// Assign values while we are iterating for defaults
		Pair.Value.MaximumRetryCount = Settings->OrganGenerationRetryCount;
		
		// TODO: DO we want to check if the component is actually in the current level here?
		PossibleComponents.AddUnique(Pair.Key);
	}

	// Step 1 - Find components who have no contained "sub" organs, as they are defined and parallelizable work
	for (auto& Pair : OrganContext)
	{
		// Handle "easy" work parallelization classification
		if (FNProcGenOperationOrganContext& ComponentContext = Pair.Value; 
			ComponentContext.ContainedComponents.Num() == 0)
		{
			PossibleComponents.Remove(Pair.Key);
			ProcessedComponents.Add(Pair.Key);
			GenerationOrder[0].Add(Pair.Key);
		}
	}
	
	// Evaluate if we should bump the generation order
	if (GenerationOrder[0].Num() > 0 && PossibleComponents.Num() > 0)
	{
		GenerationOrder.Add(TArray<UNOrganComponent*>());
		GenerationOrderIndex = 1;
	}
	
	while (PossibleComponents.Num() > 0)
	{
		// We need to track the processed components per generation iteration, so if we process this loop, it needs
		// to ensure that anything that required it bumps to the next iteration.
		TArray<UNOrganComponent*> PhaseProcessed;
		
		for (int i = PossibleComponents.Num() - 1; i >= 0; i--)
		{
			UNOrganComponent* Component = PossibleComponents[i];
			FNProcGenOperationOrganContext* ComponentContext = OrganContext.Find(Component);
			
			// Ensure all contained organs are processed / not this iteration
			bool bAllContainsProcessed = true;
			for (auto ContainedComponent : ComponentContext->ContainedComponents)
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
			}
		}
		
		// Evaluate if we should bump the generation order
		if (GenerationOrder[GenerationOrderIndex].Num() > 0 && PossibleComponents.Num() > 0)
		{
			GenerationOrder.Add(TArray<UNOrganComponent*>());
			GenerationOrderIndex++;
		}
	}
	
	// Now that we have the generation order, we now are going to assign bones to the 'first' impacted organ.
	// This step is going to remove things from BoneComponents as they are used so DO NOT use it after this point.
	int BoneCount = BoneComponents.Num();
	for (const auto& Phase : GenerationOrder)
	{
		for (auto& Component : Phase)
		{
			FNProcGenOperationOrganContext* OrganGenerationContext = OrganContext.Find(Component);
			
			if (!Component->IsVolumeBased())
			{
				UE_LOG(LogNexusProcGen, Warning, TEXT("Component %s is not volume based, unable to determine if UNBoneComponents are contained."), *Component->GetName());
				continue;
			}
			
			const AVolume* Volume = Component->GetVolume();
			
			for (int i = BoneCount - 1; i >= 0; i--)
			{
				 FNProcGenOperationBoneContext* Context = BoneContext.Find(BoneComponents[i]);
				if (Volume->EncompassesPoint(Context->CornerPoints[0]) || 
					Volume->EncompassesPoint(Context->CornerPoints[1]) ||
					Volume->EncompassesPoint(Context->CornerPoints[2]) ||
					Volume->EncompassesPoint(Context->CornerPoints[3]))
				{
					OrganGenerationContext->ContainedBones.Add(Context);
					BoneComponents.RemoveAt(i);
					BoneCount--;
				}
			}
		}
	}

	// TODO: Search voxelized? inside known spaces for other objects
	// Capture their convex hulls?
	// - Calculate obstructions in the world itself already that need to be accounted for.
	// need to get bounds?
	
}

void FNProcGenOperationContext::OutputToLog(const bool bBuildTissues)
{
	FStringBuilderBase Builder = FStringBuilderBase();
	Builder.Append(TEXT("\n[FNOrganContext] "));
	if (!OperationSettings.DisplayName.IsEmpty())
	{
		Builder.Append(*OperationSettings.DisplayName.ToString());
		Builder.Append(TEXT(" "));
	}
	
	if (bIsLocked)
	{
		Builder.Append(TEXT("LOCKED\n"));
	}
	else
	{
		Builder.Append("\n");
	}
	
	Builder.Appendf(TEXT("\tComponents (%i)\n"), OrganContext.Num());
	for (auto Component : OrganContext)
	{
		Builder.Appendf(TEXT("\t\tSource: %s\n"), *Component.Value.SourceComponent->GetDebugLabel());
		for (const auto IntersectComponent : Component.Value.IntersectComponents)
		{
			Builder.Appendf(TEXT("\t\t\tIntersects: %s\n"), *IntersectComponent->GetDebugLabel());
		}
		for (const auto ContainedComponent : Component.Value.ContainedComponents)
		{
			Builder.Appendf(TEXT("\t\t\tContains: %s\n"), *ContainedComponent->GetDebugLabel());
		}
		for (const auto ContainedBone : Component.Value.ContainedBones)
		{
			Builder.Appendf(TEXT("\t\t\tBone: %s\n"), *ContainedBone->SourceComponent->GetDebugLabel());
		}
		
		// Optionally list tissue buildout
		if (bBuildTissues)
		{
			Builder.Append(TEXT("\t\t\tTissues:\n"));
			TMap<TObjectPtr<UNCell>, FNTissueEntry> BuildTissue = Component.Value.SourceComponent->GetTissueMap();
		
			for (const auto& TissuePair : BuildTissue)
			{
				Builder.Appendf(TEXT("\t\t\t\t- %s (%i)\n"), *TissuePair.Value.Cell.GetAssetName(), TissuePair.Value.Weighting);
			}
		}
	}
	
	Builder.Appendf(TEXT("\tGeneration Order (%i)\n"), GenerationOrder.Num());
	for (int i = 0; i < GenerationOrder.Num(); i++)
	{
		Builder.Appendf(TEXT("\t\tPhase (%i)\n"), i);
		for (const auto Component : GenerationOrder[i])
		{
			Builder.Appendf(TEXT("\t\t\t%s\n"), *Component->GetDebugLabel());
		}
		Builder.Append("\n");
	}

	UE_LOG(LogNexusProcGen, Log, TEXT("%s"), Builder.ToString());
}

