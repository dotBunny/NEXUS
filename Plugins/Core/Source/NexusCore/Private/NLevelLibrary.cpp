// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NLevelLibrary.h"
#include "NLevelUtils.h"

TArray<FString> UNLevelLibrary::GetAllMapNames(const TArray<FString> SearchPaths)
{
	return FNLevelUtils::GetAllMapNames(SearchPaths);
}