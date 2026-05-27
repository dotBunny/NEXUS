// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INAssemblyOperationOwner.h"
#include "NWorldAssemblyMinimal.h"
#include "NAssemblyOperationSettings.h"
#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Assembly/NAssemblyTaskGraph.h"
#include "NAssemblyOperation.generated.h"


class FNAssemblyTaskGraphContext;
class UNCell;
class UNOrganComponent;

/**
 * Lifecycle states for a UNAssemblyOperation as it moves through registration, execution and teardown.
 */
UENUM(BlueprintType)
enum class ENWorldAssemblyOperationState : uint8
{
	None = 0,
	Registered = 1,
	Started = 2,
	Updated = 3,
	Finished = 4,
	Unregistered = 5
};

USTRUCT(BlueprintType)
struct FNAssemblyOperationResult
{
	GENERATED_BODY()
	
	bool bSuccess = false;
	bool bWarning = false;
	
	FText Title;
	FText Message;
	
	float Duration = 0.0f;
	int CreatedCells;
};

/** Broadcast when the display message shown in UI changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssemblyOperationDisplayMessageChanged, const FString&, NewMessage);
/** Broadcast when the completed/total task counts change. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssemblyOperationTasksChanged, const int, CompletedTasks, const int, TotalTasks);
/** Broadcast when the overall completion percentage changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNWorldAssemblyOperationPercentageChanged, const float, NewPercentage);

/**
 * A single procedural-generation operation: the unit of work scheduled on the World Assembly subsystem.
 *
 * Owns the generation context (participating organs, cells, tissue), the task graph that actually
 * executes the pass, and the per-operation settings (seed, level-instance behavior). Clients register
 * as the operation's owner via INAssemblyOperationOwner to receive lifecycle callbacks.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Assembly Operation")
class NEXUSWORLDASSEMBLY_API UNAssemblyOperation : public UObject
{
	friend class UNWorldAssemblySubsystem;
	friend class UNWorldAssemblyEditorSubsystem;
	friend class FNWorldAssemblyEdMode;

	friend class FNAssemblyTaskGraph;
	friend struct FNOrganGeneratorFinalizeTask;
	friend struct FNAssemblyFinalizeTask;

	GENERATED_BODY()

	explicit UNAssemblyOperation(const FObjectInitializer& ObjectInitializer);
public:
#if !UE_BUILD_SHIPPING
	FString OutputReportToFile()
	{
		FString OutputFile = FPaths::Combine(FPaths::ProjectLogDir(),
					FString::Printf(TEXT("NEXUS_WorldAssembly_%s.md"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
		TArray<FString> Output = Report.GetReportLines(ENReportOutputFormat::Markdown);
		
		Async(EAsyncExecution::TaskGraph,
			[Output = MoveTemp(Output), OutputFile]()
			{
				FFileHelper::SaveStringArrayToFile(Output, *OutputFile, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Silent);
				UE_LOG(LogNexusWorldAssembly, Log, TEXT("Report written to %s."), *OutputFile);
			});
		return OutputFile;
	}
	void OutputReportToLog()
	{
		TArray<FString> Output = Report.GetReportLines(ENReportOutputFormat::PlainText);
		for (int i = 0; i < Output.Num(); i++)
		{
			UE_LOG(LogNexusWorldAssembly, Log, TEXT("%s"), *Output[i]);
		}
	}
	FNReport* GetReport() { return &Report; }
#endif // !UE_BUILD_SHIPPING
	
	/**
	 * Convert an operation state enum into the stable string used by logs and the developer overlay.
	 * @param State The state to convert.
	 * @return Display string from NEXUS::WorldAssembly::States matching the enum value.
	 */
	static FString GetStringFromState(const ENWorldAssemblyOperationState State)
	{
		switch (State)
		{
			using enum ENWorldAssemblyOperationState;
		case Registered:
			return NEXUS::WorldAssembly::States::Registered;
		case Started:
			return NEXUS::WorldAssembly::States::Started;
		case Updated:
			return NEXUS::WorldAssembly::States::Updated;
		case Finished:
			return NEXUS::WorldAssembly::States::Finished;
		case Unregistered:
			return NEXUS::WorldAssembly::States::Unregistered;
		default:
			return NEXUS::WorldAssembly::States::None;
		}		
	}
	
	/**
	 * Build an operation from an explicit set of organ components.	 
	 * @param Components Components participating in the generation context. It is important that these have been presorted by Identifier to ensure determinism.
	 * @param OperationSettings Per-operation settings to apply (seed, level-instance behavior).
	 * @return The configured operation ready to be registered with a World Assembly subsystem.
	 */
	static UNAssemblyOperation* CreateInstance(const TArray<UNOrganComponent*>& Components, FNAssemblyOperationSettings& OperationSettings);
	/**
	 * Build an operation from a weak-pointer set of objects; unresolved or non-organ entries are skipped.
	 * @param Objects Weak references to organs to pull into the context. It is important that these have been presorted by their components Identifier to ensure determinism.
	 * @param OperationSettings Per-operation settings to apply.
	 */
	static UNAssemblyOperation* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, FNAssemblyOperationSettings& OperationSettings);
	/**
	 * Build an operation seeded from a single base organ component.
	 * @param BaseComponent Starting organ; its dependencies are discovered during LockContext.
	 * @param OperationSettings Per-operation settings to apply.
	 */
	static UNAssemblyOperation* CreateInstance(UNOrganComponent* BaseComponent, FNAssemblyOperationSettings& OperationSettings);

	/**
	 * Merge the supplied settings into the operation, updating seed/display-name/level-instance behavior.
	 * @param Settings Settings to copy in.
	 */
	void ApplySettings(FNAssemblyOperationSettings& Settings);

	/** Clear all transient operation state so the operation can be re-used for another build. */
	void Reset() const;

	/**
	 * Start build process.
	 * @param Caller Owning system that will get callbacks for building process.
	 * @param CallerObject The Caller as a UObject, used for weak-reference lifetime validation.
	 */
	void StartBuild(INAssemblyOperationOwner* Caller, UObject* CallerObject);

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
	TArray<TArray<TObjectPtr<UNOrganComponent>>>& GetGenerationOrder() const
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
	FOnAssemblyOperationDisplayMessageChanged OnDisplayMessageChanged;

	/** Broadcast whenever the completed/total task counts change. */
	UPROPERTY(BlueprintAssignable)
	FOnAssemblyOperationTasksChanged OnTasksChanged;

	/** @return Cached (completed, total) task counts captured since the last OnTasksChanged broadcast. */
	FIntVector2 GetCachedTaskStatusCounts() const { return FIntVector2(CachedCompletedTasks, CachedTotalTasks); }
	/** @return The unique 32-bit identifier assigned to this operation at creation time. */
	uint32 GetTicket() const { return Ticket; }
	
	FNAssemblyOperationResult GetResult() const;
	
	void Cancel();
	
protected:
	void Tick();
	void FinishBuild(TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext);
	FNAssemblyTaskGraph* GetTaskGraph() const { return TaskGraph.Get(); }
	void TearDownOperation();

private:
	/** Monotonically increasing ticket source used to assign a unique identifier to each operation. */
	static uint32 NextTicket;

	/** Current owner receiving lifecycle callbacks; raw pointer because the owner may be a non-UObject type. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	INAssemblyOperationOwner* Owner = nullptr;
	/** Weak reference to the owner as a UObject, used to detect owner destruction before calling through Owner. */
	TWeakObjectPtr<UObject> OwnerWeakRef;
	/** Task graph executing the current build. */
	TUniquePtr<FNAssemblyTaskGraph> TaskGraph;
	/** Context describing the organs/cells/tissue participating in this pass. */
	TUniquePtr<FNAssemblyOperationContext> Context;

	/** Hard references keeping UNCell assets alive while the operation is in flight. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNCell>> ReferencedCells;

	/** true while StartBuild has been called and the task graph has not yet finished. */
	bool bIsRunning;
	
	/** Mirrors Context->IsLocked(); cached here to avoid reaching through the TUniquePtr from const callers. */
	bool bIsContextLocked;
	/** Human-friendly label for UI. */
	FText DisplayName;
	/** Current progress message for UI. */
	FString DisplayMessage;

	/** Most recent total-task count broadcast to OnTasksChanged. */
	int32 CachedTotalTasks = 0;
	/** Most recent completed-task count broadcast to OnTasksChanged. */
	int32 CachedCompletedTasks = 0;

	/** Unique identifier for this operation, allocated from NextTicket. */
	uint32 Ticket;

#if !UE_BUILD_SHIPPING
	FNReport Report;
#endif // !UE_BUILD_SHIPPING
};
