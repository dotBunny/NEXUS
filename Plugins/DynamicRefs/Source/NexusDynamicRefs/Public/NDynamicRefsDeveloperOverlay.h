// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDynamicRef.h"
#include "Widgets/NDeveloperOverlay.h"
#include "NDynamicRefsDeveloperOverlay.generated.h"

class UBorder;
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

public:

	/** Subscribe to the dynamic-ref subsystem hosted by World so its add/remove events drive this overlay. */
	virtual void BindWorld(UWorld* World) override;
	/** Drop the subscription to the dynamic-ref subsystem hosted by World. */
	virtual void UnbindWorld(const UWorld* World) override;

	/** Fired when a row's button is clicked; typically used to focus/select the target object in the editor. */
	UPROPERTY(BlueprintAssignable)
	FOnDynamicRefButtonDelegate OnButtonClicked;

protected:

	//~UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//End UUserWidget

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
	/** Refresh the overlay banner's text based on whether any references are currently live. */
	void UpdateBanner() const;

	/**
	 * Drop wrapper rows whose tracked objects have all gone stale (destroyed without a matching remove).
	 * Weak references go invalid silently with no delegate, so this is polled each tick to keep the
	 * displayed list in sync with the live objects.
	 */
	void ReconcileStaleEntries();

	/** Add a row to the FName-keyed list for Object under Name. */
	void AddListItem(FName Name, UObject* Object);
	/** Add a row to the ENDynamicRef-keyed list for Object under DynamicRef. */
	void AddListItem(ENDynamicRef DynamicRef, UObject* Object);
	/** Remove Object's row from the FName-keyed list under Name. */
	void RemoveListItem(FName Name, UObject* Object);
	/** Remove Object's row from the ENDynamicRef-keyed list under DynamicRef. */
	void RemoveListItem(ENDynamicRef DynamicRef, UObject* Object);

	// ReSharper disable CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	/** Per-world handle for the subsystem's OnAdded (ENDynamicRef) delegate. */
	TMap<UWorld*, FDelegateHandle> OnAddedDelegates;
	/** Per-world handle for the subsystem's OnAddedByName (FName) delegate. */
	TMap<UWorld*, FDelegateHandle> OnAddedByNameDelegates;
	/** Per-world handle for the subsystem's OnRemoved (ENDynamicRef) delegate. */
	TMap<UWorld*, FDelegateHandle> OnRemovedDelegates;
	/** Per-world handle for the subsystem's OnRemovedByName (FName) delegate. */
	TMap<UWorld*, FDelegateHandle> OnRemovedByNameDelegates;
	// ReSharper restore CppUE4ProbableMemoryIssuesWithUObjectsInContainer

	/** Placeholder text shown when no references are present. */
	FText NoReferencesFoundText = NSLOCTEXT("NexusDynamicRefs", "NoReferencesFound", "No References Found");
};