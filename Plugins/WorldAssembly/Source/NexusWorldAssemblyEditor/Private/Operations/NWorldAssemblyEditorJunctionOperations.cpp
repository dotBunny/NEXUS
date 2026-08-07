// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Operations/NWorldAssemblyEditorJunctionOperations.h"

#include "Editor.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "Misc/MessageDialog.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyRegistry.h"
#include "Subsystems/EditorActorSubsystem.h"

void FNWorldAssemblyEditorJunctionOperations::AddComponent()
{
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorJunctionOperations_AddComponent", "Add Junction Component"));
	TArray<UNCellJunctionComponent*> OutComponents;
	TArray<UNCellJunctionComponent*> SelectComponents;
	bool bNeedsRefresh = false;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		AActor* Actor = Cast<AActor>( *SelectedActor );
		if (!Actor) continue;

		auto NewComponent = static_cast<UNCellJunctionComponent*>(Actor->AddComponentByClass(
			UNCellJunctionComponent::StaticClass(), true, FTransform::Identity, false));

		Actor->Modify();
		NewComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		Actor->AddInstanceComponent(NewComponent);
		NewComponent->SetFlags(RF_Transactional);
		NewComponent->RegisterComponent();

		SelectComponents.Add(NewComponent);
		bNeedsRefresh = true;

	}

	// Refresh the details panel if needed
	if (bNeedsRefresh)
	{
		GEditor->SelectNone(false, true);
		for (UNCellJunctionComponent* Component : SelectComponents)
		{
			GEditor->SelectComponent(Component, true, true, true);
		}
	}
}

void FNWorldAssemblyEditorJunctionOperations::SelectComponent(UNCellJunctionComponent* Junction)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Junction, true, true, true);
}

void FNWorldAssemblyEditorJunctionOperations::CollectComponents()
{
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	if (EditorActorSubsystem)
	{
		TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
		if (SelectedActors.Num() > 0)
		{
			AActor* TargetActor = SelectedActors[0];

			// A location-locked actor (such as the ANCellActor, which pins itself to the origin) propagates its lock to
			// every scene component it owns: USceneComponent::CanEditChange disables RelativeLocation/RelativeRotation/
			// RelativeScale3D on components owned by a location-locked actor. Collecting junctions onto such an actor would
			// leave them unmovable in the editor (Details transform greyed out, scale gizmo disabled), so bail out with a
			// warning rather than silently breaking junction authoring.
			if (TargetActor->IsLockLocation())
			{
				const TCHAR* TargetTypeName = TargetActor->IsA<ANCellActor>()
					? TEXT("NEXUS Cell Actor")
					: TEXT("location-locked actor");

				UE_LOG(LogNexusWorldAssemblyEditor, Warning,
					TEXT("Aborting Collect Junction Components: target actor '%s' is a %s and is location-locked; junctions parented to it would become unmovable in the editor."),
					*TargetActor->GetActorLabel(), TargetTypeName);

				const FText WarningMessage = FText::FromString(FString::Printf(
					TEXT("'%s' is a %s and is locked to its location.\n\nCollecting junctions onto it would lock their transforms — Unreal disables moving, rotating, and scaling components owned by a location-locked actor, so the junctions could no longer be edited.\n\nSelect a non-locked actor as the collection target instead."),
					*TargetActor->GetActorLabel(), TargetTypeName));

				FMessageDialog::Open(EAppMsgCategory::Warning, EAppMsgType::Type::Ok,
					WarningMessage, FText::FromString(TEXT("NEXUS: World Assembly")));
				return;
			}

			// Snapshot the registry: reparenting unregisters/re-registers each junction, which mutates the live
			// registry array (via OnUnregister/OnRegister), so iterating the reference directly would skip entries.
			TArray<UNCellJunctionComponent*> CellJunctions = FNWorldAssemblyRegistry::GetCellJunctionComponents();
			if (CellJunctions.Num() == 0)
			{
				return;
			}

			// Confirm the move with the user, listing the junctions and their destination actor.
			constexpr int32 MaxListedJunctions = 25;
			FString JunctionList;
			for (int32 Index = 0; Index < CellJunctions.Num(); ++Index)
			{
				if (Index >= MaxListedJunctions)
				{
					JunctionList.Append(FString::Printf(TEXT("\n  - ...and %d more"), CellJunctions.Num() - MaxListedJunctions));
					break;
				}
				JunctionList.Append(FString::Printf(TEXT("\n  - %s"), *CellJunctions[Index]->GetJunctionName()));
			}

			const FText ConfirmMessage = FText::FromString(FString::Printf(
				TEXT("This will move %d junction(s) to '%s':\n%s\n\nDo you wish to proceed?"),
				CellJunctions.Num(), *TargetActor->GetActorLabel(), *JunctionList));

			const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgCategory::Warning, EAppMsgType::Type::YesNo,
				ConfirmMessage, FText::FromString(TEXT("NEXUS: World Assembly")));
			if (Choice != EAppReturnType::Yes)
			{
				return;
			}

			const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorJunctionOperations_CollectComponents", "Collect Junction Components"));
			TargetActor->Modify();
			const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
			for (UNCellJunctionComponent* Component : CellJunctions)
			{
				AActor* PreviousOwner = Component->GetOwner();

				Component->SetFlags(RF_Transactional);
				Component->Modify();
				if (PreviousOwner)
				{
					PreviousOwner->Modify();
				}

				// Tear down render/physics state before reparenting the component.
				Component->UnregisterComponent();
				Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				// Drop the component from the previous owner's bookkeeping.
				if (PreviousOwner)
				{
					PreviousOwner->RemoveInstanceComponent(Component);
					PreviousOwner->RemoveOwnedComponent(Component);
				}

				// Reparent the UObject to the target actor and register with its bookkeeping + scene.
				Component->Rename(nullptr, TargetActor, REN_DontCreateRedirectors);
				TargetActor->AddOwnedComponent(Component);
				TargetActor->AddInstanceComponent(Component);
				Component->AttachToComponent(TargetActor->GetRootComponent(), AttachmentRules);
				Component->RegisterComponent();
			}

			// Flag the level so the change is picked up by Save.
			if (UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				CurrentWorld->MarkPackageDirty();
			}

			// Refresh the Details panel / components tree for the current selection without re-selecting the actor.
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.BroadcastComponentsEdited();
		}
	}
}
