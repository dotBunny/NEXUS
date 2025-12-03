// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "INWidgetTabDetails.generated.h"

UINTERFACE(BlueprintType)
class NEXUSUI_API UNWidgetTabDetails : public UInterface
{
	GENERATED_BODY()
};


class NEXUSUI_API INWidgetTabDetails
{
	GENERATED_BODY()

public:
	
	virtual FSlateIcon GetTabDisplayIcon() { return FSlateIcon(); }
	virtual TAttribute<const FSlateBrush*> GetTabDisplayBrush() { return TAttribute<const FSlateBrush*>(); }
	virtual FText GetTabDisplayText() const { return FText::GetEmpty(); }
};