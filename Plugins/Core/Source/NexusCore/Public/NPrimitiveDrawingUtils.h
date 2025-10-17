// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNPrimitiveDrawingUtils
{
public:
	static void DrawString(FPrimitiveDrawInterface* PDI, FString& String, FVector Position, FRotator Rotation, FLinearColor ForegroundColor, float Scale = 1, bool bBackground = false, FLinearColor BackgroundColor = FLinearColor::Black);
};
