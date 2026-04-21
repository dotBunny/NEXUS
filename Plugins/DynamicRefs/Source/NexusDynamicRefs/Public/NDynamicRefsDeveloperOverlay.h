// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRef.h"
#include "Widgets/NDeveloperOverlay.h"
#include "NDynamicRefsDeveloperOverlay.generated.h"

class UNDynamicRefObject;
class UNListView;

/**
 * Developer overlay widget that lists every live ENDynamicRef slot and FName bucket in the active world(s).
 * @see <a href="https://nexus-framework.com/docs/plugins/dynamic-references/developer-overlay/">UNDynamicRefsDeveloperOverlay</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | DynamicRefs Developer Overlay", BlueprintType, Abstract)
class NEXUSDYNAMICREFS_API UNDynamicRefsDeveloperOverlay : public UNDeveloperOverlay
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicRefButtonDelegate, UObject*, TargetObject);

	GENERATED_BODY()

	void Bind(UWorld* World);
	void Unbind(const UWorld* World);

public:
	/** Fired when a row's button is clicked; typically used to focus/select the target object in the editor. */
	UPROPERTY(BlueprintAssignable)
	FOnDynamicRefButtonDelegate OnButtonClicked;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

	/** Header displayed above the ENDynamicRef list (hidden when empty). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UBorder> DynamicReferencesHeader;

	/** List view displaying all populated ENDynamicRef slots. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> DynamicReferences;

	/** Header displayed above the FName list (hidden when empty). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UBorder> NamedReferencesHeader;

	/** List view displaying all populated FName buckets. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> NamedReferences;

	/** Cache of wrapper objects keyed by ENDynamicRef slot. */
	UPROPERTY()
	TMap<TEnumAsByte<ENDynamicRef>, TObjectPtr<UNDynamicRefObject>> DynamicRefObjects;

	/** Cache of wrapper objects keyed by FName bucket. */
	UPROPERTY()
	TMap<FName, TObjectPtr<UNDynamicRefObject>> NamedObjects;

private:
	void UpdateBanner() const;

	void AddListItem(FName Name, UObject* Object);
	void AddListItem(ENDynamicRef DynamicRef, UObject* Object);
	void RemoveListItem(FName Name, UObject* Object);
	void RemoveListItem(ENDynamicRef DynamicRef, UObject* Object);

	// ReSharper disable CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UWorld*, FDelegateHandle> OnAddedDelegates;
	TMap<UWorld*, FDelegateHandle> OnAddedByNameDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedDelegates;
	TMap<UWorld*, FDelegateHandle> OnRemovedByNameDelegates;
	// ReSharper restore CppUE4ProbableMemoryIssuesWithUObjectsInContainer

	/** Placeholder text shown when no references are present. */
	FText NoReferencesFoundText = NSLOCTEXT("NexusDynamicRefs", "NoReferencesFound", "No References Found");
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
};