// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "NColor.h"
#include "CommonUserWidget.h"
#include "NDeveloperOverlay.generated.h"

class UVerticalBox;
class UHorizontalBox;
class UCommonHierarchicalScrollBox;
class UNCheckBox;
class UButton;

/**
 * Abstract base widget for NEXUS developer/diagnostic overlays. Supplies a banner row and a
 * ContainerBox slot for subclasses to populate; bIsEditorUtilityWidget toggles editor-only
 * behavior when hosted inside an EUW.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Developer Overlay", Abstract, Blueprintable)
class NEXUSUI_API UNDeveloperOverlay :  public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Display the banner row with Text and the supplied foreground/background color pair. */
	UFUNCTION(BlueprintCallable)
	void ShowContainerBanner(const FText& Text = FText::GetEmpty(),
		ENColor MessageColor = ENColor::NC_White, ENColor BannerColor = ENColor::NC_NexusDarkBlue) const;

	/** Collapse the banner row. */
	UFUNCTION(BlueprintCallable)
	void HideContainerBanner() const;

	/** When true the overlay is hosted inside an EUW and should avoid runtime-only assumptions. */
	UPROPERTY(EditDefaultsOnly)
	bool bIsEditorUtilityWidget;

	/** Iterates all current GEngine world contexts and calls Bind() on each. Safe to call when GEngine is null. */
	void BindAllCurrentWorlds();
	/** Iterates all current GEngine world contexts and calls Unbind() on each. Safe to call when GEngine is null. */
	void UnbindAllCurrentWorlds();

	/** Override to subscribe to a specific world's subsystems when the overlay is constructed or a world is added. */
	virtual void BindWorld(UWorld* World) {}
	/** Override to unsubscribe from a specific world's subsystems when the overlay is destroyed or a world is removed. */
	virtual void UnbindWorld(const UWorld* World) {}

	/** Bound UCommonBorder that provides the banner row's background brush. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonBorder> ContainerBanner;

	/** Bound UCommonTextBlock that renders the banner row's message text. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> ContainerBannerMessage;

	/** Bound UVerticalBox where subclasses add diagnostic rows. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UVerticalBox> ContainerBox;
};