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
	TObjectPtr<UBorder> DynamicReferencesHeader;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> DynamicReferences;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UBorder> NamedReferencesHeader;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> NamedReferences;
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

	UPROPERTY()
	TMap<TEnumAsByte<ENDynamicRef>, TObjectPtr<UNDynamicRefObject>> DynamicRefObjects;
	UPROPERTY()
	TMap<FName, TObjectPtr<UNDynamicRefObject>> NamedObjects;
private:
	void UpdateBanner() const;
	
	void AddListItem(FName Name, UObject* Object);
	void AddListItem(ENDynamicRef DynamicRef, UObject* Object);
	void RemoveListItem(FName Name, UObject* Object);
	void RemoveListItem(ENDynamicRef DynamicRef, UObject* Object);	
	
	TMap<UWorld*, FDelegateHandle> OnAddedDelegates;
	TMap<UWorld*, FDelegateHandle> OnAddedByNameDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedByNameDelegates;

	FText NoReferencesFoundText = NSLOCTEXT("NexusDynamicRefs", "NoReferencesFound", "No References Found");
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
};