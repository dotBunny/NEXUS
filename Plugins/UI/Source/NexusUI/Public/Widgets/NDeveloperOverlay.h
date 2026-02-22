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

UCLASS(ClassGroup = "NEXUS", DisplayName = "Developer Overlay", Abstract, Blueprintable)
class NEXUSUI_API UNDeveloperOverlay :  public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void ShowContainerBanner(const FText& Text = FText::GetEmpty(), 
		ENColor MessageColor = ENColor::NC_White, ENColor BannerColor = ENColor::NC_NexusDarkBlue) const;
	
	UFUNCTION(BlueprintCallable)
	void HideContainerBanner() const;
	
	UPROPERTY(EditDefaultsOnly)
	bool bIsEditorUtilityWidget;
	
protected:
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonBorder> ContainerBanner;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> ContainerBannerMessage;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UVerticalBox> ContainerBox;
};