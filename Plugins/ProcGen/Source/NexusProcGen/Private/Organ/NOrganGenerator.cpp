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

	// TODO: We need to figure out the generation order

	Context->OutputToLog();
}

