// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "NUpdateCellDataCommandlet.generated.h"

/**
 * Re-saves every NCell asset so its cached data reflects the current state of the cell's world.
 *
 * For each NCell, the commandlet loads the referenced world, regenerates the cell data, and detects whether
 * the cell's version changed. Intended for batch/CI use to keep cell data current and optionally commit any
 * resulting changes to source control.
 *
 * Supported command-line switches:
 * - -ErrorOnChanges     Logs out-of-date cells as errors instead of updating silently (validation/CI gating).
 * - -CommitChanges  Submits the updated cell assets to the project's configured source control provider.
 */
UCLASS()
class NEXUSWORLDASSEMBLYEDITOR_API UNUpdateCellDataCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UNUpdateCellDataCommandlet();

	/**
	 * Entry point invoked by the commandlet host.
	 * @param Params Raw command-line string; parsed for the -ErrorOnly and -CommitChanges switches.
	 * @return 0 on success, 1 if a source control commit was requested but could not be completed.
	 */
	virtual int32 Main(const FString& Params) override;
};