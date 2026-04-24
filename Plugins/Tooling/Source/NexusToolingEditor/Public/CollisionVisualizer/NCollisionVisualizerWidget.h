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

	//~UNEditorUtilityWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//End UNEditorUtilityWidget

	/** Per-frame callback driven by ANCollisionVisualizerActor::Tick to re-run the configured query. */
	void OnWorldTick(const ANCollisionVisualizerActor* Actor);

	/** Push current widget Settings onto Actor so its transform/draw state matches. */
	void PushSettings(ANCollisionVisualizerActor* Actor) const;

	/** Pull Actor state back into the widget's Settings (used after viewport-driven moves). */
	void UpdateSettings(const ANCollisionVisualizerActor* Actor);

protected:

	/** Details-view callback that forwards per-property edits into the visualizer actor. */
	UFUNCTION()
	void OnPropertyValueChanged(FName Name);

	/** PIE-started hook — rebinds the visualizer actor to the PIE world. */
	void OnPIEStarted(UGameInstance* GameInstance);

	/** PIE-ready hook — finalises bindings once the PIE world is fully live. */
	void OnPIEReady(UGameInstance* GameInstance);

	/** PIE-ended hook — returns the visualizer to the editor world. */
	void OnPIEEnded(UGameInstance* GameInstance);

	/** Focus/select the start-point scene component in the level editor. */
	UFUNCTION()
	void SelectStartPoint();

	/** Focus/select the end-point scene component in the level editor. */
	UFUNCTION()
	void SelectEndPoint();

	/** Bound button that triggers SelectStartPoint to focus the start-point scene component. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectStartButton;

	/** Bound button that triggers SelectEndPoint to focus the end-point scene component. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UEditorUtilityButton> SelectEndButton;

	/** Bound text block displaying the currently-bound visualizer actor's name. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> ActorNameText;

	/** Bound details view that surfaces the FNCollisionVisualizerSettings struct for editing. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNDetailsView> ObjectDetails;

	/** Actor that hosts the start/end points in the current world; recreated on PIE transitions. */
	UPROPERTY()
	TObjectPtr<ANCollisionVisualizerActor> QueryActor;

	/** Persisted configuration edited via the widget's details view. */
	UPROPERTY(EditAnywhere)
	FNCollisionVisualizerSettings Settings;

private:
	/** Load the previously-saved Settings from project config. */
	void RestoreState();

	/** Persist the current Settings to project config. */
	void SaveState() const;

	/** Spawn the visualizer actor in TargetWorld (or the editor world if null). */
	void CreateActor(UWorld* TargetWorld = nullptr);

	/** Destroy the currently-bound visualizer actor. */
	void DestroyActor();

	/** @return true if a property change for Name requires PushSettings to the actor. */
	static bool DoesPropertyAffectActor(FName Name);

	FDelegateHandle OnPIEStartedHandle;
	FDelegateHandle OnPIEReadyHandle;
	FDelegateHandle OnPIEEndedHandle;
};