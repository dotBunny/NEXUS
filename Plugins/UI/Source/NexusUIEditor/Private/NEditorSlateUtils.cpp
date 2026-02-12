#include "NEditorSlateUtils.h"

#include "LevelEditor.h"

TSharedPtr<SDockTab> FNEditorSlateUtils::FindDocTabByIdentifier(const FName TabIdentifier)
{
	FName ActiveTabIdentifier = FName(FString::Printf(TEXT("%s_ActiveTab"), *TabIdentifier.ToString()));
	const TSharedPtr<SDockTab> GlobalTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabIdentifier);
	if (GlobalTab.IsValid())
	{
		return GlobalTab;
	}
	const TSharedPtr<SDockTab> GlobalActiveTab = FGlobalTabmanager::Get()->FindExistingLiveTab(ActiveTabIdentifier);
	if (GlobalActiveTab.IsValid())
	{
		return GlobalActiveTab;
	}
	
	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
		const TSharedPtr<SDockTab> LevelEditorTab = LevelEditorTabManager->FindExistingLiveTab(TabIdentifier);
		if (LevelEditorTab.IsValid())
		{
			return LevelEditorTab;
		}
		const TSharedPtr<SDockTab> LevelEditorActiveTab = LevelEditorTabManager->FindExistingLiveTab(ActiveTabIdentifier);
		if (LevelEditorActiveTab.IsValid())
		{
			return LevelEditorActiveTab;
		}
	}
	
	return nullptr;
}
