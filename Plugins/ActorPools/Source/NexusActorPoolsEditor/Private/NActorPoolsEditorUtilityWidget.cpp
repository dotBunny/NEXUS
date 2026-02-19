// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorUtilityWidget.h"

#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "NActorPoolObject.h"
#include "NActorPoolSet.h"
#include "NActorPoolSetFactory.h"
#include "NActorPoolsSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/NListView.h"

namespace NEXUS::ActorPoolsEditor::ActorPoolSet
{
	constexpr float NearestMultiple = 5.f;
}

void UNActorPoolsEditorUtilityWidget::NativeConstruct()
{
	if (CreateAPSButton != nullptr)
	{
		CreateAPSButton->OnClicked.AddDynamic(this, &UNActorPoolsEditorUtilityWidget::OnCreateActorPoolSet);
	}
	Super::NativeConstruct();
}

void UNActorPoolsEditorUtilityWidget::OnCreateActorPoolSet()
{
	// Don't have an overlay, don't do anything
	if (Overlay == nullptr) return;
	
	// Get some initial details
	TObjectPtr<UNListView> ActorPoolList = Overlay->GetActorPoolList();
	TArray<UObject*> ListItems = ActorPoolList->GetListItems();
	const int32 Count = ListItems.Num();
	
	// No pools, don't do anything
	if (Count == 0) return;
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UNActorPoolSetFactory* Factory = NewObject<UNActorPoolSetFactory>();
	UObject* TemplateObject = AssetTools.CreateAssetWithDialog(UNActorPoolSet::StaticClass(), Factory );
	
	// Cancelled
	if ( TemplateObject == nullptr) return;
	
	UNActorPoolSet* NewActorPoolSet = CastChecked<UNActorPoolSet>(TemplateObject);
	
	// Bad File?
	if ( NewActorPoolSet == nullptr) return;
	
	const UNActorPoolsSettings* DefaultSettings = UNActorPoolsSettings::Get();
	
	for (int32 i = 0; i < Count; i++)
	{
		UNActorPoolObject* PoolObject = Cast<UNActorPoolObject>(ListItems[i]);
		if (PoolObject != nullptr)
		{
			
			FNActorPoolDefinition NewDefinition;
			NewDefinition.Settings = DefaultSettings->DefaultSettings;
			
			NewDefinition.Settings.MinimumActorCount = FMath::CeilToInt((PoolObject->GetOutCount() + PoolObject->GetInCount()) / NEXUS::ActorPoolsEditor::ActorPoolSet::NearestMultiple) * NEXUS::ActorPoolsEditor::ActorPoolSet::NearestMultiple;
			NewDefinition.Settings.MaximumActorCount = NewDefinition.Settings.MinimumActorCount * 2;
			
			NewDefinition.ActorClass = PoolObject->GetTemplate();
			
			NewActorPoolSet->ActorPools.Add(NewDefinition);
		}
	}
	
	// Not going to mark package dirty cause forcing save
	UEditorAssetLibrary::SaveLoadedAsset(NewActorPoolSet, false);
	FAssetRegistryModule::AssetCreated(NewActorPoolSet);
}
