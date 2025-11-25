// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenOperation.h"

#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"
#include "Generation/NOrganGeneratorTasks.h"

UNProcGenOperation::UNProcGenOperation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNOrganGenerationContext();
	
	// A generator should never be deleted
	this->AddToRoot();
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	
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

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<UNOrganComponent*>& Components)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	
	for (const auto Component : Components)
	{
		OrganGenerator->AddToContext(Component);
	}
	
	return OrganGenerator;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(UNOrganComponent* BaseComponent)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	
	OrganGenerator->AddToContext(BaseComponent);
	
	return OrganGenerator;
}

void UNProcGenOperation::Reset() const
{
	if (Context != nullptr)
	{
		Context->Reset();
	}
}

void UNProcGenOperation::BeginDestroy()
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

void UNProcGenOperation::FinishBuild()
{
	
	// TODO: Not great but allows us to go both ways and its not frequent
	if (ExecuteCaller->IsValidLowLevel())
	{
		if (UFunction* Function = ExecuteCaller->FindFunction(TEXT("OnOperationFinished")))
		{
			ExecuteCaller->ProcessEvent(Function, this);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Organ Generator Finished Build"));
	
	// Were going to delete this object
	ConditionalBeginDestroy();
}


void UNProcGenOperation::StartBuild(UObject* Caller)
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

bool UNProcGenOperation::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}


void UNProcGenOperation::LockContext()
{
	Context->LockAndPreprocess();
}

