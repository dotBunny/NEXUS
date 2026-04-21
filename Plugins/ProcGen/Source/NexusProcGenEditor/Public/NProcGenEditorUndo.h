// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Editor undo client for ProcGen — intercepts transactions that touch cell/junction state so
 * post-undo/redo the cell's side-car asset and junction bookkeeping stay consistent.
 */
class FNProcGenEditorUndo : public FSelfRegisteringEditorUndoClient
{
protected:
	virtual bool MatchesContext( const FTransactionContext& InContext, const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjectContexts ) const override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
private:
	/** Re-sync the known-junction list on the cell actor after an undo/redo that may have added/removed junctions. */
	static void UpdateKnownJunctions();

	/** Diff the cell's live component state against the side-car asset to decide whether regeneration is needed. */
	static void CompareAgainstSidecar();
};
