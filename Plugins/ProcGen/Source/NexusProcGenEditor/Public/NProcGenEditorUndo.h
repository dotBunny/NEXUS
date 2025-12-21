// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNProcGenEditorUndo : public FSelfRegisteringEditorUndoClient
{
protected:
	virtual bool MatchesContext( const FTransactionContext& InContext, const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjectContexts ) const override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
private:
	static void UpdateKnownJunctions();
	static void CompareAgainstSidecar();
};
