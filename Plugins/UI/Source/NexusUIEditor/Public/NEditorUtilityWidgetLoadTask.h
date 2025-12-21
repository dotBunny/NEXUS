// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDelayedEditorTask.h"
#include "NEditorUtilityWidgetLoadTask.generated.h"

UCLASS()
class NEXUSUIEDITOR_API UNEditorUtilityWidgetLoadTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	static void Create();
	
private:
	
	UFUNCTION()
	void Execute();
};