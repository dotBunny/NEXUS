// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenOperation.h"

#include "NProcGenMinimal.h"
#include "NProcGenNamespace.h"
#include "NProcGenRegistry.h"
#include "NProcGenUtils.h"
#include "Organ/NOrganComponent.h"
#include "Generation/NProcGenOperationTaskGraph.h"

UNProcGenOperation::UNProcGenOperation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = new FNProcGenOperationContext();
	
	// A generator should never be deleted
	this->AddToRoot();
	
	if (!this->IsTemplate())
	{
		FNProcGenRegistry::RegisterOperation(this);
	}
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, const FString& Seed, const FText& DisplayName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	if (DisplayName.IsEmpty())
	{
		OrganGenerator->DisplayName = FText::FromString(OrganGenerator->GetName());
	}
	else
	{
		OrganGenerator->DisplayName = DisplayName;
	}
	OrganGenerator->SetSeedOnContext(Seed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *OrganGenerator->DisplayName.ToString(), *Seed)
	
	
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

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<UNOrganComponent*>& Components, const FString& Seed, const FText& DisplayName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	if (DisplayName.IsEmpty())
	{
		OrganGenerator->DisplayName = FText::FromString(OrganGenerator->GetName());
	}
	else
	{
		OrganGenerator->DisplayName = DisplayName;
	}
	OrganGenerator->SetSeedOnContext(Seed);
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *OrganGenerator->DisplayName.ToString(), *Seed)
	
	for (const auto Component : Components)
	{
		OrganGenerator->AddToContext(Component);
	}
	
	return OrganGenerator;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(UNOrganComponent* BaseComponent, const FString& Seed, const FText& DisplayName)
{
	UNProcGenOperation* OrganGenerator = NewObject<UNProcGenOperation>();
	if (DisplayName.IsEmpty())
	{
		OrganGenerator->DisplayName = FText::FromString(OrganGenerator->GetName());
	}
	else
	{
		OrganGenerator->DisplayName = DisplayName;
	}
	OrganGenerator->SetSeedOnContext(Seed);
	
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *OrganGenerator->DisplayName.ToString(), *Seed)
	
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

void UNProcGenOperation::SetDisplayMessage(FString NewDisplayMessage)
{
	if (!DisplayMessage.Equals(NewDisplayMessage))
	{
		UE_LOG(LogNexusProcGen, Log, TEXT("[%s] DisplayMessage('%s' to '%s')"), *DisplayName.ToString(), *DisplayMessage,  *NewDisplayMessage);
		DisplayMessage = NewDisplayMessage;
		OnDisplayMessageChanged.Broadcast(DisplayMessage);
	}
}

void UNProcGenOperation::BeginDestroy()
{
	if (Owner != nullptr)
	{
		Owner->OnOperationDestroyed(this);
		Owner = nullptr;
	}
	
	if (Context != nullptr)
	{
		Context->Reset();
		delete Context;

		Context = nullptr;
	}
	
	if (Graph != nullptr)
	{
		Graph->Reset();
		delete Graph;
		
		Graph = nullptr;
	}

	if (!this->IsTemplate())
	{
		FNProcGenRegistry::UnregisterOperation(this);
	}
	
	this->RemoveFromRoot();
	
	Super::BeginDestroy();
}

void UNProcGenOperation::Tick()
{
	if (Graph == nullptr) return;
	
	if (const FIntVector2 Status = Graph->GetTaskStatus(); 
		Status.Y != CachedTotalTasks || Status.X != CachedCompletedTasks)
	{
		CachedTotalTasks = Status.Y;
		CachedCompletedTasks = Status.X;

		OnTasksChanged.Broadcast(CachedCompletedTasks, CachedTotalTasks);
	}
}

void UNProcGenOperation::FinishBuild()
{

	if (Owner != nullptr)
	{
		Owner->OnOperationFinished(this);
	}
	
	// Were going to delete this object
	ConditionalBeginDestroy();
}

void UNProcGenOperation::StartBuild(INProcGenOperationOwner* Caller)
{
	// Cache our caller
	Owner = Caller;
	
	// Ensure that we have locked context and done the preprocessing.
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess();
		SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::ContextLocked);
	}
	
	// TODO: We shouldn't have a graph, but maybe we do?
	if (Graph != nullptr)
	{
		Graph->Reset();
		delete Graph;
	}
	
	// Build out our new graph
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::BuildingTaskGraph);
	Graph = new FNProcGenOperationTaskGraph(this, Context);
	
	// Add callback to tasks?
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::StartingTasks);
	Graph->UnlockTasks();
}

void UNProcGenOperation::SetSeedOnContext(const FString& NewSeed) const
{
	if (Context->IsLocked())
	{
		UE_LOG(LogNexusProcGen, Warning, TEXT("Unable to set the friendly seed on FNOrganGenerationContext when it has already been locked."));
	}
	else
	{
		Context->FriendlySeed = NewSeed;
	}
}

bool UNProcGenOperation::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}

void UNProcGenOperation::LockContext()
{
	Context->LockAndPreprocess();
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::ContextLocked);
}

