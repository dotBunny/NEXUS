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
	/**
	 * Renders a multi-line text box item to the supplied canvas, drawing its background, border, and lines.
	 * @param TextBox The text box content to draw; re-measured first if its contents are dirty.
	 * @param Canvas The canvas to draw onto.
	 * @param TopLeftPosition Screen-space position of the box's top-left corner.
	 */
	static void DrawCanvasTextBox(FNMultiLineTextBoxCanvasItem* TextBox, FCanvas* Canvas, FVector2D TopLeftPosition);
};
