// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorSettings.h"

#if WITH_EDITOR	

bool UNToolingEditorSettings::IsAssetIgnored(const FSoftObjectPath& AssetPath) const
{
	if (ValidatorIgnoredAssets.Contains(AssetPath)) return true;

	const FString Path = AssetPath.GetAssetPathString();
	for (int i = 0; i < ValidatorIgnoredPrefixes.Num(); i++)
	{
		if (Path.StartsWith(ValidatorIgnoredPrefixes[i])) return true;
	}

	return false;
}

#endif // WITH_EDITOR