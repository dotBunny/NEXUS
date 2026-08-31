// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganCollisionCache.h"

#include "NWorldAssemblyMinimal.h"
#include "Algo/Unique.h"

void FNOrganCollisionCache::SetBakeResult(const uint64 InFingerprint, TArray<uint64>&& InKeys)
{
	SourceKeys = MoveTemp(InKeys);
	SourceKeys.Sort();

	// One element can only be named once. A duplicate would survive into the resolved mesh list and be tested twice
	// by every builder, which costs work without changing any answer.
	const int32 UniqueCount = Algo::Unique(SourceKeys);
	SourceKeys.SetNum(UniqueCount);

	Fingerprint = InFingerprint;
	BakeTime = FDateTime::UtcNow();
}

bool FNOrganCollisionCache::Serialize(FArchive& Ar)
{
	uint8 Version = CurrentVersion;
	Ar << Version;

	if (Ar.IsLoading() && Version != CurrentVersion)
	{
		UE_LOG(LogNexusWorldAssembly, Log,
			TEXT("Discarding organ collision cache written at version %u; this build reads version %u."),
			Version, CurrentVersion);
		Reset();
		return true;
	}

	Ar << Fingerprint;
	Ar << SourceKeys;
	Ar << BakeTime;

	return true;
}
