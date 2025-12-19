// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNOrganGenerationContextMap;
class UNOrganComponent;

class FNOrganGeneratorTaskContext
{
public:
	FNOrganGeneratorTaskContext(FNOrganGenerationContextMap* GeneratorContextMap, uint64 TaskSeed);
};
