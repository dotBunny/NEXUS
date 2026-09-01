// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorTagUtils.h"

#include "Editor.h"
#include "Components/ActorComponent.h"
#include "ScopedTransaction.h"
#include "Selection.h"

/**
 * @param Tag The actor tag to test for.
 * @return true when at least one selected actor already carries Tag.
 * @note Deliberately "any" rather than "all": see the note on ToggleTagOnSelection for why a mixed selection removes.
 */
bool FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedActor(const FName Tag)
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

void FNWorldAssemblyEditorTagUtils::ToggleTagOnSelection(const FName Tag, const FText& AddTransaction, const FText& RemoveTransaction)
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

bool FNWorldAssemblyEditorTagUtils::HasComponentsSelected()
{
	const USelection* Selection = GEditor->GetSelectedComponents();
	return Selection != nullptr && Selection->Num() > 0;
}

bool FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedComponent(const FName Tag)
{
	// Non-const: FSelectionIterator only takes a mutable USelection&, even to read.
	USelection* Selection = GEditor->GetSelectedComponents();
	if (Selection == nullptr) return false;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		if (const UActorComponent* Component = Cast<UActorComponent>(*It);
			Component != nullptr && Component->ComponentHasTag(Tag))
		{
			return true;
		}
	}
	return false;
}

void FNWorldAssemblyEditorTagUtils::ToggleTagOnComponentSelection(const FName Tag, const FText& AddTransaction,
	const FText& RemoveTransaction)
{
	const bool bRemove = IsTagOnAnySelectedComponent(Tag);

	const FScopedTransaction Transaction(bRemove ? RemoveTransaction : AddTransaction);

	USelection* Selection = GEditor->GetSelectedComponents();
	if (Selection == nullptr) return;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		UActorComponent* Component = Cast<UActorComponent>(*It);
		if (Component == nullptr) continue;

		Component->Modify(true);
		if (bRemove)
		{
			Component->ComponentTags.RemoveSwap(Tag);
		}
		else
		{
			Component->ComponentTags.Add(Tag);
		}

		// The tag changes what a collision gather emits for this component, and the level is what stores that. An
		// instance component belongs to its owning actor's package, which a Modify on the component alone does not
		// reach, so the actor is marked too.
		if (AActor* Owner = Component->GetOwner())
		{
			Owner->MarkPackageDirty();
		}
	}
}
