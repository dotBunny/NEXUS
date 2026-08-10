// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NTerrainUtils.h"

#include "AssetCompilingManager.h"
#include "NActorUtils.h"
#include "TickableEditorObject.h"
#include "Containers/Ticker.h"
#include "Engine/Level.h"
#include "Misc/ScopedSlowTask.h"

uint32 FNTerrainUtils::ComputeFingerprint(const ULevel* InLevel)
{
	if (InLevel == nullptr) return 0;

	uint32 Hash = 0;
	for (const AActor* Actor : InLevel->Actors)
	{
		if (!FNActorUtils::IsTerrainActor(Actor)) continue;

		// A terrain actor whose components all still report placeholder bounds yields an invalid box. Folding in a
		// marker rather than skipping is what makes the later transition to real geometry register as a change.
		if (const FBox Box = FNActorUtils::GetBuiltComponentsBoundingBox(Actor, false); Box.IsValid)
		{
			Hash = HashCombine(Hash, GetTypeHash(FIntVector(Box.Min)));
			Hash = HashCombine(Hash, GetTypeHash(FIntVector(Box.Max)));
		}
		else
		{
			Hash = HashCombine(Hash, 1u);
		}
	}
	return Hash;
}

bool FNTerrainUtils::WaitForSettle(const ULevel* InLevel, const double TimeoutSeconds)
{
	if (InLevel == nullptr) return true;

	FScopedSlowTask WaitTask(0, NSLOCTEXT("NexusCoreEditor", "Task_WaitForTerrain", "Waiting for terrain to finish building ..."));
	WaitTask.MakeDialog(false);

	const double StartTime = FPlatformTime::Seconds();
	uint32 Fingerprint = ComputeFingerprint(InLevel);
	double LastChangeTime = StartTime;

	// Settle is inferred from the geometry holding still rather than queried: no engine-side barrier covers the whole
	// Mesh Partition pipeline, and a direct "is anything unbuilt" test never clears for a section that legitimately
	// covers nothing. Inferring also means this always eventually returns.
	while (true)
	{
		const double Now = FPlatformTime::Seconds();

		if (const uint32 Current = ComputeFingerprint(InLevel); Current != Fingerprint)
		{
			Fingerprint = Current;
			LastChangeTime = Now;
		}
		else if (Now - LastChangeTime >= NEXUS::CoreEditor::Terrain::SettleSeconds)
		{
			return true;
		}

		if (Now - StartTime >= TimeoutSeconds)
		{
			UE_LOG(LogNexusCoreEditor, Warning,
				TEXT("Gave up waiting for the terrain in '%s' to finish building after %.0f seconds. Anything calculated from it now may describe a partially built terrain."),
				*InLevel->GetOutermost()->GetName(), TimeoutSeconds);
			return false;
		}

		// Advance what a terrain build actually depends on, and no more. FTickableEditorObject carries
		// UMeshPartitionEditorSubsystem, which is what promotes finished build tasks into section actors; the asset
		// compiler builds the section static meshes; the core ticker drives the deferred work behind both. Deliberately
		// not a full engine or Slate tick — see the remark on this function's declaration.
		constexpr float PumpDelta = 1.0f / 60.0f;
		FTickableEditorObject::TickObjects(PumpDelta);
		FAssetCompilingManager::Get().ProcessAsyncTasks();
		FTSTicker::GetCoreTicker().Tick(PumpDelta);

		// Yield rather than spin; the work being waited on is largely on other threads.
		FPlatformProcess::Sleep(0.001f);
	}
}
