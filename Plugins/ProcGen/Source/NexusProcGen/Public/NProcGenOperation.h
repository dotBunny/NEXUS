// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INProcGenOperationOwner.h"
#include "NProcGenMinimal.h"
#include "NProcGenOperationSettings.h"
#include "Generation/NProcGenOperationContext.h"
#include "Generation/NProcGenTaskGraph.h"
#include "NProcGenOperation.generated.h"


class FNProcGenTaskGraphContext;
class UNOrganComponent;

/**
 * Lifecycle states for a UNProcGenOperation as it moves through registration, execution and teardown.
 */
UENUM(BlueprintType)
enum class ENProcGenOperationState : uint8
{
	None = 0,
	Registered = 1,
	Started = 2,
	Updated = 3,
	Finished = 4,
	Unregistered = 5
};

/** Broadcast when the display message shown in UI changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNProcGenOperationDisplayMessageChanged, const FString&, NewMessage);
/** Broadcast when the completed/total task counts change. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNProcGenOperationTasksChanged, const int, CompletedTasks, const int, TotalTasks);
/** Broadcast when the overall completion percentage changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNProcGenOperationPercentageChanged, const float, NewPercentage);

/**
 * A single procedural-generation operation: the unit of work scheduled on the ProcGen subsystem.
 *
 * Owns the generation context (participating organs, cells, tissue), the task graph that actually
 * executes the pass, and the per-operation settings (seed, level-instance behaviour). Clients register
 * as the operation's owner via INProcGenOperationOwner to receive lifecycle callbacks.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Operation")
class NEXUSPROCGEN_API UNProcGenOperation : public UObject
{
	friend class UNProcGenSubsystem;
	friend class UNProcGenEditorSubsystem;
	friend class FNProcGenEdMode;

	friend struct FNOrganGeneratorFinalizeTask;
	friend struct FNProcGenFinalizeTask;

	GENERATED_BODY()

	explicit UNProcGenOperation(const FObjectInitializer& ObjectInitializer);

public:
	/**
	 * Convert an operation state enum into the stable string used by logs and the developer overlay.
	 * @param State The state to convert.
	 * @return Display string from NEXUS::ProcGen::States matching the enum value.
	 */
	static FString GetStringFromState(const ENProcGenOperationState State)
	{
		switch (State)
		{
			using enum ENProcGenOperationState;
		case Registered:
			return NEXUS::ProcGen::States::Registered;
		case Started:
			return NEXUS::ProcGen::States::Started;
		case Updated:
			return NEXUS::ProcGen::States::Updated;
		case Finished:
			return NEXUS::ProcGen::States::Finished;
		case Unregistered:
			return NEXUS::ProcGen::States::Unregistered;
		default:
			return NEXUS::ProcGen::States::None;
		}		
	}
	
	/**
	 * Build an operation from an explicit set of organ components.
	 * @param Components Components participating in the generation context.
	 * @param OperationSettings Per-operation settings to apply (seed, level-instance behaviour).
	 * @return The configured operation ready to be registered with a ProcGen subsystem.
	 */
	static UNProcGenOperation* CreateInstance(const TArray<UNOrganComponent*>& Components, FNProcGenOperationSettings& OperationSettings);
	/**
	 * Build an operation from a weak-pointer set of objects; unresolved or non-organ entries are skipped.
	 * @param Objects Weak references to organs to pull into the context.
	 * @param OperationSettings Per-operation settings to apply.
	 */
	static UNProcGenOperation* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, FNProcGenOperationSettings& OperationSettings);
	/**
	 * Build an operation seeded from a single base organ component.
	 * @param BaseComponent Starting organ; its dependencies are discovered during LockContext.
	 * @param OperationSettings Per-operation settings to apply.
	 */
	static UNProcGenOperation* CreateInstance(UNOrganComponent* BaseComponent, FNProcGenOperationSettings& OperationSettings);


	virtual void BeginDestroy() override;

	/**
	 * Merge the supplied settings into the operation, updating seed/display-name/level-instance behaviour.
	 * @param Settings Settings to copy in.
	 */
	void ApplySettings(FNProcGenOperationSettings& Settings);

	/** Clear all transient operation state so the operation can be re-used for another build. */
	void Reset() const;

	/**
	 * Start build process.
	 * @param Caller Owning system that will get callbacks for building process
	 */
	void StartBuild(INProcGenOperationOwner* Caller);
	/**
	 * Append an organ component to the generation context.
	 * @param Component Organ to include.
	 * @return true if the component was added; false if the context is already locked.
	 */
	bool AddToContext(UNOrganComponent* Component) const;

	/** @return true if the context has been locked and the dependency order resolved. */
	bool IsLocked() const { return Context->IsLocked(); }
	/** @return true if a build is currently running for this operation. */
	bool IsRunning() const { return bIsRunning; }

	/** Lock the context added to the generator and figure out all the generation dependencies and order. */
	void LockContext(UWorld* World);

	/** @return The topologically ordered batches of organs determined during LockContext. */
	TArray<TArray<UNOrganComponent*>>& GetGenerationOrder() const
	{
		return Context->GenerationOrder;
	}

	/** @return Human-friendly label for UI display. */
	const FText& GetDisplayName() const { return DisplayName; }
	/** @return Current progress/status message shown in UI and logs. */
	const FString& GetDisplayMessage() const { return DisplayMessage; }
	/** Update the display message and broadcast OnDisplayMessageChanged to any listeners. */
	void SetDisplayMessage(FString NewDisplayMessage);

	/** Broadcast whenever the display message changes. */
	UPROPERTY(BlueprintAssignable)
	FOnNProcGenOperationDisplayMessageChanged OnDisplayMessageChanged;

	/** Broadcast whenever the completed/total task counts change. */
	UPROPERTY(BlueprintAssignable)
	FOnNProcGenOperationTasksChanged OnTasksChanged;

	/** @return Cached (completed, total) task counts captured since the last OnTasksChanged broadcast. */
	FIntVector2 GetCachedTasksStatus() const { return FIntVector2(CachedCompletedTasks, CachedTotalTasks); }
	/** @return The unique 32-bit identifier assigned to this operation at creation time. */
	uint32 GetTicket() const { return Ticket; }
protected:
	void Tick();
	void FinishBuild(TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext);
	FNProcGenTaskGraph* GetTaskGraph() const { return TaskGraph.Get(); }

private:
	/** Monotonically increasing ticket source used to assign a unique identifier to each operation. */
	static uint32 NextTicket;

	/** Current owner receiving lifecycle callbacks; raw pointer because the owner may be a non-UObject subsystem. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	INProcGenOperationOwner* Owner = nullptr;
	/** Task graph executing the current build. */
	TUniquePtr<FNProcGenTaskGraph> TaskGraph;
	/** Context describing the organs/cells/tissue participating in this pass. */
	TUniquePtr<FNProcGenOperationContext> Context;

	/** true while StartBuild has been called and the task graph has not yet finished. */
	bool bIsRunning;
	/** Mirrors Context->IsLocked(); cached here to avoid reaching through the TUniquePtr from const callers. */
	bool bIsContextLocked;
	/** Human-friendly label for UI. */
	FText DisplayName;
	/** Current progress message for UI. */
	FString DisplayMessage;

	/** Most recent total-task count broadcast to OnTasksChanged. */
	int CachedTotalTasks = 0;
	/** Most recent completed-task count broadcast to OnTasksChanged. */
	int CachedCompletedTasks = 0;

	/** Unique identifier for this operation, allocated from NextTicket. */
	uint32 Ticket;
};
