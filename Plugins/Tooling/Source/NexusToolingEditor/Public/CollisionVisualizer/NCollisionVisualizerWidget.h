// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "EditorUtilityWidgetComponents.h"
#include "NCollisionVisualizerActor.h"
#include "NEditorUtilityWidget.h"
#include "NCollisionVisualizerSettings.h"
#include "Components/NDetailsView.h"
#include "NCollisionVisualizerWidget.generated.h"

/**
 * Used to test collision queries at design time in the editor.
 * @remarks The was originally started after watching George Prosser's UnrealFest 2023 talk "Collision Data in UE5". 
 * I later found the public version of the tool and was amused at how similar they were. Credits to George and the
 * team at Studio Gobo for the great inspiration, there is just a slightly different spin on the implementation here.
 * @ref https://www.youtube.com/watch?v=xIQI6nXFygA
 * @ref https://github.com/StudioGobo/UECollisionQueryTools 
 */
UCLASS(BlueprintType)
class NEXUSTOOLINGEDITOR_API UNCollisionVisualizerWidget : public UNEditorUtilityWidget
{
	friend class ANCollisionVisualizerActor;
	
	GENERATED_BODY()

public:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void OnWorldTick(const ANCollisionVisualizerActor* Actor);
	void PushSettings(ANCollisionVisualizerActor* Actor) const;
	void UpdateSettings(const ANCollisionVisualizerActor* Actor);
	
protected:
	
	UFUNCTION()
	void OnPropertyValueChanged(FName Name);
	
	void OnPIEStarted(UGameInstance* GameInstance);
	void OnPIEReady(UGameInstance* GameInstance);
	void OnPIEEnded(UGameInstance* GameInstance);
	
	UFUNCTION()
	void SelectStartPoint();
	
	UFUNCTION()
	void SelectEndPoint();
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectStartButton;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectEndButton;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> ActorNameText;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNDetailsView> ObjectDetails;
	
	UPROPERTY()
	TObjectPtr<ANCollisionVisualizerActor> QueryActor;
	
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerSettings Settings;

private:
	void RestoreState();
	void SaveState() const;
	
	void CreateActor(UWorld* TargetWorld = nullptr);
	void DestroyActor();

	static bool DoesPropertyAffectActor(FName Name);
	
	FDelegateHandle OnPIEStartedHandle;
	FDelegateHandle OnPIEReadyHandle;
	FDelegateHandle OnPIEEndedHandle;
};