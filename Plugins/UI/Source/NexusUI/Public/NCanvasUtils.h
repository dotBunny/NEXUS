// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CanvasItems/NMultiLineTextBoxCanvasItem.h"


/**
 * A utility methods collection for Canvas
 */
class NEXUSUI_API FNCanvasUtils
{
public:
	static void DrawCanvasTextBox(FNMultiLineTextBoxCanvasItem* TextBox, FCanvas* Canvas, FVector2D TopLeftPosition);
};
