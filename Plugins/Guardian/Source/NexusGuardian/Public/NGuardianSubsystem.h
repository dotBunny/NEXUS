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
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Guardian Subsystem")
class NEXUSGUARDIAN_API UNGuardianSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(
		UNGuardianSubsystem, 
		FNBuildConfigurationAvailability::IsAvailableInBuild(
			static_cast<ENBuildConfigurationAvailability>(UNGuardianSettings::Get()->BuildAvailability)))

	/**
	 * Capture the current UObject count as the baseline all subsequent deltas are measured against.
	 * @note Thresholds are relative to this baseline; thresholds are evaluated only after it is set.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Set Baseline", Category = "NEXUS|Developer",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/guardian/#setting-a-baseline"))
	void SetBaseline();

	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override { return bBaselineSet && IsInitialized(); };

	N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(ETickableTickType::Conditional)

	/** @return The last sampled total UObject count. */
	int32 GetLastObjectCount() const { return LastObjectCount; }
	/** @return The baseline UObject count set by SetBaseline. */
	int32 GetBaseObjectCount() const { return BaseObjectCount; }

	/** @return The configured delta (from baseline) that triggers a warning log. */
	int32 GetObjectCountWarningThreshold() const { return ObjectCountWarningThreshold; }
	/** @return The configured delta (from baseline) that triggers a snapshot capture. */
	int32 GetObjectCountSnapshotThreshold() const { return ObjectCountSnapshotThreshold; }
	/** @return The configured delta (from baseline) that triggers a snapshot comparison. */
	int32 GetObjectCountCompareThreshold() const { return ObjectCountCompareThreshold; }

	/** @return true if the warning threshold has been crossed since the baseline was set. */
	bool HasPassedWarningThreshold() const { return bPassedObjectCountWarningThreshold; }
	/** @return true if the snapshot threshold has been crossed since the baseline was set. */
	bool HasPassedSnapshotThreshold() const { return bPassedObjectCountSnapshotThreshold; }
	/** @return true if the compare threshold has been crossed since the baseline was set. */
	bool HasPassedCompareThreshold() const { return bPassedObjectCountCompareThreshold; }

private:
	/** Latched true once the warning threshold has been crossed. */
	bool bPassedObjectCountWarningThreshold = false;
	/** Latched true once the snapshot threshold has been crossed. */
	bool bPassedObjectCountSnapshotThreshold = false;
	/** Latched true once the compare threshold has been crossed. */
	bool bPassedObjectCountCompareThreshold = false;

	/** True after SetBaseline has captured a baseline count. */
	bool bBaselineSet = false;
	/** Most recent UObject count sampled in Tick. */
	int32 LastObjectCount = 0;
	/** UObject count at the time SetBaseline was called. */
	int32 BaseObjectCount = 0;
	/** Resolved warning threshold (baseline + UNGuardianSettings::ObjectCountWarningThreshold). */
	int32 ObjectCountWarningThreshold = 0;
	/** Resolved snapshot threshold (baseline + UNGuardianSettings::ObjectCountSnapshotThreshold). */
	int32 ObjectCountSnapshotThreshold = 0;
	/** Resolved compare threshold (baseline + UNGuardianSettings::ObjectCountCompareThreshold). */
	int32 ObjectCountCompareThreshold = 0;
	/** Mirrors UNGuardianSettings::bObjectCountCaptureOutput; when true, snapshots are written to disk. */
	bool bShouldOutputSnapshot = false;

	/** In-memory snapshot captured at the snapshot threshold and compared at the compare threshold. */
	FNObjectSnapshot CaptureSnapshot;
};