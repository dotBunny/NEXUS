// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/NDeveloperOverlay.h"
#include "NWorldAssemblyDeveloperOverlay.generated.h"

enum class ENWorldAssemblyOperationState : uint8;
class UNAssemblyOperation;
class UNListView;
struct FNStatusChannelUpdate;

/**
 * Developer overlay widget that lists every live World Assembly operation and its current state.
 *
 * Subscribes to the registry's state-change broadcast so the list refreshes as operations
 * start, progress, and complete. Intended for diagnostics; not shipped to end users.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | World Assembly Developer Overlay", BlueprintType, Abstract)
class NEXUSWORLDASSEMBLY_API UNWorldAssemblyDeveloperOverlay : public UNDeveloperOverlay
{

	GENERATED_BODY()

	/** Bound to the registry broadcast; refreshes the operations list as operations change state. */
	void OnOperationStatusChanged(UNAssemblyOperation* Operation, const ENWorldAssemblyOperationState NewState);

	/** Bound to the registry broadcast; routes channel deltas to the operation's row for child-bar display. */
	void OnOperationChannelsChanged(UNAssemblyOperation* Operation, const TArray<FNStatusChannelUpdate>& Changes);

protected:

	//~UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//End UUserWidget

	/** List widget that displays one row per currently-known operation. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> OperationsList;

private:
	/** Swap the header banner between "no operations" and the live summary as the list changes. */
	void UpdateBanner() const;

	/** Banner text shown when no operations are currently known. */
	FText NoOperationsFoundText = NSLOCTEXT("NexusWorldAssembly", "NoOperationsFound", "No Operations Found");

	/** Handle for the registry state-change subscription. */
	FDelegateHandle OperationsStatusChangedDelegateHandle;

	/** Handle for the registry channels-changed subscription. */
	FDelegateHandle OperationChannelsChangedDelegateHandle;

	/** @return true if the operation with the given name should appear in the overlay (filters internal/system operations). */
	static bool ShouldShowOperation(const FName& OperationName);
};
