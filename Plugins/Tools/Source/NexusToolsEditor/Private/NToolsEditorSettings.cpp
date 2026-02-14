// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorSettings.h"

bool UNToolsEditorSettings::IsAssetIgnored(const FSoftObjectPath& AssetPath) const
{
	if (IgnoredAssets.Contains(AssetPath)) return true;

	const FString Path = AssetPath.GetAssetPathString();
	for (int i = 0; i < IgnoredPrefixes.Num(); i++)
	{
		if (Path.StartsWith(IgnoredPrefixes[i])) return true;
	}

	return false;
}
