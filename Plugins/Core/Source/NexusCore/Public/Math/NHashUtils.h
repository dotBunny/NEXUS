// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
* A collection of utility methods for working with Vectors.
 */
class NEXUSCORE_API FNHashUtils
{
public:
	/**
	* Create a hash uint64 from a given FString using the dbj2 algorithm.
	*/
	static uint64 dbj2(const FString& InString);
};