// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganContext.h"

#include "NCoreMinimal.h"
#include "NProcGenUtils.h"
#include "Math/NBoundsUtils.h"
#include "Organ/NOrganComponentContext.h"

void FNOrganContext::Reset()
{
	Components.Empty();
	bIsLocked = false;
}



bool FNOrganContext::AddOrganComponent(UNOrganComponent* Component)
{
	if (IsLocked())
	{
		N_LOG(Warning, TEXT("[FNOrganContext::AddToContext] Attempted to add context from a UNProcGenComponent when the Context has been locked."))
		return false;
	}
	
	// We've already added this component
	if (Components.Contains(Component))
	{
		return true;
	}
	
	Components.Add(Component, FNOrganComponentContext(Component));
	FNOrganComponentContext* WorkingContext = Components.Find(Component);

	// We're going to capture all the other components in the level
	TArray<UNOrganComponent*> LevelComponents = FNProcGenUtils::GetOrganComponentsFromLevel(Component->GetComponentLevel());

	// We will handle ordering when we lock the context of generation
	if (Component->IsVolumeBased())
	{
		// Look at volume, search for sub volumes to add context too? order matters?
		AVolume* ComponentVolume = Cast<AVolume>(Component->GetOwner());
		FBoxSphereBounds ComponentVolumeBounds = ComponentVolume->GetBounds();
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

void FNOrganContext::LockAndPreprocess()
{
	bIsLocked = true;
	
	// TODO: Figure out ordering now
	
	/* 
Log          LogNexus                  [FNOrganContext] LOCKED
Log          LogNexus                  	Components (4)
Log          LogNexus                  		Source: Organ_Main
Log          LogNexus                  			Intersects: Organ_Sub
Log          LogNexus                  			Intersects: Organ_Partial
Log          LogNexus                  			Intersects: Organ_Sub_Sub
Log          LogNexus                  			Contains: Organ_Sub
Log          LogNexus                  			Contains: Organ_Sub_Sub
Log          LogNexus                  		Source: Organ_Sub
Log          LogNexus                  			Intersects: Organ_Main
Log          LogNexus                  			Intersects: Organ_Sub_Sub
Log          LogNexus                  			Contains: Organ_Sub_Sub
Log          LogNexus                  		Source: Organ_Sub_Sub
Log          LogNexus                  			Intersects: Organ_Main
Log          LogNexus                  			Intersects: Organ_Sub
Log          LogNexus                  		Source: Organ_Partial
Log          LogNexus                  			Intersects: Organ_Main
	*/
	
	
	for (auto& Pair : Components)
	{
		FNOrganComponentContext&  ComponentContext = Pair.Value;
		const int ChildCount = ComponentContext.IntersectComponents.Num();
		for (int i = 0; i < ChildCount; i++)
		{
			UNOrganComponent* OtherOrganComponent = ComponentContext.IntersectComponents[i];
		}
		
	}
	
	// Figure out who is inside of who?
	
	// Itterate through 'base' organs, the other components are all the ones that intersect
	
	
	
	// look at all children
	// figure out who is nested in who
	
	
}

void FNOrganContext::OutputToLog()
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
	for (auto Pair : Components)
	{
		// Source of generation graph
		Builder.Appendf(TEXT("\t\tSource: %s\n"), *Pair.Value.SourceComponent->GetDebugLabel());
		for (auto Pair2 : Pair.Value.IntersectComponents)
		{
			Builder.Appendf(TEXT("\t\t\tIntersects: %s\n"), *Pair2->GetDebugLabel());
		}
		for (auto Pair2 : Pair.Value.ContainedComponents)
		{
			Builder.Appendf(TEXT("\t\t\tContains: %s\n"), *Pair2->GetDebugLabel());
		}
	}

	N_LOG(Log, TEXT("%s"), Builder.ToString());
}

