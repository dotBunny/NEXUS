// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NDynamicRefDeveloperOverlayWidget.generated.h"

enum class ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "DynamicRef Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSDYNAMICREF_API UNDynamicRefDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
};
