// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotUtils.h"
#include "NObjectSnapshot.h"
#include "NObjectSnapshotDiff.h"

class NEXUSCORE_API FNObjectSnapshotUtils
{
public:
	static int32 TakeTicket() { return ++SnapshotTicket; }
	static FNObjectSnapshot Snapshot();
	static FNObjectSnapshotDiff Diff(FNObjectSnapshot OldSnapshot, FNObjectSnapshot NewSnapshot, bool bRemoveKnownLeaks = false);
	static void RemoveKnownLeaks(FNObjectSnapshotDiff& Diff);
private:
	static int32 SnapshotTicket;
};
