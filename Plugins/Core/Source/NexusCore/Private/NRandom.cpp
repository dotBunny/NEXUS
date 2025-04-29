// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NRandom.h"

#include "Math/NSeedGenerator.h"

// We'll do a static initialization of the core random for now, but this eventually would be set during any sort of multiplayer or loading scenario.
FNMersenneTwister FNRandom::Deterministic = FNMersenneTwister(FNSeedGenerator::SeedFromText(TEXT("NEXUS")));
FRandomStream FNRandom::NonDeterministic = FRandomStream(FDateTime::Now().GetMillisecond());