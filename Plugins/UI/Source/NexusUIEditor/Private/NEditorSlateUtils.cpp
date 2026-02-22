// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorSlateUtils.h"
#include "LevelEditor.h"
#include "NSlateUtils.h"

TSharedPtr<SDockTab> FNEditorSlateUtils::FindDocTab(const TSharedPtr<SWidget>& BaseWidget, const FText& TargetLabel,
                                                    const FName TabIdentifier)
{
	TSharedPtr<SDockTab> RuntimeTab = FNSlateUtils::FindDocTabWithLabel(BaseWidget, TargetLabel);
	if (RuntimeTab.IsValid())
	{
		return RuntimeTab;
	}
	return FindDocTabByIdentifier(TabIdentifier);
}

TSharedPtr<SDockTab> FNEditorSlateUtils::FindDocTabByIdentifier(const FName TabIdentifier)
{
	const FName ActiveTabIdentifier = FName(FString::Printf(TEXT("%s_ActiveTab"), *TabIdentifier.ToString()));
	
	// Try the global tabs (nomad/etc)
	{
		const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();
	
		const TSharedPtr<SDockTab> GlobalTab = GlobalTabManager->FindExistingLiveTab(TabIdentifier);
		if (GlobalTab.IsValid())
		{
			return GlobalTab;
		}
	
		const TSharedPtr<SDockTab> GlobalActiveTab = GlobalTabManager->FindExistingLiveTab(ActiveTabIdentifier);
		if (GlobalActiveTab.IsValid())
		{
			return GlobalActiveTab;
		}
	}
	
	// Try the level editor
	{
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
	}
	
	return nullptr;
}
