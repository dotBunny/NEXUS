// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGenerator.h"

#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"

UNOrganGenerator::UNOrganGenerator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNOrganContext();
}

UNOrganGenerator* UNOrganGenerator::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects)
{
	UNOrganGenerator* OrganGenerator = NewObject<UNOrganGenerator>();
	
	for (TWeakObjectPtr<UObject> WeakObject : Objects)
	{
		if (UObject* Object = WeakObject.Get())
		{
			if (UNOrganComponent* Component = Cast<UNOrganComponent>(Object))
			{
				OrganGenerator->AddToContext(Component);
			}
		}
	}
	
	return OrganGenerator;
}

UNOrganGenerator* UNOrganGenerator::CreateInstance(const TArray<UNOrganComponent*>& Components)
{
	UNOrganGenerator* OrganGenerator = NewObject<UNOrganGenerator>();
	
	for (const auto Component : Components)
	{
		OrganGenerator->AddToContext(Component);
	}
	
	return OrganGenerator;
}

UNOrganGenerator* UNOrganGenerator::CreateInstance(UNOrganComponent* BaseComponent)
{
	UNOrganGenerator* OrganGenerator = NewObject<UNOrganGenerator>();
	
	OrganGenerator->AddToContext(BaseComponent);
	
	return OrganGenerator;
}

void UNOrganGenerator::Reset() const
{
	if (Context != nullptr)
	{
		Context->Reset();
	}
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
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess();
	}
	
	// TODO: Do
}

bool UNOrganGenerator::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}


void UNOrganGenerator::LockContext()
{
	Context->LockAndPreprocess();
}

