// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenOperation.h"

#include "NProcGenMinimal.h"
#include "NProcGenRegistry.h"
#include "Organ/NOrganComponent.h"
#include "Generation/NProcGenOperationTaskGraph.h"

UNProcGenOperation::UNProcGenOperation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = MakeUnique<FNProcGenOperationContext>();
	
	// A generator should never be deleted
	this->AddToRoot();
	
	if (!this->IsTemplate())
	{
		FNProcGenRegistry::RegisterOperation(this);
	}
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<UNOrganComponent*>& Components, const FNProcGenOperationSettings& Settings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(Settings);
	for (const auto Component : Components)
	{
		Operation->AddToContext(Component);
	}
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *Settings.Seed)
	return Operation;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, const FNProcGenOperationSettings& Settings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(Settings);
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Objects);
	for (const auto Organ : OrganComponents)
	{
		Operation->AddToContext(Organ);
	}
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *Settings.Seed)
	return Operation;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(UNOrganComponent* BaseComponent, const FNProcGenOperationSettings& Settings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(Settings);
	Operation->AddToContext(BaseComponent);
	
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *Settings.Seed)
	return Operation;
}

void UNProcGenOperation::ApplySettings(const FNProcGenOperationSettings& Settings)
{
	if (Settings.DisplayName.IsEmpty())
	{
		DisplayName = FText::FromString(GetName());
	}
	else
	{
		DisplayName = Settings.DisplayName;
	}
	
	// TODO: Add settings to context?
	
	Context->SetDisplayName(DisplayName.ToString());
	Context->ApplySettings(Settings);
}

void UNProcGenOperation::Reset() const
{
	if (Context != nullptr)
	{
		Context->ResetContext();
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
	
	if (Context.IsValid())
	{
		Context->ResetContext();
		Context.Reset();
	}
	
	if (Graph.IsValid())
	{
		Graph->ResetGraph();
		Graph.Reset();
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
	if (!Graph.IsValid()) return;
	
	if (const FIntVector2 Status = Graph->GetTaskStatus(); 
		Status.Y != CachedTotalTasks || Status.X != CachedCompletedTasks)
	{
		CachedTotalTasks = Status.Y;
		CachedCompletedTasks = Status.X;

		OnTasksChanged.Broadcast(CachedCompletedTasks, CachedTotalTasks);
	}
}

void UNProcGenOperation::FinishBuild(const TSharedRef<FNProcGenOperationSharedContext> SharedContext)
{
	if (Owner != nullptr)
	{
		Owner->OnOperationFinished(this, SharedContext);
	}
	
	for (const auto Component : Context->InputComponents)
	{
		Component->SetLastGenerationOperationKey(this->GetFName());
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
		Context->LockAndPreprocess(Owner->GetDefaultWorld());
		SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::ContextLocked);
	}
	
	if (Graph.IsValid())
	{
		Graph->ResetGraph();
		Graph.Reset();
	}
	
	// Output debug in our editor only
#if WITH_EDITOR	
	Context->OutputToLog(true);
#endif // WITH_EDITOR	
	
	// Build out our new graph
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::BuildingTaskGraph);
	Graph = MakeUnique<FNProcGenOperationTaskGraph>(this, Context.Get());
	
	// Add callback to tasks?
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::StartingTasks);
	Graph->UnlockTasks();
}

bool UNProcGenOperation::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}

void UNProcGenOperation::LockContext(UWorld* World)
{
	Context->LockAndPreprocess(World);
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::ContextLocked);
}

