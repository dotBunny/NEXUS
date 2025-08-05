// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NDelayedDetailsRefreshInstanceObject.generated.h"

UCLASS()
class NEXUSCOREEDITOR_API UNDelayedDetailsRefreshInstanceObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void Create()
	{
		UAsyncEditorDelay* DelayedRefresh = NewObject<UAsyncEditorDelay>();
		UNDelayedDetailsRefreshInstanceObject* LibraryObject = NewObject<UNDelayedDetailsRefreshInstanceObject>(DelayedRefresh);
		LibraryObject->AddToRoot();
		
		DelayedRefresh->Complete.AddDynamic(LibraryObject, &UNDelayedDetailsRefreshInstanceObject::Execute);
		DelayedRefresh->Start(0.5f, 5);
	}
	
private:	
	UFUNCTION()
	void Execute()
	{
		RemoveFromRoot();
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
};