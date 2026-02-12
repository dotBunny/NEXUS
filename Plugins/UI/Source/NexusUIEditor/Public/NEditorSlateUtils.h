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
	TSharedPtr<SDockTab> FindDocTabByIdentifier(const FName TabIdentifier);
};

