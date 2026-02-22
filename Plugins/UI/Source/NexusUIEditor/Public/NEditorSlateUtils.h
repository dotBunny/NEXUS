// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * A utility methods collection for editor-time Slate
 */
class NEXUSUIEDITOR_API FNEditorSlateUtils
{
public:
	static TSharedPtr<SDockTab> FindDocTab(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel, const FName TabIdentifier);
	static TSharedPtr<SDockTab> FindDocTabByIdentifier(const FName TabIdentifier);
};

