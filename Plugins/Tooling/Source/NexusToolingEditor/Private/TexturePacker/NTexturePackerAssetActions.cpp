// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerAssetActions.h"

#include "ContentBrowserMenuContexts.h"
#include "NToolingEditorStyle.h"
#include "ToolMenus.h"
#include "Engine/Texture2D.h"
#include "TexturePacker/NTexturePackerLibrary.h"
#include "TexturePacker/NTexturePackerTab.h"
#include "TexturePacker/NTexturePackerUtils.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker::AssetActions
{
	/** The menu the entries are installed on, and the section and submenu they occupy. */
	const FName MenuName = TEXT("ContentBrowser.AssetContextMenu.Texture2D");
	const FName SectionName = TEXT("NEXUS");
	const FName SubMenuName = TEXT("NEXUS_TexturePacker");

	/** @return The textures the Content Browser has selected, loaded. */
	TArray<UTexture2D*> GetSelection(const UToolMenu* Menu)
	{
		const UContentBrowserAssetContextMenuContext* Context =
			Menu->FindContext<UContentBrowserAssetContextMenuContext>();

		return Context != nullptr ? Context->LoadSelectedObjects<UTexture2D>() : TArray<UTexture2D*>();
	}

	/** Shows what a finished run produced, since a Content Browser action has no panel to report into. */
	void ReportResult(const FNTexturePackerResult& Result)
	{
		if (!Result.bSucceeded) return;

		TArray<UObject*> Written;
		Written.Reserve(Result.Textures.Num());
		for (const TObjectPtr<UTexture2D>& Texture : Result.Textures)
		{
			Written.Add(Texture);
		}

		FNTexturePackerUtils::SelectAssetsInContentBrowser(Written);
	}
}

void FNTexturePackerAssetActions::Register()
{
	using namespace NEXUS::ToolingEditor::TexturePacker::AssetActions;

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(MenuName);
	if (Menu == nullptr) return;

	FToolMenuSection& Section = Menu->FindOrAddSection(SectionName,
		LOCTEXT("Actions_Section", "NEXUS"));

	Section.AddSubMenu(
		SubMenuName,
		LOCTEXT("Actions_SubMenu", "Texture Packer"),
		LOCTEXT("Actions_SubMenu_Tooltip", "Pack or unpack the selected textures."),
		FNewToolMenuChoice(FNewToolMenuDelegate::CreateStatic(&FNTexturePackerAssetActions::BuildSubMenu)),
		false,
		FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.TexturePacker"));
}

void FNTexturePackerAssetActions::Unregister()
{
	using namespace NEXUS::ToolingEditor::TexturePacker::AssetActions;

	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		ToolMenus->RemoveEntry(MenuName, SectionName, SubMenuName);
	}
}

void FNTexturePackerAssetActions::BuildSubMenu(UToolMenu* Menu)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::AssetActions;

	// Read once here rather than in each action: the submenu is rebuilt every time it opens, so this is
	// current, and the entries below close over it instead of each reloading the same assets.
	const TArray<UTexture2D*> Selection = GetSelection(Menu);
	const bool bCanPack = Selection.Num() >= 1 && Selection.Num() <= 4;
	const bool bHasSelection = Selection.Num() > 0;

	FToolMenuSection& Section = Menu->AddSection(TEXT("TexturePacker"));

	Section.AddMenuEntry(
		TEXT("PackAutomatic"),
		LOCTEXT("Actions_PackAuto", "Pack"),
		LOCTEXT("Actions_PackAuto_Tooltip",
			"Pack the selected textures, working out the arrangement and the name from what they are called."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([Selection]()
				{
					ReportResult(UNTexturePackerLibrary::PackTextures(Selection));
				}),
			FCanExecuteAction::CreateLambda([bCanPack]() { return bCanPack; })));

	Section.AddMenuEntry(
		TEXT("UnpackAutomatic"),
		LOCTEXT("Actions_UnpackAuto", "Unpack"),
		LOCTEXT("Actions_UnpackAuto_Tooltip",
			"Unpack every selected texture whose name says what it holds, skipping the ones that do not."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([Selection]()
				{
					ReportResult(UNTexturePackerLibrary::UnpackTextures(Selection));
				}),
			FCanExecuteAction::CreateLambda([bHasSelection]() { return bHasSelection; })));

	Section.AddSeparator(TEXT("OpenSeparator"));

	Section.AddMenuEntry(
		TEXT("OpenPack"),
		LOCTEXT("Actions_OpenPack", "Open in Texture Packer"),
		LOCTEXT("Actions_OpenPack_Tooltip",
			"Open the window with the selection assigned to channels, to review before writing anything."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([Selection]()
				{
					FNTexturePackerTab::OpenForPack(Selection);
				}),
			FCanExecuteAction::CreateLambda([bCanPack]() { return bCanPack; })));

	Section.AddMenuEntry(
		TEXT("OpenUnpack"),
		LOCTEXT("Actions_OpenUnpack", "Open in Texture Packer (Unpack)"),
		LOCTEXT("Actions_OpenUnpack_Tooltip",
			"Open the window on the first selected texture, to say what it holds before writing anything."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([Selection]()
				{
					FNTexturePackerTab::OpenForUnpack(Selection.Num() > 0 ? Selection[0] : nullptr);
				}),
			FCanExecuteAction::CreateLambda([bHasSelection]() { return bHasSelection; })));
}

#undef LOCTEXT_NAMESPACE
