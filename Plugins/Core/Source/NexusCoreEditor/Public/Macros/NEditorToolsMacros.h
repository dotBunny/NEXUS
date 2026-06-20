// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// A separator entry whose visibility can be driven dynamically. We use a widget block (not InitSeparator)
// because UToolMenus drops the visibility attribute for native separator entries in a toolbar.
// To make it indistinguishable from the automatic section dividers we mirror SlimToolBar's native
// separator metrics: the -5 vertical padding (SeparatorPadding = FMargin(4, -5)) lets the line overflow
// into the toolbar's background padding so it spans the full toolbar height, the 2.0 thickness matches
// FToolBarStyle's default SeparatorThickness, and we reuse the style's SeparatorBrush (recessed colour)
// rather than SSeparator's lighter default "Separator" brush.
#define N_DYNAMIC_SEPARATOR(Name, VisibilityLogic, Label) \
	FToolMenuEntry::InitWidget(Name, \
		SNew(SBox).Padding(FMargin(4.0f, -5.0f)).VAlign(VAlign_Fill).Visibility_Lambda([]() { return VisibilityLogic; }) \
		[ SNew(SSeparator).Orientation(Orient_Vertical).Thickness(2.0f) \
			.SeparatorImage(&FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("SlimToolBar").SeparatorBrush)], Label, true)