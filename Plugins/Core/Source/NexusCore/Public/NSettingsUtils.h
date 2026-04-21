// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Provides the canonical container/category names every NEXUS plugin registers its
 * developer and project settings under. Use these accessors rather than duplicating
 * the string literals so that all NEXUS settings appear under a single, consistent
 * section in the Project Settings UI.
 */
class FNSettingsUtils
{
public:
	/**
	 * The settings container name (e.g. "Project") NEXUS settings are registered in.
	 * @return The container name used by ISettingsModule::RegisterSettings.
	 */
	static FName GetContainerName()
	{
		const FName ProjectContainerName = TEXT("Project");
		return ProjectContainerName;
	}

	/**
	 * The category name all NEXUS settings groups appear under in the Project Settings UI.
	 * @return The canonical "NEXUS" category name.
	 */
	static FName GetCategoryName()
	{
		const FName NexusCategoryName = TEXT("NEXUS");
		return NexusCategoryName;
	}
};
