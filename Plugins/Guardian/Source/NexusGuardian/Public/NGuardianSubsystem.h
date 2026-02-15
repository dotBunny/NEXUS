// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NGuardianSettings.h"
#include "Developer/NObjectSnapshot.h"
#include "Macros/NSubsystemMacros.h"
#include "NGuardianSubsystem.generated.h"

/**
 * A developer-focused subsystem to help monitor specific metrics related to UObject usage.
 * @see <a href="https://nexus-framework.com/docs/plugins/guardian/">Guardian</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Developer Subsystem")
class NEXUSGUARDIAN_API UNGuardianSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG || UE_BUILD_TEST	
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNGuardianSubsystem, UNGuardianSettings::Get()->bDeveloperSubsystemEnabled)
#else // !(UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG || UE_BUILD_TEST)
 	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNGuardianSubsystem, false)
#endif // UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG || UE_BUILD_TEST

	UFUNCTION(BlueprintCallable, DisplayName = "Set Baseline", Category = "NEXUS|Developer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/guardian/#setting-a-baseline"))
	void SetBaseline();
	
	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override { return bBaselineSet && IsInitialized(); };
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	
private:
	bool bPassedObjectCountWarningThreshold = false;
	bool bPassedObjectCountSnapshotThreshold = false;
	bool bPassedObjectCountCompareThreshold = false;

	bool bBaselineSet = false;
	int32 BaseObjectCount = 0;
	int32 ObjectCountWarningThreshold = 0;
	int32 ObjectCountSnapshotThreshold = 0;
	int32 ObjectCountCompareThreshold = 0;
	bool bShouldOutputSnapshot = false;
	
	FNObjectSnapshot CaptureSnapshot;
};