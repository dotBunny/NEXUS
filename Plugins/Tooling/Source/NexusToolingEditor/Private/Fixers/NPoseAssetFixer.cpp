// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Fixers/NPoseAssetFixer.h"
#include "EditorUtilityLibrary.h"
#include "NEditorUtils.h"
#include "NToolingEditorMinimal.h"
#include "PackageTools.h"
#include "Animation/PoseAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Subsystems/EditorAssetSubsystem.h"

void FNPoseAssetFixer::OutOfDateAnimationSource(bool bIsContextMenu)
{
	FScopedSlowTask MainTask = FScopedSlowTask(4, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource", "Find & Fix: Outdated PoseAssets Animation Sources"));
	MainTask.MakeDialog(false);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

	// STEP 1 - Collect parts where we will scan
	TArray<FString> SelectedPaths;
	TArray<UObject*> PoseAssets;
	TArray<UPackage*> CleanupPackages;
	
	if (bIsContextMenu)
	{
		MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step1", "Collecting ..."));
		PoseAssets = UEditorUtilityLibrary::GetSelectedAssetsOfClass(UPoseAsset::StaticClass());
		SelectedPaths = FNEditorUtils::GetSelectedContentBrowserPaths();
		for (FString AdditionalPath : UEditorUtilityLibrary::GetSelectedPathViewFolderPaths())
		{
			SelectedPaths.AddUnique(AdditionalPath);
		}
	}
	else
	{
		MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step1", "Collecting Everything ..."));
		SelectedPaths = { TEXT("/Game") };
	}
	const int PathCount = SelectedPaths.Num();

	// STEP 2 - Look for PoseAssets
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2", "Scanning ..."));
	FScopedSlowTask ScanTask = FScopedSlowTask(PathCount, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2_Scan", "Scanning For Pose Assets"));
	ScanTask.MakeDialog(false);
	for (int i = 0; i < PathCount; i++)
	{
		// Plugins should be their root path
		SelectedPaths[i].RemoveFromStart("/All/Plugins");
		// We just dont want the all, just incase.
		SelectedPaths[i].RemoveFromStart("/All");
		
		ScanTask.EnterProgressFrame(1, FText::Format(NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2_Item", "Scanning {0}"), FText::FromString(SelectedPaths[i])));
		
		if (!EditorAssetSubsystem->DoesDirectoryExist(SelectedPaths[i]))
		{
			continue;
		}

		for (FString AssetPath : EditorAssetSubsystem->ListAssets(SelectedPaths[i], true))
		{
			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(AssetPath));
			if (AssetData.IsValid() && AssetData.AssetClassPath == UPoseAsset::StaticClass()->GetClassPathName())
			{
				UObject* LoadedAsset = EditorAssetSubsystem->LoadAsset(AssetPath);
				PoseAssets.AddUnique(LoadedAsset);
				CleanupPackages.AddUnique(LoadedAsset->GetOutermost());
			}
		}
	}

	// Step 3 - Process Poses
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step3", "Processing ..."));
	FScopedSlowTask FixTask = FScopedSlowTask(PoseAssets.Num(), NSLOCTEXT("NexusToolingEditor", "OutOfDateAnimationSource_Fix", "Processing Pose Assets"));
	FixTask.MakeDialog(true);
	int ProblemsFixed = 0;
	for (int i = 0; i < PoseAssets.Num(); i++)
	{
		UPoseAsset* PoseAsset = Cast<UPoseAsset>(PoseAssets[i]);
		
		FixTask.EnterProgressFrame(1, FText::Format(NSLOCTEXT("NexusToolingEditor", "OutOfDateAnimationSource_Fix_Item", "Processing {0}"), FText::FromString(PoseAssets[i]->GetName())));

		UE_LOG(LogNexusToolingEditor, Log, TEXT("Updating out-of-date UPose(%s)."), *PoseAsset->GetName());
		
		if (UpdatePoseAsset(EditorAssetSubsystem, PoseAsset, CleanupPackages))
		{
			ProblemsFixed++;
		}
		
		if (GWarn->ReceivedUserCancel())
		{
			FixTask.Destroy();
			break;
		}
	}
	UE_LOG(LogNexusToolingEditor, Log, TEXT("Fixed %i out-of-date UPose assets."), ProblemsFixed);

	// Step 4 - Cleanup
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusToolingEditor", "FindAndFix_PoseAssets_OutOfDateAnimationSource_Step4", "Cleanup ..."));
	UPackageTools::UnloadPackages(CleanupPackages);
}

bool FNPoseAssetFixer::UpdatePoseAsset(UEditorAssetSubsystem* EditorAssetSubsystem, UPoseAsset* PoseAsset, TArray<UPackage*>& CleanupPackages)
{
	if (PoseAsset->SourceAnimation == nullptr)
	{
		return false;
	}
	
	if(PoseAsset->SourceAnimation->HasAnyFlags(RF_NeedLoad))
	{
		FLinkerLoad* Linker = PoseAsset->SourceAnimation->GetLinker();
		if (Linker != nullptr)
		{
			Linker->Preload(PoseAsset->SourceAnimation);
		}
		CleanupPackages.AddUnique(PoseAsset->SourceAnimation->GetOutermost());
	}
	PoseAsset->SourceAnimation->ConditionalPostLoad();
		
	if (PoseAsset->SourceAnimationRawDataGUID.IsValid() && PoseAsset->SourceAnimationRawDataGUID != PoseAsset->SourceAnimation->GetDataModel()->GenerateGuid())
	{
		PoseAsset->UpdatePoseFromAnimation(PoseAsset->SourceAnimation);
		
		// ReSharper disable once CppExpressionWithoutSideEffects
		PoseAsset->MarkPackageDirty();
		EditorAssetSubsystem->SaveLoadedAsset(PoseAsset);
		return true;
	}
	
	return false;
}
