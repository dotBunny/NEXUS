// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NDynamicRefsDeveloperOverlayWidget.generated.h"

enum class ENProcGenOperationState : uint8;
class UNProcGenOperation;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "DynamicRefs Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSDYNAMICREFS_API UNDynamicRefsDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
};
