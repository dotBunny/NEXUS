// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/NDeveloperOverlay.h"
#include "NProcGenDeveloperOverlay.generated.h"

enum class ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

/**
 * Developer overlay widget that lists every live ProcGen operation and its current state.
 *
 * Subscribes to the registry's state-change broadcast so the list refreshes as operations
 * start, progress, and complete. Intended for diagnostics; not shipped to end users.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen Developer Overlay", BlueprintType, Abstract)
class NEXUSPROCGEN_API UNProcGenDeveloperOverlay : public UNDeveloperOverlay
{

	GENERATED_BODY()

	/** Bound to the registry broadcast; refreshes the operations list as operations change state. */
	void OnOperationStatusChanged(UNProcGenOperation* Operation, const ENProcGenOperationState NewState);

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
	FText NoOperationsFoundText = NSLOCTEXT("NexusProcGen", "NoOperationsFound", "No Operations Found");

	/** Handle for the registry state-change subscription. */
	FDelegateHandle OperationsStatusChangedDelegateHandle;

	/** @return true if the operation with the given name should appear in the overlay (filters internal/system operations). */
	static bool ShouldShowOperation(const FName& OperationName);
};
