// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyOperation.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "Organ/NOrganComponent.h"
#include "Assembly/NAssemblyTaskGraph.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"

int32 UNAssemblyOperation::NextTicket = 1;

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

	if (Owner != nullptr && OwnerWeakRef.IsValid())
	{
		Owner->OnOperationDestroyed(this);
	}
	Owner = nullptr;
	OwnerWeakRef.Reset();
	
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

	ReferencedCells.Empty();

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

void UNAssemblyOperation::SetStatusMessage(FString NewStatusMessage)
{
	if (!StatusMessage.Equals(NewStatusMessage))
	{
		UE_LOG(LogNexusWorldAssembly, Log, TEXT("[%s] StatusMessage('%s' to '%s')"), *DisplayName.ToString(), *StatusMessage,  *NewStatusMessage);
		StatusMessage = NewStatusMessage;
		OnStatusMessageChanged.Broadcast(StatusMessage);
	}
}

UE_DISABLE_OPTIMIZATION

void UNAssemblyOperation::UpdateCachedResult()
{
	Result.Reset();
	
	// Early out cause it's still running
	if (bIsRunning)
	{
		Result.bSuccess = false;
		Result.bWarning = true;
		Result.Title = FText::FromString("Operation In-Flight");
		Result.Message = FText::FromString("The Assembly Operation has not finished running.");
		return;
	}
	
	Result.Title = FText::FromString("Assembly Operation Finished");
	
	// Build our state map
	TMap<FGuid, bool> OrganResults;
	int32 CreatedCells = 0;
	int32 AcceptableFails = 0;
	
	for (int i = 0; i < Context->InputComponents.Num(); i++)
	{
		UNOrganComponent* Organ = Context->InputComponents[i];
		FGuid OrganIdentifier = Organ->Identifier;
		if (Organ == nullptr) continue;
		
		// Add default fail
		OrganResults.Add(OrganIdentifier, false);
		const bool bIsRequired = Organ->bRequired;
		
		// Check for cell counts
		const TSharedRef<FNAssemblyTaskGraphContext> GraphContext = TaskGraph->GetContext();
		if (GraphContext->OrganCellCount.Contains(OrganIdentifier))
		{
			int32 CellCountGenerated = GraphContext->OrganCellCount[OrganIdentifier];
			CreatedCells += CellCountGenerated;
			
			if (Organ->MinimumCellCount > 0 && CellCountGenerated < Organ->MinimumCellCount)
			{
				if (!bIsRequired)
				{
					AcceptableFails++;
					OrganResults[OrganIdentifier] = true;
				}
				continue;
			}
			
			if (Organ->MaximumCellCount > 0 && CellCountGenerated > Organ->MaximumCellCount)
			{
				if (!bIsRequired)
				{
					AcceptableFails++;
					OrganResults[OrganIdentifier] = true;
				}
				continue;
			}
			
			// Made it through the gauntlet, it is a good assembly
			OrganResults[OrganIdentifier] = true;
		}
		else
		{
			// Not in the graph 
			if (!bIsRequired)
			{
				AcceptableFails++;
				OrganResults[OrganIdentifier] = true;
			}

		}
	}
	
	
	// Give some number
	Result.CreatedCells = CreatedCells;
	
	// Iterate for any fails
	bool bAssemblySuccess = true;
	for (auto Itr = OrganResults.CreateConstIterator(); Itr; ++Itr)
	{
		if (!Itr.Value())
		{
			bAssemblySuccess = false;
			break;
		}
	}
	Result.bSuccess = bAssemblySuccess;
	
#if !UE_BUILD_SHIPPING
	Result.Duration = TaskGraph->N_ASSEMBLY_ANALYTICS_MEMBER_PTR->GetTotalDuration();
	Result.Message = FText::FromString(FString::Printf(TEXT("%f ms / %i Cells / %i Skips"), Result.Duration, CreatedCells, AcceptableFails));
#else
	Result.Message = FText::FromString(FString::Printf(TEXT("%i Cells / %i Skips"), CreatedCells, AcceptableFails));
#endif
}

UE_ENABLE_OPTIMIZATION


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

	// Drain any progress message published by the (possibly worker-thread) tasks and broadcast it on the game thread.
	FString PendingMessage;
	if (TaskGraph->ConsumeStatusMessage(PendingMessage))
	{
		SetStatusMessage(MoveTemp(PendingMessage));
	}

	// Drain the per-stage progress channels into their child-list view-models.
	DrainStatusChannels();
}

void UNAssemblyOperation::DrainStatusChannels()
{
	if (!TaskGraph.IsValid()) return;

	// Drain the (possibly worker-thread authored) channel deltas and forward them to the registry, which
	// routes them to this operation's row. The row owns the resulting view-models and their lifecycle.
	TArray<FNStatusChannelUpdate> Changes;
	if (TaskGraph->ConsumeChannelUpdates(Changes))
	{
		FNWorldAssemblyRegistry::NotifyOperationChannelsChanged(this, Changes);
	}
}

void UNAssemblyOperation::FinishBuild(const TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	bIsRunning = false;
	UpdateCachedResult();
	
	// Add one last update to subscribers for task updates
	const FIntVector2 Status = TaskGraph->GetTaskStatus();
	OnTasksChanged.Broadcast(Status.X, Status.Y);

	// Flush any final display message the tasks published before the poll loop stopped.
	FString PendingMessage;
	if (TaskGraph->ConsumeStatusMessage(PendingMessage))
	{
		SetStatusMessage(MoveTemp(PendingMessage));
	}

	// Flush any final channel states (e.g. close-to-100%) the tasks published before the poll loop stopped.
	DrainStatusChannels();
	
	if (Owner != nullptr && OwnerWeakRef.IsValid())
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

void UNAssemblyOperation::StartBuild(INAssemblyOperationOwner* Caller, UObject* CallerObject)
{
	// Don't double run
	if (bIsRunning) return;

	// Cache our caller
	Owner = Caller;
	OwnerWeakRef = CallerObject;
	
	// Ensure that we have locked context and done the preprocessing.
	if (!Context->IsLocked())
	{
		Context->LockAndPreprocess(Owner->GetDefaultWorld());
		SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::ContextLocked);
	}

	ReferencedCells.Empty();

	if (TaskGraph.IsValid())
	{
		TaskGraph->TearDownGraph();
		TaskGraph.Reset();
	}

#if !UE_BUILD_SHIPPING
	Context->AddToReport(GetReport(), true);
#endif // !UE_BUILD_SHIPPING	
	
	// Build out our new graph
	SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::BuildingTaskGraph);
	TaskGraph = MakeUnique<FNAssemblyTaskGraph>(this, Context.Get());
	
	// Add callback to tasks?
	SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::StartingTasks);
	
	bIsRunning = true;
	TaskGraph->UnlockTasks();
	
	UpdateCachedResult();
	
}

bool UNAssemblyOperation::AddToContext(UNOrganComponent* Component) const
{
	return Context->AddOrganComponent(Component);
}

void UNAssemblyOperation::LockContext(UWorld* World)
{
	Context->LockAndPreprocess(World);
	SetStatusMessage(NEXUS::WorldAssembly::StatusMessage::ContextLocked);
}

