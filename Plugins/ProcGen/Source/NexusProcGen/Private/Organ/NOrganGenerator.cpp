// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGenerator.h"

#include "NCoreMinimal.h"
#include "NProcGenUtils.h"
#include "Organ/NOrganProcGenComponentContext.h"

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

bool UNOrganGenerator::AddToContext(UNProcGenComponent* Component)
{
	if (bIsContextLocked)
	{
		N_LOG(Warning, TEXT("[UNOrganGenerator::AddContext] Attempted to add context from a UNProcGenComponent when the Context has been locked."))
		return false;
	}
	
	// Find/Add context for the component
	if (!Context->Components.Contains(Component))
	{
		Context->Components.Add(Component, FNOrganProcGenComponentContext(Component));
	}
	FNOrganProcGenComponentContext* WorkingContext = Context->Components.Find(Component);
	WorkingContext->Reset();


	// We're going to capture all the other components in the level
	TArray<UNProcGenComponent*> LevelComponents = FNProcGenUtils::GetNProcGenComponentsFromLevel(Component->GetComponentLevel());

	// need to figre out what volumes are inside of other volumes and build them innard outward
	
	if (Component->IsVolumeBased())
	{
		// Look at volume, search for sub volumes to add context too? order matters?
		AVolume* ComponentVolume = Cast<AVolume>(Component->GetOwner());
		FBoxSphereBounds ComponentVolumeBounds = ComponentVolume->GetBounds();
		for (UNProcGenComponent* OtherComponent : LevelComponents)
		{
			if (OtherComponent == Component)
			{
				continue;
			}

			AVolume* OtherComponentVolume = Cast<AVolume>(OtherComponent->GetOwner());
			FBoxSphereBounds OtherVolumeBounds = OtherComponentVolume->GetBounds();
			
			if (ComponentVolumeBounds.BoxesIntersect(ComponentVolumeBounds, OtherVolumeBounds))
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

