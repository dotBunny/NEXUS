// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NHashUtils.h"

#include "Containers/StringConv.h"

uint64 FNHashUtils::djb2(const FString& InString)
{
	// Hash the UTF-8 encoding rather than raw TCHAR code units so the result does not
	// depend on the platform's TCHAR width and non-ASCII characters are not truncated.
	const FTCHARToUTF8 Converted(*InString, InString.Len());
	const ANSICHAR* Data = Converted.Get();
	const int32 Length = Converted.Length();
	uint64 ComputeHash = 5381;
	for (int32 i = 0; i < Length; ++i)
		ComputeHash = 33 * ComputeHash + static_cast<uint8>(Data[i]);
	return ComputeHash;
}
