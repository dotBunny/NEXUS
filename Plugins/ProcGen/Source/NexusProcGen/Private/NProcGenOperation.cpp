// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenOperation.h"

#include "NProcGenMinimal.h"
#include "NProcGenRegistry.h"
#include "Organ/NOrganComponent.h"
#include "Generation/NProcGenTaskGraph.h"

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

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<UNOrganComponent*>& Components, FNProcGenOperationSettings& OperationSettings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(OperationSettings);
	for (const auto Component : Components)
	{
		Operation->AddToContext(Component);
	}
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *OperationSettings.Seed)
	return Operation;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, FNProcGenOperationSettings& OperationSettings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(OperationSettings);
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Objects);
	for (const auto Organ : OrganComponents)
	{
		Operation->AddToContext(Organ);
	}
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *OperationSettings.Seed)
	return Operation;
}

UNProcGenOperation* UNProcGenOperation::CreateInstance(UNOrganComponent* BaseComponent, FNProcGenOperationSettings& OperationSettings)
{
	UNProcGenOperation* Operation = NewObject<UNProcGenOperation>();
	Operation->ApplySettings(OperationSettings);
	Operation->AddToContext(BaseComponent);
	
	UE_LOG(LogNexusProcGen, Log, TEXT("Created new UNProcGenOperation(%s) with Seed(%s)"), *Operation->DisplayName.ToString(), *OperationSettings.Seed)
	return Operation;
}

void UNProcGenOperation::ApplySettings(FNProcGenOperationSettings& Settings)
{
	// Validate the Display Name, otherwise assign the operation name to it.
	if (Settings.DisplayName.IsEmpty())
	{
		Settings.DisplayName = FText::FromString(GetName());
	}
	// Copy so it is detached
	DisplayName = Settings.DisplayName;
	Context->SetOperationSettings(Settings);
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
	
	if (TaskGraph.IsValid())
	{
		TaskGraph->ResetGraph();
		TaskGraph.Reset();
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
	if (!TaskGraph.IsValid()) return;
	
	if (const FIntVector2 Status = TaskGraph->GetTaskStatus(); 
		Status.Y != CachedTotalTasks || Status.X != CachedCompletedTasks)
	{
		CachedTotalTasks = Status.Y;
		CachedCompletedTasks = Status.X;

		OnTasksChanged.Broadcast(CachedCompletedTasks, CachedTotalTasks);
	}
}

void UNProcGenOperation::FinishBuild(const TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext)
{
	bIsRunning = false;
	
	if (Owner != nullptr)
	{
		Owner->OnOperationFinished(this, TaskGraphContext);
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
	// Don't double run
	if (bIsRunning) return;
	
	// Cache our caller
	Owner = Caller;
	
	// Ensure that we have locked context and done the preprocessing.
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess(Owner->GetDefaultWorld());
		SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::ContextLocked);
	}
	
	if (TaskGraph.IsValid())
	{
		TaskGraph->ResetGraph();
		TaskGraph.Reset();
	}
	
	// Output debug in our editor only
#if WITH_EDITOR	
	Context->OutputToLog(true);
#endif // WITH_EDITOR	
	
	// Build out our new graph
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::BuildingTaskGraph);
	TaskGraph = MakeUnique<FNProcGenTaskGraph>(this, Context.Get());
	
	// Add callback to tasks?
	SetDisplayMessage(NEXUS::ProcGen::DisplayMessages::StartingTasks);
	
	bIsRunning = true;
	TaskGraph->UnlockTasks();
	
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

