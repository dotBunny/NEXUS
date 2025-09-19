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
	static UAsyncEditorDelay* CreateDelayMechanism() { return NewObject<UAsyncEditorDelay>(); };
	
protected:
	void Lock(UAsyncEditorDelay* DelayMechanism)
	{
		Parent = DelayMechanism;
		Parent->AddToRoot();

		AddToRoot();
	}
	void Release()
	{
		Parent->RemoveFromRoot();

		RemoveFromRoot();
	}

private:
	UPROPERTY()
	TObjectPtr<UAsyncEditorDelay> Parent;
};