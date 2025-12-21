// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNOrganGenerationContext;
class UNOrganComponent;

class FNOrganGeneratorTaskContext
{
	friend struct FNOrganGeneratorTask;
public:
	FNOrganGeneratorTaskContext(FNOrganGenerationContext* GeneratorContextMap, uint64 TaskSeed);
protected:
	uint64 Seed;
};
