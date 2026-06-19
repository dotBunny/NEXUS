// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commandlets/NUpdateCellDataCommandlet.h"

#include "FileHelpers.h"
#include "ISourceControlModule.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorUtils.h"
#include "SourceControlOperations.h"
#include "Cell/NCell.h"
#include "Subsystems/EditorAssetSubsystem.h"

UNUpdateCellDataCommandlet::UNUpdateCellDataCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UNUpdateCellDataCommandlet::Main(const FString& Params)
{
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamMap;
	ParseCommandLine(*Params, Tokens, Switches, ParamMap);

	return Execute(
		Switches.Contains(TEXT("ErrorOnChanges")), 
			Switches.Contains(TEXT("CommitChanges")));
}

int32 UNUpdateCellDataCommandlet::Execute(bool bShouldErrorOnChanges, bool bShouldCommitChanges)
{
	TArray<FString> ChangedAssetPaths;
	TArray<FAssetData> CellAssetData = FNWorldAssemblyEditorUtils::GetAllCellDataAssetData(true);
	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	TArray<FString> CommitPaths;
	int32 ReturnCode = 0;
	
	// Process Cell Assets
	for (auto AssetData : CellAssetData)
	{
		if (!AssetData.IsValid())
		{
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("NCell(%s) @ %s is invalid."),* AssetData.AssetName.ToString(),  *AssetData.GetObjectPathString());
			continue;
		}
		
		UObject* LoadedAsset = EditorAssetSubsystem->LoadAsset(AssetData.GetObjectPathString());
		if (IsValid(LoadedAsset))
		{
			UNCell* Cell = Cast<UNCell>(LoadedAsset);
		
			if (Cell == nullptr) continue;
			
			int32 PreviousVersion = Cell->GetVersion();
			FString WorldPath = Cell->World.GetLongPackageName();
			
			if (FEditorFileUtils::LoadMap(WorldPath))
			{
				FNWorldAssemblyEditorUtils::SaveCell(FNEditorUtils::GetCurrentWorld());
				if (Cell->GetVersion() != PreviousVersion)
				{
					FString FullPath = FNEditorUtils::GetAssetPathOnDisk(Cell);
					CommitPaths.Add(FullPath);
					
					if (bShouldErrorOnChanges)
					{
						UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("NCell(%s) is not up-to-date @ %s"),*Cell->GetName(), *FullPath);
						ReturnCode = 1;
					}
					else
					{
						UE_LOG(LogNexusWorldAssemblyEditor, Log, TEXT("Updated to NCell(%s) @ %s"),*Cell->GetName(), *FullPath);
					}
				}
			}
			else
			{
				UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Failed to load World(%s)."), *WorldPath)
				ReturnCode = 1;
			}
			LoadedAsset->MarkAsGarbage();
		}
	}
	
	// Should we use UE's VCS settings to commit changes (CI/CD)
	if (bShouldCommitChanges && CommitPaths.Num() > 0)
	{
		ISourceControlModule& SourceControlModule = ISourceControlModule::Get();
    
		if (!SourceControlModule.IsEnabled())
		{
			UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Source control is not enabled in this project, cannot commit changes."));
			return 1;
		}
		
		ISourceControlProvider& SourceControlProvider = SourceControlModule.GetProvider();
		SourceControlProvider.Init();
		TSharedRef<FCheckIn, ESPMode::ThreadSafe> CheckInOperation = ISourceControlOperation::Create<FCheckIn>();
		
		FText ChangelistDescription = FText::FromString(FString::Printf(TEXT("#NEXUS #AUTOMATION UNUpdateCellDataCommandlet: Updating %i Cells"), CommitPaths.Num()));
		CheckInOperation->SetDescription(ChangelistDescription);

		UE_LOG(LogNexusWorldAssemblyEditor, Log, TEXT("Submitting %d file(s) to version control..."), CommitPaths.Num());
		
		ECommandResult::Type Result = SourceControlProvider.Execute(CheckInOperation, CommitPaths, EConcurrency::Synchronous);
		
		if (Result == ECommandResult::Succeeded)
		{
			UE_LOG(LogNexusWorldAssemblyEditor, Log, TEXT("Successfully submitted changes to version control!"));
			return ReturnCode;
		}
		else if (Result == ECommandResult::Cancelled)
		{
			UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Source control submission was cancelled."));
			return 1;
		}
    
		UE_LOG(LogNexusWorldAssemblyEditor, Error, TEXT("Failed to submit changes to version control. Check log for details."));
		return 1;
	}
	
	return ReturnCode;
}

