// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGameUserSettingsLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FText> UNGameUserSettingsLibrary::DisplayModeTexts = {
	NSLOCTEXT("NexusUI", "FullscreenDisplayModeLabel", "Fullscreen"),
	NSLOCTEXT("NexusUI", "FullscreenWindowDisplayModeLabel", "Fullscreen Window"),
	NSLOCTEXT("NexusUI", "WindowedDisplayModeLabel", "Windowed")
};

TArray<FString> UNGameUserSettingsLibrary::DisplayModeLabels = {
	DisplayModeTexts[0].ToString(),
	DisplayModeTexts[1].ToString(),
	DisplayModeTexts[2].ToString()
};

EWindowMode::Type UNGameUserSettingsLibrary::GetWindowModeFromString(const FString& Selection)
{
	if (DisplayModeLabels[2].Compare(Selection) == 0)
	{
		return EWindowMode::Type::Windowed;
	}
	if (DisplayModeLabels[1].Compare(Selection) == 0)
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
}

EWindowMode::Type UNGameUserSettingsLibrary::GetWindowModeFromText(const FText& Selection)
{
	if (Selection.EqualTo(DisplayModeTexts[2]))
	{
		return EWindowMode::Type::Windowed;
	}
	if (Selection.EqualTo(DisplayModeTexts[1]))
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
}

FString& UNGameUserSettingsLibrary::GetSelectionStringFromWindowMode(const EWindowMode::Type Mode)
{
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return DisplayModeLabels[1];
	case EWindowMode::Windowed:
		return DisplayModeLabels[2];
	default:
		return DisplayModeLabels[0];
	}
}

FText& UNGameUserSettingsLibrary::GetSelectionTextFromWindowMode(EWindowMode::Type Mode)
{
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return DisplayModeTexts[1];
	case EWindowMode::Windowed:
		return DisplayModeTexts[2];
	default:
		return DisplayModeTexts[0];
	}
}

FString UNGameUserSettingsLibrary::GetSelectionFromDisplayResolution(const FIntPoint Resolution)
{
	return FString::Printf(TEXT("%ix%i"), Resolution.X, Resolution.Y);
}

FIntPoint UNGameUserSettingsLibrary::GetDisplayResolutionFromSelection(const FString& Selection)
{
	TArray<FString> Resolution;
	Selection.ParseIntoArray(Resolution, TEXT("x"), true);
	return FIntPoint(
		FCString::Atoi(*Resolution[0].TrimStartAndEnd()),
		FCString::Atoi(*Resolution[1].TrimStartAndEnd())
		);
}

TArray<FString> UNGameUserSettingsLibrary::GetSupportedDisplayResolutions()
{
	TArray<FIntPoint> AllResolutions;
	TArray<FString> SupportedResolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(AllResolutions);
	for (const FIntPoint& Resolution : AllResolutions)
	{
		SupportedResolutions.Add(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
	}
	return MoveTemp(SupportedResolutions);
}

