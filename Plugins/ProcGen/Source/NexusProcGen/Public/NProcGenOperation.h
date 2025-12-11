// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Organ/NOrganGenerationContext.h"
#include "NProcGenOperation.generated.h"

class UNOrganComponent;
class FNOrganGeneratorTasks;

UENUM(BlueprintType)
enum ENProcGenOperationState : uint8
{
	PGOS_None = 0		UMETA(DisplayName="None"),
	PGOS_Registered		UMETA(DisplayName="Registered"),
	PGOS_Started		UMETA(DisplayName="Started"),
	PGOS_Updated		UMETA(DisplayName="Updated"),
	PGOS_Finished		UMETA(DisplayName="Finished"),
	PGOS_Unregistered	UMETA(DisplayName="Unregistered")
};

UCLASS(ClassGroup=(Nexus), DisplayName = "NOrgan Generator")
class NEXUSPROCGEN_API UNProcGenOperation : public UObject
{
	friend class UNProcGenSubsystem;
	friend class UNProcGenEditorSubsystem;
	friend class FNProcGenEdMode;
	friend struct FNOrganGeneratorFinalizeUnsafeTask;
	
	GENERATED_BODY()

	explicit UNProcGenOperation(const FObjectInitializer& ObjectInitializer);
	
public:
	static UNProcGenOperation* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects, const FText& DisplayName = FText::GetEmpty());
	static UNProcGenOperation* CreateInstance(const TArray<UNOrganComponent*>& Components, const FText& DisplayName = FText::GetEmpty());
	static UNProcGenOperation* CreateInstance(UNOrganComponent* BaseComponent, const FText& DisplayName= FText::GetEmpty());
	
	void Reset() const;
	void StartBuild(UObject* Caller);
	bool AddToContext(UNOrganComponent* Component) const;
	bool IsLocked() const { return Context->IsLocked(); }
	
	
	int GetTaskCount();
	int GetCompletedTaskCount();
	
	/**
	 * Lock the context added to the generator and figure out all the generation dependencies and order.
	 */
	void LockContext();
	TArray<TArray<UNOrganComponent*>>& GetGenerationOrder() const
	{
		return Context->GenerationOrder;
	}
	
	const FText& GetDisplayName() const { return DisplayName; }
	
	// TODO: Do we expose this?
	
	
	

protected:
	virtual void BeginDestroy() override;
	void FinishBuild();
	FNOrganGeneratorTasks* GetTasks() const { return Tasks; }

	
private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	TObjectPtr<UObject> ExecuteCaller = nullptr;
	FNOrganGeneratorTasks* Tasks = nullptr;
	FNOrganGenerationContext* Context = nullptr;
	bool bIsContextLocked;
	FText DisplayName;

};
