// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NHashUtils.h"

uint64 FNHashUtils::dbj2(const FString& InString)
{
	auto Array = InString.GetCharArray();
	const auto Data = Array.GetData();
	uint64 ComputeHash = 5381;
	for (size_t i = 0; i < Array.Num(); ++i)
		ComputeHash = 33 * ComputeHash + static_cast<unsigned char>(Data[i]);
	return ComputeHash;
}
