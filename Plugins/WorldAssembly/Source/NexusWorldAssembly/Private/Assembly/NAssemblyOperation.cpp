// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyOperation.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "Organ/NOrganComponent.h"
#include "Assembly/NAssemblyTaskGraph.h"

uint32 UNAssemblyOperation::NextTicket = 1;

UNAssemblyOperation::UNAssemblyOperation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Only do things when it is NOT the CDO
	if (!this->IsTemplate())
	{
		Ticket = NextTicket++;
		Context = MakeUnique<FNAssemblyOperationContext>(Ticket);
		this->AddToRoot();
		
		FNWorldAssemblyRegistry::RegisterOperation(this);
	}
}

UNAssemblyOperation* UNAssemblyOperation::CreateInstance(const TArray<UNOrganComponent*>& Components, FNAssemblyOperationSettings& OperationSettings)
{
	UNAssemblyOperation* Operation = NewObject<UNAssemblyOperation>();
	Operation->ApplySettings(OperationSettings);
	for (const auto Component : Components)
	{
		Operation->AddToContext(Component);
	}
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Created new UNWorldAssemblyOperation(%s) with Ticket(%i) and Seed(%s)"),
		*Operation->DisplayName.ToString(), Operation->GetTicket(), *OperationSettings.Seed);
	return Operation;
}

UNAssemblyOperation* UNAssemblyOperation::CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, FNAssemblyOperationSettings& OperationSettings)
{
	UNAssemblyOperation* Operation = NewObject<UNAssemblyOperation>();
	Operation->ApplySettings(OperationSettings);
	TArray<UNOrganComponent*> OrganComponents = UNOrganComponent::GetOrganComponents(Objects);
	for (const auto Organ : OrganComponents)
	{
		Operation->AddToContext(Organ);
	}
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Created new UNWorldAssemblyOperation(%s) with GUID(%i) and Seed(%s)"),
		*Operation->DisplayName.ToString(), Operation->GetTicket(), *OperationSettings.Seed);
	return Operation;
}

UNAssemblyOperation* UNAssemblyOperation::CreateInstance(UNOrganComponent* BaseComponent, FNAssemblyOperationSettings& OperationSettings)
{
	UNAssemblyOperation* Operation = NewObject<UNAssemblyOperation>();
	Operation->ApplySettings(OperationSettings);
	Operation->AddToContext(BaseComponent);
	
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("Created new UNWorldAssemblyOperation(%s) with Ticket(%i) and Seed(%s)"),
		*Operation->DisplayName.ToString(), Operation->GetTicket(), *OperationSettings.Seed);
	return Operation;
}

void UNAssemblyOperation::TearDownOperation()
{
	// Don't do anything for CDO
	if (this->IsTemplate())
	{
		return;
	}

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
		TaskGraph->TearDownGraph();
		TaskGraph.Reset();
	}
	
	FNWorldAssemblyRegistry::UnregisterOperation(this);
	RemoveFromRoot();
	MarkAsGarbage();
}

void UNAssemblyOperation::ApplySettings(FNAssemblyOperationSettings& Settings)
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

void UNAssemblyOperation::Reset() const
{
	if (Context != nullptr)
	{
		Context->ResetContext();
	}
}

void UNAssemblyOperation::SetDisplayMessage(FString NewDisplayMessage)
{
	if (!DisplayMessage.Equals(NewDisplayMessage))
	{
		UE_LOG(LogNexusWorldAssembly, Log, TEXT("[%s] DisplayMessage('%s' to '%s')"), *DisplayName.ToString(), *DisplayMessage,  *NewDisplayMessage);
		DisplayMessage = NewDisplayMessage;
		OnDisplayMessageChanged.Broadcast(DisplayMessage);
	}
}

FNAssemblyOperationResult UNAssemblyOperation::GetResult() const
{
	FNAssemblyOperationResult Result;
	
	// Early out cause its still running
	if (bIsRunning == true)
	{
		Result.bSuccess = false;
		Result.bWarning = true;
		Result.Title = FText::FromString("Operation In-Flight");
		Result.Message = FText::FromString("The Assembly Operation has not finished running.");
		return MoveTemp(Result);
	}
	
	Result.Title = FText::FromString("Assembly Operation Finished");
	
	// Fill with analytics
#if !UE_BUILD_SHIPPING
	Result.CreatedCells = TaskGraph->N_ASSEMBLY_ANALYTICS_MEMBER_PTR->GetSpawnedCellProxiesCount();
	Result.Duration = TaskGraph->N_ASSEMBLY_ANALYTICS_MEMBER_PTR->GetTotalDuration();
	Result.bSuccess = Result.CreatedCells > 0; // not great but it's a start
	Result.Message = FText::FromString(FString::Printf(TEXT("%f ms / %i Cells."), Result.Duration, Result.CreatedCells));
#else
	Result.bSuccess = true;
#endif
	

	return MoveTemp(Result);
}

void UNAssemblyOperation::Cancel()
{
	if (TaskGraph.IsValid())
	{
		TaskGraph->Cancel();
	}

	TearDownOperation();
}

void UNAssemblyOperation::Tick()
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

void UNAssemblyOperation::FinishBuild(const TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	bIsRunning = false;
	
	// Add one last update to subscribers for task updates
	const FIntVector2 Status = TaskGraph->GetTaskStatus();
	OnTasksChanged.Broadcast(Status.X, Status.Y);
	
	if (Owner != nullptr)
	{
		Owner->OnOperationFinished(this, TaskGraphContext);
	}
	
	for (const auto Component : Context->InputComponents)
	{
		Component->SetLastOperationTicket(GetTicket());
	}
	
	// Were going to delete this object
	TearDownOperation();
}

void UNAssemblyOperation::StartBuild(INAssemblyOperationOwner* Caller)
{
	// Don't double run
	if (bIsRunning) return;
	
	// Cache our caller
	Owner = Caller;
	
	// Ensure that we have locked context and done the preprocessing.
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess(Owner->GetDefaultWorld());
		SetDisplayMessage(NEXUS::WorldAssembly::DisplayMessages::ContextLocked);
	}
	
	if (TaskGraph.IsValid())
	{
		TaskGraph->TearDownGraph();
		TaskGraph.Reset();
	}

#if !UE_BUILD_SHIPPING
	Context->AddToReport(GetReport(), true);
#endif // !UE_BUILD_SHIPPING	
	
	// Build out our new graph
	SetDisplayMessage(NEXUS::WorldAssembly::DisplayMessages::BuildingTaskGraph);
	TaskGraph = MakeUnique<FNAssemblyTaskGraph>(this, Context.Get());
	
	// Add callback to tasks?
	SetDisplayMessage(NEXUS::WorldAssembly::DisplayMessages::StartingTasks);
	
	bIsRunning = true;
	TaskGraph->UnlockTasks();
	
}

bool UNAssemblyOperation::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}

void UNAssemblyOperation::LockContext(UWorld* World)
{
	Context->LockAndPreprocess(World);
	SetDisplayMessage(NEXUS::WorldAssembly::DisplayMessages::ContextLocked);
}

