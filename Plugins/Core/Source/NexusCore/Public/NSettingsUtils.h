// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNSettingsUtils
{
public:	
	static FName GetContainerName()
	{
		const FName ProjectContainerName = TEXT("Project");
		return ProjectContainerName;
	}
	static FName GetCategoryName()
	{
		const FName NexusCategoryName = TEXT("NEXUS");
		return NexusCategoryName;
	}
};
