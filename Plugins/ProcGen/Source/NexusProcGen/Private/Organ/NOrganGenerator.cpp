// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganGenerator.h"

#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganGeneratorTasks.h"

UNOrganGenerator::UNOrganGenerator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNOrganGeneratorContext();
	
	// A generator should never be deleted
	this->AddToRoot();
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
	this->RemoveFromRoot();
	
	if (Context != nullptr)
	{
		Context->Reset();
		delete Context;

		Context = nullptr;
	}
	
	if (Tasks != nullptr)
	{
		Tasks->Reset();
		delete Tasks;
		
		Tasks = nullptr;
	}

	
	Super::BeginDestroy();
}

void UNOrganGenerator::FinishBuild()
{
	
	// TODO: Not great but allows us to go both ways and its not frequent
	if (ExecuteCaller->IsValidLowLevel())
	{
		if (UFunction* Function = ExecuteCaller->FindFunction(TEXT("OnFinishedBuild")))
		{
			ExecuteCaller->ProcessEvent(Function, this);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Organ Generator Finished Build"));
	
	// Were going to delete this object
	ConditionalBeginDestroy();
}


void UNOrganGenerator::StartBuild(UObject* Caller)
{
	// Cache our caller
	ExecuteCaller = Caller;
	
	// Ensure that we have locked context, and done the preprocess.
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess();
	}
	
	// TODO: We shouldnt have a graph, but maybe we do?
	if (Tasks != nullptr)
	{
		Tasks->Reset();
		delete Tasks;
	}
	
	// Build out our new graph
	Tasks = new FNOrganGeneratorTasks(this, Context);
	
	// Add callback to tasks?
	
	Tasks->UnlockTasks();
	
}

bool UNOrganGenerator::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}


void UNOrganGenerator::LockContext()
{
	Context->LockAndPreprocess();
}

