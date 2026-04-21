// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotUtils.h"
#include "NObjectSnapshot.h"
#include "NObjectSnapshotDiff.h"

/**
 * Entry points for capturing and comparing UObject snapshots.
 *
 * The full snapshot pipeline is process-global state held as static members on this class:
 * Snapshot() walks the global object array to produce an FNObjectSnapshot, Diff() compares two
 * snapshots into an FNObjectSnapshotDiff, and the on-disk helpers provide a coarse way to persist
 * a baseline snapshot across editor sessions or commandlet runs.
 */
class NEXUSCORE_API FNObjectSnapshotUtils
{
public:
	/**
	 * Produces the next monotonic ticket number used to tag new snapshots.
	 * @return A new ticket value guaranteed to be strictly greater than any previously issued.
	 */
	static int32 TakeTicket() { return ++SnapshotTicket; }

	/**
	 * Walks the global UObject array and captures a new FNObjectSnapshot.
	 * @return The captured snapshot; its Ticket field is set from TakeTicket().
	 * @note Game-thread only; this function mutates static state and allocates.
	 */
	static FNObjectSnapshot Snapshot();

	/**
	 * Computes the diff between two snapshots.
	 * @param OldSnapshot The earlier (baseline) snapshot.
	 * @param NewSnapshot The later snapshot.
	 * @param bRemoveKnownLeaks Remove entries classified as known-leaks from the result if true.
	 * @return A fully populated FNObjectSnapshotDiff describing the Added/Maintained/Removed sets.
	 */
	static FNObjectSnapshotDiff Diff(FNObjectSnapshot OldSnapshot, FNObjectSnapshot NewSnapshot, bool bRemoveKnownLeaks = false);

	/**
	 * Strips known-leak entries from a diff in place so only real churn remains.
	 * @param Diff The diff to filter; its Added/Removed arrays are mutated.
	 */
	static void RemoveKnownLeaks(FNObjectSnapshotDiff& Diff);

	/** Captures a snapshot and persists it to disk so it can be restored in a later session. */
	static void SnapshotToDisk();

	/** Releases the in-memory cached snapshot held by CacheSnapshot(). */
	static void ClearCachedSnapshot();

	/** Captures a snapshot and caches it in memory for later use by CompareSnapshotToDisk(). */
	static void CacheSnapshot();

	/** Compares the cached in-memory snapshot to the last one written to disk and logs the result. */
	static void CompareSnapshotToDisk();

private:
	static int32 SnapshotTicket;
	static FNObjectSnapshot CachedSnapshot;
};
