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

	//~UCommandlet
	/**
	 * Commandlet entry point; parses the command line and runs the cell data update.
	 * @param Params Raw command-line string. Recognized switches: -ErrorOnChanges, -CommitChanges.
	 * @return 0 on success; non-zero if any cell was out-of-date (with -ErrorOnChanges), a world failed to load, or a commit failed.
	 */
	virtual int32 Main(const FString& Params) override;
	//End UCommandlet

	/**
	 * Re-saves every NCell asset so its cached data matches the current state of its referenced world.
	 * @param bShouldErrorOnChanges When true, out-of-date cells are logged as errors and the return code is set to 1 instead of being updated silently (validation/CI gating).
	 * @param bShouldCommitChanges When true, updated cell assets are submitted to the project's configured source control provider.
	 * @return 0 on success; 1 if a cell was out-of-date under bShouldErrorOnChanges, a world failed to load, or the source control commit failed.
	 * @remark Editor/commandlet use only; loads and saves maps, so it must run on the game thread.
	 */
	static int32 Execute(bool bShouldErrorOnChanges = true, bool bShouldCommitChanges = false);
};