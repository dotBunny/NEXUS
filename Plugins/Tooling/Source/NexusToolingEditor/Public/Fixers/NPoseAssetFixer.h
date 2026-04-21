// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Subsystems/EditorAssetSubsystem.h"

/**
 * Fixer that re-syncs pose assets whose source animation has changed under them. Exposed as a
 * content-browser action and registered with FNFixersMenu.
 */
class FNPoseAssetFixer
{
public:
	/** Update the currently-selected pose assets' sources; bIsContextMenu toggles confirmation UI. */
	static void OutOfDateAnimationSource(bool bIsContextMenu);

	/** @return true if the current selection contains at least one pose asset that can be updated. */
	static bool CanExecuteOutOfDataAnimationSource();

private:
	/** Re-source PoseAsset from its configured animation and mark dependent packages for save. */
	static bool UpdatePoseAsset(UEditorAssetSubsystem* EditorAssetSubsystem,
		UPoseAsset* PoseAsset, TArray<UPackage*>& CleanupPackages);
};
