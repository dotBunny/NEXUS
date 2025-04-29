// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Utility methods to work with the IDetailLayoutBuilder.
 */
class NEXUSCOREEDITOR_API FNDetailLayoutBuilderUtils
{
public:
	static void HideActorCategories(IDetailLayoutBuilder& DetailBuilder);
	static void HideDefaultCategories(IDetailLayoutBuilder& DetailBuilder);
	static void HideLevelCategories(IDetailLayoutBuilder& DetailBuilder);
	static void HideNetworkingCategories(IDetailLayoutBuilder& DetailBuilder);
	static void HidePhysicsCategories(IDetailLayoutBuilder& DetailBuilder);
	static void HideVisualCategories(IDetailLayoutBuilder& DetailBuilder);
};