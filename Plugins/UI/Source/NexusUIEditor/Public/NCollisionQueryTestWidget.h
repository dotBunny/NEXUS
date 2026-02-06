// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityWidgetComponents.h"
#include "NTransientActor.h"
#include "NEditorUtilityWidget.h"
#include "NCollisionQueryTestSettings.h"
#include "Components/NDetailsView.h"
#include "NCollisionQueryTestWidget.generated.h"


/**
 * Used to test collision queries at design time in the editor.
 * @remarks The was originally started after watching George Prosser's UnrealFest 2023 talk "Collision Data in UE5". 
 * I later found the public version of the tool and was amused at how similar they were. Credits to George and the
 * team at Studio Gobo for the great inspiration, there is just a slightly different spin on the implementation here.
 * @ref https://www.youtube.com/watch?v=xIQI6nXFygA
 * @ref https://github.com/StudioGobo/UECollisionQueryTools 
 */
UCLASS(BlueprintType)
class NEXUSUIEDITOR_API UNCollisionQueryTestWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual void RestoreWidgetState(UObject* BlueprintWidget, FName Identifier, FNWidgetState& InState) override;
	virtual FNWidgetState GetWidgetState(UObject* BlueprintWidget) override;
	
	
protected:
	
	UFUNCTION() 
	void OnPIEMapCreated(UGameInstance* GameInstance);
	UFUNCTION()
	void OnWorldTick(float DeltaTime);
	
	UFUNCTION()
	void OnSelectStartButtonClicked();
	
	UFUNCTION()
	void OnSelectEndButtonClicked();
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectStartButton;
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectEndButton;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNDetailsView> ObjectDetails;
	
	UPROPERTY()
	TObjectPtr<ANTransientActor> StartActor;
	
	UPROPERTY()
	TObjectPtr<ANTransientActor> EndActor;
	
	UPROPERTY(EditAnywhere)
	FNCollisionQueryTestSettings Settings;
	
private:
	void CheckActors();
	
	FDelegateHandle OnPIEMapCreatedHandle;
};