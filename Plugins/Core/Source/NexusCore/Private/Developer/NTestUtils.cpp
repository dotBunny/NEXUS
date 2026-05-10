// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NTestUtils.h"

FString FNTestUtils::WaitForNewLogFile(const TCHAR* Wildcard, const TSet<FString>& PreExisting, const double TimeoutSeconds)
{
	const double Deadline = FPlatformTime::Seconds() + TimeoutSeconds;
	FString FoundPath;
	int64 LastSize = -1;
	while (FPlatformTime::Seconds() < Deadline)
	{
		if (FoundPath.IsEmpty())
		{
			TArray<FString> Found;
			IFileManager::Get().FindFiles(Found, *(FPaths::ProjectLogDir() / Wildcard), true, false);
			for (const FString& Name : Found)
			{
				if (!PreExisting.Contains(Name))
				{
					FoundPath = FPaths::ProjectLogDir() / Name;
					break;
				}
			}
		}
		else
		{
			const int64 CurrentSize = IFileManager::Get().FileSize(*FoundPath);
			if (CurrentSize > 0 && CurrentSize == LastSize)
			{
				return FoundPath;
			}
			LastSize = CurrentSize;
		}
		FPlatformProcess::Sleep(0.025f);
	}
	return FString();
}