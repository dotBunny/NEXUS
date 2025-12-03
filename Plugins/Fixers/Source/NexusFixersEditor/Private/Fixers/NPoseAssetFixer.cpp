// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Fixers/NPoseAssetFixer.h"
#include "NCoreEditorMinimal.h"
#include "EditorUtilityLibrary.h"
#include "NEditorUtils.h"
#include "PackageTools.h"
#include "Animation/PoseAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Subsystems/EditorAssetSubsystem.h"

#define LOCTEXT_NAMESPACE "NexusFixersEditor"

void FNPoseAssetFixer::OutOfDateAnimationSource(bool bIsContextMenu)
{
	FScopedSlowTask MainTask = FScopedSlowTask(4, LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource", "Find & Fix: Outdated PoseAssets Animation Sources"));
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
		MainTask.EnterProgressFrame(1,LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step1", "Collecting ..."));
		PoseAssets = UEditorUtilityLibrary::GetSelectedAssetsOfClass(UPoseAsset::StaticClass());
		NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Finding all PoseAssets from selection ...");
		SelectedPaths = FNEditorUtils::GetSelectedContentBrowserPaths();
		for (FString AdditionalPath : UEditorUtilityLibrary::GetSelectedPathViewFolderPaths())
		{
			SelectedPaths.AddUnique(AdditionalPath);
		}
	}
	else
	{
		MainTask.EnterProgressFrame(1,LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step1", "Collecting Everything ..."));
		SelectedPaths = { TEXT("/Game") };
		NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Finding all PoseAssets in Game ...");
	}
	const int PathCount = SelectedPaths.Num();

	// STEP 2 - Look for PoseAssets
	MainTask.EnterProgressFrame(1, LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2", "Scanning ..."));
	FScopedSlowTask ScanTask = FScopedSlowTask(PathCount, LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2_Scan", "Scanning For Pose Assets"));
	ScanTask.MakeDialog(false);
	for (int i = 0; i < PathCount; i++)
	{
		SelectedPaths[i].RemoveFromStart(TEXT("/All"));
		ScanTask.EnterProgressFrame(1, FText::Format(LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step2_Item", "Scanning {0}"), FText::FromString(SelectedPaths[i])));
		if (!EditorAssetSubsystem->DoesDirectoryExist(SelectedPaths[i]))
		{
			NE_LOG_WARNING("[FNPoseAssetFixer::OutOfDateAnimationSource] Directory %s does not exist!", *SelectedPaths[i]);
			continue;
		}

		NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Scanning %s ...", *SelectedPaths[i]);
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
	NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Found %i PoseAssets", PoseAssets.Num());

	// Step 3 - Process Poses
	MainTask.EnterProgressFrame(1, LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step3", "Processing ..."));
	FScopedSlowTask FixTask = FScopedSlowTask(PoseAssets.Num(), LOCTEXT("OutOfDateAnimationSource_Fix", "Processing Pose Assets"));
	FixTask.MakeDialog(true);
	int ProblemsFixed = 0;
	for (int i = 0; i < PoseAssets.Num(); i++)
	{
		UPoseAsset* PoseAsset = Cast<UPoseAsset>(PoseAssets[i]);
		
		FixTask.EnterProgressFrame(1, FText::Format(LOCTEXT("OutOfDateAnimationSource_Fix_Item", "Processing {0}"), FText::FromString(PoseAssets[i]->GetName())));

		NE_LOG("Processing %s", *PoseAsset->GetName());
		
		if (PoseAsset->SourceAnimation != nullptr)
		{
			if(PoseAsset->SourceAnimation->HasAnyFlags(RF_NeedLoad))
			{
				if (FLinkerLoad* Linker = PoseAsset->SourceAnimation->GetLinker())
				{
					Linker->Preload(PoseAsset->SourceAnimation);
				}
				CleanupPackages.AddUnique(PoseAsset->SourceAnimation->GetOutermost());
			}
			PoseAsset->SourceAnimation->ConditionalPostLoad();
			
			if (PoseAsset->SourceAnimationRawDataGUID.IsValid() && PoseAsset->SourceAnimationRawDataGUID != PoseAsset->SourceAnimation->GetDataModel()->GenerateGuid())
			{
				PoseAsset->UpdatePoseFromAnimation(PoseAsset->SourceAnimation);
				PoseAsset->MarkPackageDirty();
				EditorAssetSubsystem->SaveLoadedAsset(PoseAsset);
				ProblemsFixed++;
			}
		}
		
		if (GWarn->ReceivedUserCancel())
		{
			FixTask.Destroy();
			break;
		}
	}
	NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Fixed %i Problems.", ProblemsFixed);

	// Step 4 - Cleanup
	MainTask.EnterProgressFrame(1, LOCTEXT("FindAndFix_PoseAssets_OutOfDateAnimationSource_Step4", "Cleanup ..."));
	UPackageTools::UnloadPackages(CleanupPackages);
	
	NE_LOG("[FNPoseAssetFixer::OutOfDateAnimationSource] Done.");
}

bool FNPoseAssetFixer::OutOfDateAnimationSource_CanExecute()
{
	return true;
}

#undef LOCTEXT_NAMESPACE