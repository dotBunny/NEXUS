// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INProcGenOperationOwner.h"
#include "NProcGenMinimal.h"
#include "Math/NSeedGenerator.h"
#include "Generation/NProcGenOperationContext.h"
#include "NProcGenOperation.generated.h"

class UNOrganComponent;
class FNProcGenOperationTaskGraph;

UENUM(BlueprintType)
enum ENProcGenOperationState : uint8
{
	PGOS_None = 0			UMETA(DisplayName="None"),
	PGOS_Registered = 1		UMETA(DisplayName="Registered"),
	PGOS_Started = 2		UMETA(DisplayName="Started"),
	PGOS_Updated = 3		UMETA(DisplayName="Updated"),
	PGOS_Finished = 4		UMETA(DisplayName="Finished"),
	PGOS_Unregistered = 5	UMETA(DisplayName="Unregistered")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNProcGenOperationDisplayMessageChanged, const FString&, NewMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNProcGenOperationTasksChanged, const int, CompletedTasks, const int, TotalTasks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNProcGenOperationPercentageChanged, const float, NewPercentage);

UCLASS(ClassGroup = "NEXUS", DisplayName = "ProcGen Operation")
class NEXUSPROCGEN_API UNProcGenOperation : public UObject
{
	friend class UNProcGenSubsystem;
	friend class UNProcGenEditorSubsystem;
	friend class FNProcGenEdMode;
	
	friend struct FNOrganGeneratorFinalizeTask;
	friend struct FNProcGenOperationFinalizeTask;
	
	GENERATED_BODY()

	explicit UNProcGenOperation(const FObjectInitializer& ObjectInitializer);
	
public:
	static FString GetStringFromState(const ENProcGenOperationState State)
	{
		switch (State)
		{
		case PGOS_Registered:
			return NEXUS::ProcGen::States::Registered;
		case PGOS_Started:
			return NEXUS::ProcGen::States::Started;
		case PGOS_Updated:
			return NEXUS::ProcGen::States::Updated;
		case PGOS_Finished:
			return NEXUS::ProcGen::States::Finished;
		case PGOS_Unregistered:
			return NEXUS::ProcGen::States::Unregistered;
		default:
			return NEXUS::ProcGen::States::None;
		}		
	}
	
	static UNProcGenOperation* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, const FString& Seed = FNSeedGenerator::RandomFriendlySeed(), const FText& DisplayName = FText::GetEmpty());
	static UNProcGenOperation* CreateInstance(const TArray<UNOrganComponent*>& Components, const FString& Seed = FNSeedGenerator::RandomFriendlySeed(), const FText& DisplayName = FText::GetEmpty());
	static UNProcGenOperation* CreateInstance(UNOrganComponent* BaseComponent, const FString& Seed = FNSeedGenerator::RandomFriendlySeed(), const FText& DisplayName= FText::GetEmpty());
	
	void Reset() const;
	void StartBuild(INProcGenOperationOwner* Caller);
	
	void SetSeedOnContext(const FString& NewSeed) const;
	bool AddToContext(UNOrganComponent* Component) const;
	
	bool IsLocked() const { return Context->IsLocked(); }
	
	/**
	 * Lock the context added to the generator and figure out all the generation dependencies and order.
	 */
	void LockContext();
	TArray<TArray<UNOrganComponent*>>& GetGenerationOrder() const
	{
		return Context->GenerationOrder;
	}
	
	
	const FText& GetDisplayName() const { return DisplayName; }
	const FString& GetDisplayMessage() const { return DisplayMessage; }
	void SetDisplayMessage(FString NewDisplayMessage);
	
	UPROPERTY(BlueprintAssignable)
	FOnNProcGenOperationDisplayMessageChanged OnDisplayMessageChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnNProcGenOperationTasksChanged OnTasksChanged;

	FIntVector2 GetCachedTasksStatus() const { return FIntVector2(CachedCompletedTasks, CachedTotalTasks); }

protected:
	void Tick();
	virtual void BeginDestroy() override;
	void FinishBuild();
	FNProcGenOperationTaskGraph* GetGraph() const { return Graph.Get(); }
	
private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	INProcGenOperationOwner* Owner = nullptr;
	TSharedPtr<FNProcGenOperationTaskGraph> Graph;
	TSharedPtr<FNProcGenOperationContext> Context;
	bool bIsContextLocked;
	FText DisplayName;
	FString DisplayMessage;

	int CachedTotalTasks = 0;
	int CachedCompletedTasks = 0;
};
