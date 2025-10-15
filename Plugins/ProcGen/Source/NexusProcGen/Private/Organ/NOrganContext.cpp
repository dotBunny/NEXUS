// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganContext.h"

#include "NCoreMinimal.h"
#include "NProcGenUtils.h"
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
			if (FBoxSphereBounds OtherVolumeBounds = OtherComponentVolume->GetBounds();
				ComponentVolumeBounds.BoxesIntersect(ComponentVolumeBounds, OtherVolumeBounds))
			{
				AddOrganComponent(OtherComponent);
				WorkingContext->OtherComponents.AddUnique(OtherComponent);
			}
		}
	}
	return true;
}

void FNOrganContext::LockAndPreprocess()
{
	bIsLocked = true;
	
	// TODO: Figure out ordering now
	
	
	// Loop through each base (really there should only ever be one, but here we are)
	for (auto& Pair : Components)
	{
		FNOrganComponentContext&  ComponentContext = Pair.Value;
		const int ChildCount = ComponentContext.OtherComponents.Num();
		for (int i = 0; i < ChildCount; i++)
		{
			UNOrganComponent* OtherOrganComponent = ComponentContext.OtherComponents[i];
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
		for (auto Pair2 : Pair.Value.OtherComponents)
		{
			Builder.Appendf(TEXT("\t\t\tOtherComponent: %s\n"), *Pair2->GetDebugLabel());
		}
	}

	N_LOG(Log, TEXT("%s"), Builder.ToString());
}

