// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Subsystems/EditorAssetSubsystem.h"

class FNPoseAssetFixer
{
public:
	static void OutOfDateAnimationSource(bool bIsContextMenu);
private:
	static bool UpdatePoseAsset(UEditorAssetSubsystem* EditorAssetSubsystem, UPoseAsset* PoseAsset, TArray<UPackage*>& CleanupPackages);
};
