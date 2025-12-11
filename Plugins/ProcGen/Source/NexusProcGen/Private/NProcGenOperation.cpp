// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenOperation.h"

#include "NProcGenRegistry.h"
#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"
#include "Generation/NOrganGeneratorTasks.h"

UNProcGenOperation::UNProcGenOperation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNOrganGenerationContext();
	
	// A generator should never be deleted
	this->AddToRoot();
	
	if (!this->IsTemplate())
	{
		FNProcGenRegistry::RegisterOperation(this);
	}
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, FName ObjectName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>(GetTransientPackage(), StaticClass(), ObjectName);
	
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

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<UNOrganComponent*>& Components, const FName ObjectName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>(GetTransientPackage(), StaticClass(), ObjectName);
	
	for (const auto Component : Components)
	{
		OrganGenerator->AddToContext(Component);
	}
	
	return OrganGenerator;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(UNOrganComponent* BaseComponent, const FName ObjectName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>(GetTransientPackage(), StaticClass(), ObjectName);
	
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

	if (!this->IsTemplate())
	{
		FNProcGenRegistry::UnregisterOperation(this);
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

