// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNDeveloperUtils
{
public:
	FORCEINLINE static int GetCurrentObjectCount() { return GUObjectArray.GetObjectArrayNumMinusAvailable(); }
};
