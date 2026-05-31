// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_DYNAMIC_SEPARATOR(Name, VisibilityLogic, Label) \
	FToolMenuEntry::InitWidget(Name, \
		SNew(SBox).Padding(FMargin(4.0f, 0.f, 4.0f, 0.f)).VAlign(VAlign_Fill).Visibility_Lambda([]() { return VisibilityLogic; }) \
		[ SNew(SSeparator).Orientation(Orient_Vertical).Thickness(2.0f)], Label, true)