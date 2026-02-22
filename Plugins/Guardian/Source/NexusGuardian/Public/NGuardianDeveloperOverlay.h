// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonNumericTextBlock.h"
#include "NGuardianSubsystem.h"
#include "Widgets/NDeveloperOverlay.h"
#include "NGuardianDeveloperOverlay.generated.h"

class UNDynamicRefObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Guardian Developer Overlay", BlueprintType, Abstract)
class NEXUSGUARDIAN_API UNGuardianDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World);

protected:	
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ObjectCountBox;	
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> ObjectCountNumber;	
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> BaseCountNumber;	
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> ObjectCountNextNumber;	
	
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

	
private:
	void UpdateBanner() const;
	
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
	
	FText RuntimeOnlyText = NSLOCTEXT("NexusGuardian", "RuntimeOnly", "Only Available At Runtime");
	
	
	TArray<TObjectPtr<UNGuardianSubsystem>> Subsystems; 
};