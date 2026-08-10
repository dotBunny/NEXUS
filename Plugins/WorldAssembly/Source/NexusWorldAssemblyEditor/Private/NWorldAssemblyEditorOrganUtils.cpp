// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorOrganUtils.h"

#include "Editor.h"
#include "EditorViewportClient.h"
#include "ScopedTransaction.h"
#include "Assembly/NAssemblyOperation.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyRegistry.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganVolume.h"

void FNWorldAssemblyEditorOrganUtils::AddVolume()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr) return;

	// No unsaved-world guard, unlike the cell's AddActor: a cell writes a side-car asset keyed on its level's package
	// path, so it needs one that exists. An organ volume is just an actor in the level and has nothing to write.
	const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorOrganOperations_AddVolume", "Add Organ Volume"));

	ANOrganVolume* SpawnedVolume = CurrentWorld->SpawnActor<ANOrganVolume>(
		ANOrganVolume::StaticClass(), FTransform::Identity, FActorSpawnParameters());
	if (SpawnedVolume == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to spawn the ANOrganVolume in the current world."));
		return;
	}

	// Dropped in front of the viewport rather than at the origin: a volume is placed to bound a region the user is
	// looking at, and one spawned at the world origin is easy to miss entirely in a large level.
	if (const FEditorViewportClient* ViewportClient = GEditor->GetActiveViewport() != nullptr
		? static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient())
		: nullptr)
	{
		constexpr double SpawnDistance = 500.0;
		SpawnedVolume->SetActorLocation(ViewportClient->GetViewLocation()
			+ ViewportClient->GetViewRotation().Vector() * SpawnDistance);
	}

	GEditor->SelectNone(false, true);
	GEditor->SelectActor(SpawnedVolume, true, true, true);
}

void FNWorldAssemblyEditorOrganUtils::GenerateProxies()
{
	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	UNWorldAssemblyEditorSubsystem::Get()->StartOperation(
		UNAssemblyOperation::CreateInstance(FNWorldAssemblyEditorUtils::GetSelectedOrganComponents(), EditorSettings));
}

void FNWorldAssemblyEditorOrganUtils::GenerateAllProxies()
{
	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	UNWorldAssemblyEditorSubsystem::Get()->StartOperation(
		UNAssemblyOperation::CreateInstance(FNWorldAssemblyRegistry::GetOrganComponents(), EditorSettings));
}

void FNWorldAssemblyEditorOrganUtils::ClearGenerated()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->ClearGenerated(Component->GetAndResetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorOrganUtils::ClearAllProxies()
{
	UNWorldAssemblyEditorSubsystem::Get()->ClearAllProxies();
}

void FNWorldAssemblyEditorOrganUtils::LoadProxyLevels()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (const UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->LoadGeneratedProxies(Component->GetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorOrganUtils::LoadAllProxyLevels()
{
	UNWorldAssemblyEditorSubsystem::Get()->LoadAllGeneratedProxies();
}

void FNWorldAssemblyEditorOrganUtils::UnloadProxyLevels()
{
	TArray<UNOrganComponent*> SelectedOrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	for (const UNOrganComponent* Component : SelectedOrganComponents)
	{
		Subsystem->UnloadGeneratedProxies(Component->GetLastOperationTicket());
	}
}

void FNWorldAssemblyEditorOrganUtils::UnloadAllProxyLevels()
{
	UNWorldAssemblyEditorSubsystem::Get()->UnloadAllGeneratedProxies();
}

void FNWorldAssemblyEditorOrganUtils::SelectComponent(UNOrganComponent* Organ)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Organ, true, true, true);
}
