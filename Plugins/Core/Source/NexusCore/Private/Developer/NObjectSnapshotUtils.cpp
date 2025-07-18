﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NObjectSnapshotUtils.h"

#include "NCoreMinimal.h"
#include "Developer/NDeveloperUtils.h"
#include "Runtime/Launch/Resources/Version.h"

int32 FNObjectSnapshotUtils::SnapshotTicket = 0;

FNObjectSnapshot FNObjectSnapshotUtils::Snapshot()
{
	// Create our Snapshot struct
	FNObjectSnapshot Snapshot(FNDeveloperUtils::GetCurrentObjectCount());
	Snapshot.Ticket = TakeTicket();
	
	FChunkedFixedUObjectArray& Objects = GUObjectArray.GetObjectItemArrayUnsafe();
	for (int i = 0; i < GUObjectArray.GetObjectArrayNum(); i++)
	{
#if UE_VERSION_OLDER_THAN(5, 6, 0) // .Object gets deprecated in 5.6
		if (Objects[i].Object)
#else
		if (Objects[i].GetObject())
#endif
		{
			Snapshot.CapturedObjects.Add(FNObjectSnapshotEntry(Objects[i]));
			Snapshot.CapturedObjectCount++;
		}
	}

	// Send it back
	return MoveTemp(Snapshot);
}

FNObjectSnapshotDiff FNObjectSnapshotUtils::Diff(FNObjectSnapshot OldSnapshot, FNObjectSnapshot NewSnapshot, const bool bRemoveKnownLeaks)
{
	FNObjectSnapshotDiff Diff;

	if (OldSnapshot.Ticket > NewSnapshot.Ticket)
	{
		const FNObjectSnapshot TempSnapshot = OldSnapshot;
		OldSnapshot = NewSnapshot;
		NewSnapshot = TempSnapshot;
		N_LOG(Warning, TEXT("[FNObjectSnapshotUtils::Diff] OldSnapshot was actually newer then NewSnapshot. Swapping."))
	}

	// Calculate before we start chopping things up
	Diff.CapturedObjectCount = NewSnapshot.CapturedObjectCount - OldSnapshot.CapturedObjectCount;

	// If we check what has been maintained or removed, that leaves whats left in the array as having been added.
	for (int OldIndex = OldSnapshot.CapturedObjectCount - 1; OldIndex >= 0; OldIndex--)
	{
		FNObjectSnapshotEntry& EntryA = OldSnapshot.CapturedObjects[OldIndex];
		bool bFoundEntry = false;

		// Find things we have
		for (int NewIndex = NewSnapshot.CapturedObjectCount - 1; NewIndex >= 0 ; NewIndex--)
		{
			if (FNObjectSnapshotEntry& EntryB = NewSnapshot.CapturedObjects[NewIndex];
				EntryA.IsEqual(EntryB))
			{
				Diff.Maintained.Add(EntryB);
				OldSnapshot.CapturedObjects.RemoveAt(OldIndex);
				NewSnapshot.CapturedObjects.RemoveAt(NewIndex);
				NewSnapshot.CapturedObjectCount--;
				bFoundEntry = true;
				break;
			}
		}

		// Not found, must have been removed
		if (!bFoundEntry)
		{
			Diff.Removed.Add(EntryA);
			OldSnapshot.CapturedObjects.RemoveAt(OldIndex);
		}
	}

	// Whats left is added
	for (int i = 0; i < NewSnapshot.CapturedObjects.Num(); i++)
	{
		Diff.Added.Add(NewSnapshot.CapturedObjects[i]);
	}

	// Update counts
	Diff.AddedCount = Diff.Added.Num();
	Diff.RemovedCount = Diff.Removed.Num();
	Diff.MaintainedCount = Diff.Maintained.Num();

	if (bRemoveKnownLeaks)
	{
		RemoveKnownLeaks(Diff);
	}
	
	return MoveTemp(Diff);
}

void FNObjectSnapshotUtils::RemoveKnownLeaks(FNObjectSnapshotDiff& Diff)
{
	for (int i = Diff.AddedCount - 1; i >= 0; i--)
	{
		FNObjectSnapshotEntry& Entry = Diff.Added[i];

		// Remove Engine Entry
		if (Entry.Name.Equals(TEXT("/Script/Engine")) && Entry.SerialNumber == 0)
		{
			Diff.Added.RemoveAt(i, EAllowShrinking::No);
			Diff.AddedCount--;
		}

		// Remove InputCore
		if (Entry.Name.Equals(TEXT("/Script/InputCore")))
		{
			Diff.Added.RemoveAt(i, EAllowShrinking::No);
			Diff.AddedCount--;
		}

		// Remove Niagara Component Pool entry (will have a _# in its name)
		if (Entry.Name.StartsWith(TEXT("NiagaraComponentPool")))
		{
			Diff.Added.RemoveAt(i, EAllowShrinking::No);
			Diff.AddedCount--;
		}
	}
}