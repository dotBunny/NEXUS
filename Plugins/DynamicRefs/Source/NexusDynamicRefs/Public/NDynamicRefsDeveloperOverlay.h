// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRef.h"
#include "Widgets/NDeveloperOverlay.h"
#include "NDynamicRefsDeveloperOverlay.generated.h"

class UNDynamicRefObject;
enum class ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "DynamicRefs Developer Overlay", BlueprintType, Abstract)
class NEXUSDYNAMICREFS_API UNDynamicRefsDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World, bool bClearItems = true);

protected:	
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> References;
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

private:
	void UpdateBanner() const;
	
	TMap<UWorld*, FDelegateHandle> OnAddedDelegates;
	TMap<UWorld*, FDelegateHandle> OnAddedByNameDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedByNameDelegates;
	
	TMap<ENDynamicRef, TObjectPtr<UNDynamicRefObject>> DynamicRefObjects;
	TMap<FName, TObjectPtr<UNDynamicRefObject>> NamedObjects;
	
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
};