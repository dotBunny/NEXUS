// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGenerationContext.h"

#include "NProcGenMinimal.h"
#include "NProcGenUtils.h"
#include "Math/NBoundsUtils.h"
#include "Organ/NOrganComponent.h"

void FNOrganGenerationContext::Reset()
{
	Components.Empty();
	bIsLocked = false;
}

bool FNOrganGenerationContext::AddOrganComponent(UNOrganComponent* Component)
{
	if (IsLocked())
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Attempted to add additional context from a UNOrganComponent when the FNOrganGenerationContext has already been locked."))
		return false;
	}
	
	// We've already added this component
	if (Components.Contains(Component))
	{
		return true;
	}
	
	Components.Add(Component, FNOrganGenerationContextMap());
	FNOrganGenerationContextMap* WorkingContext = Components.Find(Component);
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

void FNOrganGenerationContext::LockAndPreprocess()
{
	bIsLocked = true;
	
	// Create a separate list of components that we will operate on and clear out.
	int GenerationOrderIndex = 0;
	GenerationOrder.Empty();
	GenerationOrder.Add(TArray<UNOrganComponent*>());
	TArray<UNOrganComponent*> PossibleComponents;
	TArray<UNOrganComponent*> ProcessedComponents;
	for (auto& Pair : Components)
	{
		PossibleComponents.AddUnique(Pair.Key);
	}

	// Step 1 - Find components who have no contained "sub" organs, as they are defined and parallelizable work
	for (auto& Pair : Components)
	{
		if (FNOrganGenerationContextMap& ComponentContext = Pair.Value; 
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
			FNOrganGenerationContextMap* ComponentContext = Components.Find(Component);
			
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
}

void FNOrganGenerationContext::OutputToLog()
{
	FStringBuilderBase Builder = FStringBuilderBase();
	Builder.Append(TEXT("\n[FNOrganContext]"));
	if (bIsLocked)
	{
		Builder.Append(TEXT(" LOCKED\n"));
	}
	else
	{
		Builder.Append("\n");
	}
	
	Builder.Appendf(TEXT("\tComponents (%i)\n"), Components.Num());
	for (auto Component : Components)
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

