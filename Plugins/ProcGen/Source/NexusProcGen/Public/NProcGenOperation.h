// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Organ/NOrganGenerationContext.h"
#include "NProcGenOperation.generated.h"

class UNOrganComponent;
class FNOrganGeneratorTasks;

UCLASS(ClassGroup=(Nexus), DisplayName = "NOrgan Generator")
class NEXUSPROCGEN_API UNProcGenOperation : public UObject
{
	friend class UNProcGenSubsystem;
	friend class UNProcGenEditorSubsystem;
	friend struct FNOrganGeneratorFinalizeUnsafeTask;
	
	GENERATED_BODY()

	explicit UNProcGenOperation(const FObjectInitializer& ObjectInitializer);
	
public:
	static UNProcGenOperation* CreateInstance(const TArray<TWeakObjectPtr<UObject>>& Objects);
	static UNProcGenOperation* CreateInstance(const TArray<UNOrganComponent*>& Components);
	static UNProcGenOperation* CreateInstance(UNOrganComponent* BaseComponent);
	
	void Reset() const;
	void StartBuild(UObject* Caller);
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
};
