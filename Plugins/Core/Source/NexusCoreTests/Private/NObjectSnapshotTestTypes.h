// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// NOTE: Deliberately not wrapped in a preprocessor guard. UHT emits these types into the module's
// unscoped registration table, so any #if around a reflected type breaks non-editor targets where
// that define is 0. Guard test bodies with WITH_TESTS instead.

#include "CoreMinimal.h"
#include "NObjectSnapshotTestTypes.generated.h"

/**
 * Bare UObject used as the subject of the object-snapshot leak-detection tests.
 *
 * Deliberately empty, and deliberately not a UTexture2D or any other asset-backed type: those carry
 * render resources whose release is fenced against the render thread, so one CollectGarbage pass is
 * not guaranteed to reclaim them. The "an unreferenced object is collected, not leaked" control needs
 * a type that collection is certain to take on the first pass, or it would fail intermittently for a
 * reason that has nothing to do with the detector under test.
 *
 * Its other job is to be unmistakable in a diff: nothing outside these tests ever creates one, so a
 * snapshot entry of this class can only have come from the test that is running.
 */
UCLASS()
class NEXUSCORETESTS_API UNObjectSnapshotTestObject : public UObject
{
	GENERATED_BODY()
};
