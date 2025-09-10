﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreSettings.h"
#include "NObjectSnapshot.h"
#include "Macros/NSubsystemMacros.h"
#include "NDeveloperSubsystem.generated.h"

UCLASS()
class NEXUSCORE_API UNDeveloperSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG || UE_BUILD_TEST	
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNDeveloperSubsystem, UNCoreSettings::Get()->bDeveloperSubsystemEnabled)
#else
 	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(UNDeveloperSubsystem, false)
#endif	

	UFUNCTION(BlueprintCallable, DisplayName = "Set Baseline", Category = "NEXUS|Developer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/core/types/developer-subsystem/#setting-a-baseline"))
	void SetBaseline();
	
	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override { return bBaselineSet; };

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