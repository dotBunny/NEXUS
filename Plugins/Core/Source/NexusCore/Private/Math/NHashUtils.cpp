// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NHashUtils.h"

uint64 FNHashUtils::djb2(const FString& InString)
{
	const TCHAR* Data = *InString;
	const int32 Length = InString.Len();
	uint64 ComputeHash = 5381;
	for (int32 i = 0; i < Length; ++i)
		ComputeHash = 33 * ComputeHash + static_cast<unsigned char>(Data[i]);
	return ComputeHash;
}
