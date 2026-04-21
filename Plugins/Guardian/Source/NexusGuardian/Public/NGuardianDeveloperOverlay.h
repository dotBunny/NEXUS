// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonNumericTextBlock.h"
#include "NGuardianSubsystem.h"
#include "Widgets/NDeveloperOverlay.h"
#include "NGuardianDeveloperOverlay.generated.h"

class UNDynamicRefObject;
class UNListView;

/**
 * Developer overlay widget that displays the Guardian subsystem's baseline, current count, and next threshold.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Guardian Developer Overlay", BlueprintType, Abstract)
class NEXUSGUARDIAN_API UNGuardianDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void Bind(UWorld* World);
	void Unbind(const UWorld* World);

protected:

	/** Container showing the object count trio (current / base / next threshold). */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ObjectCountBox;

	/** Current UObject count. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> ObjectCountNumber;

	/** Baseline UObject count captured via SetBaseline. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> BaseCountNumber;

	/** Value of the next threshold the current count is approaching. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> ObjectCountNextNumber;


	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);


private:
	void UpdateBanner() const;

	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;

	/** Placeholder text shown while no game world is active. */
	FText RuntimeOnlyText = NSLOCTEXT("NexusGuardian", "RuntimeOnly", "Only Available At Runtime");


	/** Every Guardian subsystem currently being observed (one per active world). */
	TArray<TObjectPtr<UNGuardianSubsystem>> Subsystems;
};