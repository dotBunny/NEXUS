// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorUtils.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NMultiplayerEditorSubsystem.generated.h"

/**
 * Editor subsystem that drives the NexusMultiplayer test workflow.
 *
 * Launches additional local client/server processes using the project's play-in-editor
 * settings augmented by UNMultiplayerEditorUserSettings, tracks their FProcHandles, and
 * shuts them down when the test ends or the editor exits.
 */
UCLASS()
class NEXUSMULTIPLAYEREDITOR_API UNMultiplayerEditorSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	N_EDITOR_TICKABLE_SUBSYSTEM(UNMultiplayerEditorSubsystem)

	//~FTickableGameObject
	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override
	{
		if (FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}
		return bIsMultiplayerTestRunning && ProcessHandles.Num() > 0;
	}
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNProcGenEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	//End FTickableGameObject

	/** Start the multiplayer test, spawning client/server processes according to the user settings. */
	void StartMultiplayerTest();
	/** Stop the multiplayer test and terminate any tracked processes. */
	void StopMultiplayerTest();
	/** Flip between started and stopped states. */
	void ToggleMultiplayerTest();

	/** @return true if a multiplayer test session is currently active. */
	bool IsTestRunning() const {  return bIsMultiplayerTestRunning; }

	/**
	 * Track a locally launched process so the subsystem can monitor and shut it down.
	 * @param ProcessIdentifier OS-level process id of the launched editor/client/server instance.
	 */
	UFUNCTION()
	void AddLocalProcess(const uint32 ProcessIdentifier);

private:
	/** Delegate handle for the local process registration callback. */
	FDelegateHandle LocalProcessDelegateHandle;
	/** Handles for all child processes spawned as part of the current test session. */
	TArray<FProcHandle> ProcessHandles;
	/** true while a multiplayer test is active. */
	bool bIsMultiplayerTestRunning;

	/** Accumulated delta time used to throttle process-state polling inside Tick. */
	float DeltaAccumulator;

};