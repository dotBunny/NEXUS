// Copyright dotBunny Inc. All Rights Reserved.

#include "NUIEditorStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Macros/NEditorStyleMacros.h"
#include "Styling/SlateStyle.h"
#include "Styling/ToolBarStyle.h"

N_EDITOR_STYLE(FNUIEditorStyle)

TSharedRef<FSlateStyleSet> FNUIEditorStyle::Create()
{
	N_EDITOR_STYLE_CREATE

	Style.Set("Command.AddActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_AddActor"), Icon16x16));
	Style.Set("Command.Build", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Build"), Icon16x16));
	Style.Set("Command.RemoveActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_RemoveActor"), Icon16x16));
	Style.Set("Command.Reset", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_Reset"), Icon16x16));
	Style.Set("Command.SelectActor", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Command_SelectActor"), Icon16x16));

	Style.Set("Command.Select", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Select"), Icon20x20));
	Style.Set("Command.Tag", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Tag"), Icon20x20));
	Style.Set("Command.TagRemove", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_TagRemove"), Icon20x20));
	Style.Set("Command.Calculate", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_Calculate"), Icon20x20));
	Style.Set("Command.ToggleOn", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ToggleOn"), Icon20x20));
	Style.Set("Command.ToggleOff", new N_MODULE_IMAGE_BRUSH_SVG(PluginDirectory, TEXT("Commands/Command_ToggleOff"), Icon20x20));


	// RAIL
	const FSlateRoundedBoxBrush RailGroupBackground(FStyleColors::Recessed, 4.0f);
	Style.Set("Rail.GroupBackground", new FSlateRoundedBoxBrush(RailGroupBackground));

	FToolBarStyle PaletteStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("PaletteToolBar");
	PaletteStyle.SetBackground(RailGroupBackground);
	Style.Set("Rail.CommandPalette", PaletteStyle);

	FToolBarStyle CommandStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("SlimPaletteToolBar");
	CommandStyle.SetBackground(FSlateNoResource());
	CommandStyle.SetBackgroundPadding(FMargin(0.0f, 0.0f, 4.0f, 4.0f));
	CommandStyle.SetNumColumns(2);
	Style.Set("Rail.CommandGrid", CommandStyle);

	FToolBarStyle CommandListStyle = CommandStyle;
	CommandListStyle.SetNumColumns(1);
	CommandListStyle.ButtonStyle.SetNormal(FSlateNoResource());
	CommandListStyle.ToggleButton.SetUncheckedImage(FSlateNoResource());
	Style.Set("Rail.CommandList", CommandListStyle);

	Style.Set("Rail.GroupSeparator", new FSlateColorBrush(FStyleColors::Dropdown));

	FTextBlockStyle GroupSeparatorLabelStyle = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("SmallText");
	GroupSeparatorLabelStyle.SetColorAndOpacity(FStyleColors::White25);
	FSlateFontInfo GroupSeparatorLabelFont = GroupSeparatorLabelStyle.Font;
	GroupSeparatorLabelFont.Size = 7;
	GroupSeparatorLabelFont.TypefaceFontName = "Bold";
	GroupSeparatorLabelStyle.SetFont(GroupSeparatorLabelFont);
	Style.Set("Rail.GroupSeparatorLabel", GroupSeparatorLabelStyle);

	Style.Set("Rail.RailBackground",
		new FSlateRoundedBoxBrush(FStyleColors::Dropdown, 6.0f, FColor(85, 85, 85), 1.0f));

	FLinearColor PanelBackground = FStyleColors::Panel.GetSpecifiedColor();
	PanelBackground.A = 0.95f;
	Style.Set("Rail.PanelBackground",
		new FSlateRoundedBoxBrush(PanelBackground, 6.0f, FColor(85, 85, 85), 1.0f));

	FToolBarStyle CategoryToolBarStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("CategoryDrivenContentBuilderToolbarWithoutLabels");
	CategoryToolBarStyle.SetBackground(FSlateNoResource());
	CategoryToolBarStyle.SetBackgroundPadding(FMargin(0.0f));
	CategoryToolBarStyle.SetIconPadding(FMargin(4.0f));
	CategoryToolBarStyle.SetButtonContentMaxWidth(28.0f);
	CategoryToolBarStyle.SetButtonPadding(FMargin(2.0f));
	CategoryToolBarStyle.SetIconSize(FVector2D(20.0f, 20.0f));
	FCheckBoxStyle CategoryToggleButton = CategoryToolBarStyle.ToggleButton;
	CategoryToggleButton.SetUncheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::Hover, 4.0f));
	CategoryToggleButton.SetUncheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::DropdownOutline, 4.0f));
	CategoryToggleButton.SetHoveredForegroundColor(FStyleColors::ForegroundHover);
	CategoryToggleButton.SetPressedForegroundColor(FStyleColors::ForegroundHover);
	CategoryToolBarStyle.SetToggleButtonStyle(CategoryToggleButton);
	Style.Set("Rail.CategoryToolBar", CategoryToolBarStyle);

	return StyleRef;
}