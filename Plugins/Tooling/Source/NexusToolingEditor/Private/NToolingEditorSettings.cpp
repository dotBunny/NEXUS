// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorSettings.h"

#if WITH_EDITOR	

bool UNToolingEditorSettings::IsAssetIgnored(const FSoftObjectPath& AssetPath) const
{
	if (IgnoredAssets.Contains(AssetPath)) return true;

	const FString Path = AssetPath.GetAssetPathString();
	for (int i = 0; i < IgnoredPrefixes.Num(); i++)
	{
		if (Path.StartsWith(IgnoredPrefixes[i])) return true;
	}

	return false;
}

#endif // WITH_EDITOR