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
	* Create a hash <code>uint64</code> from a given <code>FString</code> using the <code>dbj2</code> algorithm.
	*/
	static uint64 dbj2(const FString& InString);
};