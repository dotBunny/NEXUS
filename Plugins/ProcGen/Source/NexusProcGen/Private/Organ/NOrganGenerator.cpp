// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGenerator.h"

#include "NCoreMinimal.h"
#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganComponentContext.h"

UNOrganGenerator::UNOrganGenerator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNOrganContext();
}

void UNOrganGenerator::BeginDestroy()
{
	if (Context != nullptr)
	{
		Context->Reset();
		delete Context;
	}
	UObject::BeginDestroy();
}


void UNOrganGenerator::Generate()
{
	// Ensure that we have locked context, and done the preprocess.
	if (!bIsContextLocked)
	{
		LockContext();
	}
}

bool UNOrganGenerator::AddToContext(UNOrganComponent* Component) const
{
	if (bIsContextLocked)
	{
		N_LOG(Warning, TEXT("[UNOrganGenerator::AddContext] Attempted to add context from a UNProcGenComponent when the Context has been locked."))
		return false;
	}
	
	// Find/Add context for the component
	if (!Context->Components.Contains(Component))
	{
		Context->Components.Add(Component, FNOrganComponentContext(Component));
	}
	FNOrganComponentContext* WorkingContext = Context->Components.Find(Component);
	WorkingContext->Reset();


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
				WorkingContext->OtherComponents.AddUnique(OtherComponent);
			}
		}
	}
	return true;
}

void UNOrganGenerator::LockContext()
{
	bIsContextLocked = true;

	// TODO: We need to figure out the generation order

	Context->OutputLockedContext();
}

