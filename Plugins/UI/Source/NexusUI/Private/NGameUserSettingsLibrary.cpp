// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGameUserSettingsLibrary.h"

#include "Components/NComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FText> UNGameUserSettingsLibrary::DisplayModeTexts = {
	NSLOCTEXT("NexusUI", "FullscreenDisplayModeLabel", "Fullscreen"),
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	NSLOCTEXT("NexusUI", "FullscreenWindowDisplayModeLabel", "Fullscreen Window"),
	NSLOCTEXT("NexusUI", "WindowedDisplayModeLabel", "Windowed")
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
};

TArray<FString> UNGameUserSettingsLibrary::DisplayModeLabels = {
	DisplayModeTexts[0].ToString(),
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX		
	DisplayModeTexts[1].ToString(),
	DisplayModeTexts[2].ToString()
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX	
};

EWindowMode::Type UNGameUserSettingsLibrary::GetWindowModeFromString(const FString& Selection)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX		
	if (DisplayModeLabels[2].Compare(Selection) == 0)
	{
		return EWindowMode::Type::Windowed;
	}
	if (DisplayModeLabels[1].Compare(Selection) == 0)
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
#else // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	return EWindowMode::Type::Fullscreen;
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

EWindowMode::Type UNGameUserSettingsLibrary::GetWindowModeFromText(const FText& Selection)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX	
	if (Selection.EqualTo(DisplayModeTexts[2]))
	{
		return EWindowMode::Type::Windowed;
	}
	if (Selection.EqualTo(DisplayModeTexts[1]))
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
#else // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	return EWindowMode::Type::Fullscreen;
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

FString& UNGameUserSettingsLibrary::GetSelectionStringFromCurrentWindowMode()
{
	return GetSelectionStringFromWindowMode(GEngine->GetGameUserSettings()->GetFullscreenMode());
}

FText& UNGameUserSettingsLibrary::GetSelectionTextFromCurrentWindowMode()
{
	return GetSelectionTextFromWindowMode(GEngine->GetGameUserSettings()->GetFullscreenMode());
}

FString& UNGameUserSettingsLibrary::GetSelectionStringFromWindowMode(const EWindowMode::Type Mode)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return DisplayModeLabels[1];
	case EWindowMode::Windowed:
		return DisplayModeLabels[2];
	default:
		return DisplayModeLabels[0];
	}
#else // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	return DisplayModeLabels[0];
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

FText& UNGameUserSettingsLibrary::GetSelectionTextFromWindowMode(EWindowMode::Type Mode)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX	
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return DisplayModeTexts[1];
	case EWindowMode::Windowed:
		return DisplayModeTexts[2];
	default:
		return DisplayModeTexts[0];
	}
#else // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	return DisplayModeLabels[0];
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

FString UNGameUserSettingsLibrary::GetSelectionFromCurrentDisplayResolution()
{
	return GetSelectionFromDisplayResolution(GEngine->GetGameUserSettings()->GetScreenResolution());
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

void UNGameUserSettingsLibrary::InitializeWindowModeComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent)
{
	ComboBox->ClearOptions();
	for (auto& Label : DisplayModeLabels)
	{
		ComboBox->AddOption(Label);
	}
	if (bSelectCurrent)
	{
		ComboBox->SetSelectedOption_NoBroadcast(GetSelectionStringFromCurrentWindowMode());
	}
}

void UNGameUserSettingsLibrary::InitializeDisplayResolutionComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent)
{
	ComboBox->ClearOptions();
	for (auto& Resolution : GetSupportedDisplayResolutions())
	{
		ComboBox->AddOption(Resolution);
	}
	if (bSelectCurrent)
	{
		ComboBox->SetSelectedOption_NoBroadcast(GetSelectionFromCurrentDisplayResolution());
	}
}

