// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
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
	 * Associates the task with DelayMechanism and root-sets both so neither is GC'd while the task is pending.
	 * @param DelayMechanism The delay driver that will fire this task's work.
	 */
	void Lock(UAsyncEditorDelay* DelayMechanism)
	{
		Parent = DelayMechanism;
		Parent->AddToRoot();

		AddToRoot();
	}

	/** Unbinds the task from its delay mechanism and removes both from the root set; safe to call from the completion callback. */
	void Release()
	{
		Parent->Complete.RemoveAll(this);
		Parent->RemoveFromRoot();

		RemoveFromRoot();
	}

private:
	UPROPERTY()
	TObjectPtr<UAsyncEditorDelay> Parent;
};