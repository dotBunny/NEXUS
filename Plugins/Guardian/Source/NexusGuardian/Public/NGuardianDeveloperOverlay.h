// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World);

protected:	
	
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

	
private:
	void UpdateBanner() const;
	
	
	
	FText NoReferencesFoundText = NSLOCTEXT("NexusDynamicRefs", "NoReferencesFound", "No References Found");
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
};