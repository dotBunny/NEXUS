// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Operations/NWorldAssemblyEditorTagOperations.h"

#include "Editor.h"
#include "ScopedTransaction.h"
#include "Selection.h"

/**
 * @param Tag The actor tag to test for.
 * @return true when at least one selected actor already carries Tag.
 * @note Deliberately "any" rather than "all": see the note on ToggleTagOnSelection for why a mixed selection removes.
 */
static bool IsTagOnAnySelectedActor(const FName Tag)
{
	// Non-const: FSelectionIterator only takes a mutable USelection&, even to read.
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection == nullptr) return false;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		if (const AActor* Actor = Cast<AActor>(*It); Actor != nullptr && Actor->ActorHasTag(Tag))
		{
			return true;
		}
	}
	return false;
}

void FNWorldAssemblyEditorTagOperations::ToggleTagOnSelection(const FName Tag, const FText& AddTransaction, const FText& RemoveTransaction)
{
	const bool bRemove = IsTagOnAnySelectedActor(Tag);

	const FScopedTransaction Transaction(bRemove ? RemoveTransaction : AddTransaction);

	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection == nullptr) return;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (Actor == nullptr) continue;

		Actor->Modify(true);
		if (bRemove)
		{
			Actor->Tags.RemoveSwap(Tag);
		}
		else
		{
			Actor->Tags.Add(Tag);
		}
	}
}
