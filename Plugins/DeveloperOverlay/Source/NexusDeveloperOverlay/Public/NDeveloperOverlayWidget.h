// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonUserWidget.h"
#include "NDeveloperOverlayWidget.generated.h"

class UCommonBorder;
class UVerticalBox;
class UHorizontalBox;
class UCommonHierarchicalScrollBox;
class UNCheckBox;
class UButton;

class UCommonTextBlock;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Developer Overlay Widget", Abstract, Blueprintable)
class NEXUSDEVELOPEROVERLAY_API UNDeveloperOverlayWidget :  public UCommonUserWidget
{
	GENERATED_BODY()
};