// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "EditorUtilitySubsystem.h"
#include "NDelayedEditorTask.generated.h"

/**
 * An abstract class designed to encompass work to be completed at some level of delay from the time of its creation.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/editor-types/delayed-editor-task/">UNDelayedEditorTask</a>
 */
UCLASS(Abstract)
class NEXUSCOREEDITOR_API UNDelayedEditorTask : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Factory for the UAsyncEditorDelay that drives the delay timer.
	 * @return A new delay mechanism the derived task can schedule work on.
	 */
	static UAsyncEditorDelay* CreateDelayMechanism() { return NewObject<UAsyncEditorDelay>(); };

protected:
	/**
	 * Associates the task with DelayMechanism and registers the task with the EditorUtilitySubsystem so it survives GC
	 * while pending. The delay mechanism stays reachable transitively — it is both this task's Parent property and its
	 * Outer — so it does not need its own registration.
	 * @param DelayMechanism The delay driver that will fire this task's work.
	 */
	void Lock(UAsyncEditorDelay* DelayMechanism)
	{
		Parent = DelayMechanism;

		if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
		{
			EditorUtilitySubsystem->RegisterReferencedObject(this);
		}
	}

	/** Unbinds the task from its delay mechanism and unregisters it from the EditorUtilitySubsystem; safe to call from the completion callback. */
	void Release()
	{
		Parent->Complete.RemoveAll(this);

		if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
		{
			EditorUtilitySubsystem->UnregisterReferencedObject(this);
		}
	}

private:
	/** The delay mechanism driving this task; held between Lock and Release so it stays reachable for GC. */
	UPROPERTY()
	TObjectPtr<UAsyncEditorDelay> Parent;
};