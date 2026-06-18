// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGameUserSettingsLibrary.h"

#include "NUIMinimal.h"
#include "Components/NComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Macros/NValidationMacros.h"

TArray<FText> UNGameUserSettingsLibrary::DisplayModeTexts = {
	NSLOCTEXT("NexusUI", "FullscreenDisplayModeLabel", "Fullscreen"),
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	NSLOCTEXT("NexusUI", "FullscreenWindowDisplayModeLabel", "Fullscreen Window"),
	NSLOCTEXT("NexusUI", "WindowedDisplayModeLabel", "Windowed")
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
};

EWindowMode::Type UNGameUserSettingsLibrary::GetWindowModeFromString(const FString& Selection)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (DisplayModeTexts[2].ToString().Compare(Selection) == 0)
	{
		return EWindowMode::Type::Windowed;
	}
	if (DisplayModeTexts[1].ToString().Compare(Selection) == 0)
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
#else // !(PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
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
#else // !(PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
	return EWindowMode::Type::Fullscreen;
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

TArray<FString> UNGameUserSettingsLibrary::GetWindowModeStringSelections()
{
	TArray<FString> Selections;
	Selections.Reserve(DisplayModeTexts.Num());
	for (const FText& Text : DisplayModeTexts)
	{
		Selections.Add(Text.ToString());
	}
	return Selections;
}

FString UNGameUserSettingsLibrary::GetSelectionStringFromCurrentWindowMode()
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (Settings == nullptr)
	{
		return DisplayModeTexts[0].ToString();
	}
	return GetSelectionStringFromWindowMode(Settings->GetFullscreenMode());
}

FText UNGameUserSettingsLibrary::GetSelectionTextFromCurrentWindowMode()
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (Settings == nullptr)
	{
		return DisplayModeTexts[0];
	}
	return GetSelectionTextFromWindowMode(Settings->GetFullscreenMode());
}

FString UNGameUserSettingsLibrary::GetSelectionStringFromWindowMode(const EWindowMode::Type Mode)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return DisplayModeTexts[1].ToString();
	case EWindowMode::Windowed:
		return DisplayModeTexts[2].ToString();
	default:
		return DisplayModeTexts[0].ToString();
	}
#else // !(PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
	return DisplayModeTexts[0].ToString();
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

FText UNGameUserSettingsLibrary::GetSelectionTextFromWindowMode(EWindowMode::Type Mode)
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
#else // !(PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
	return DisplayModeTexts[0];
#endif // PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
}

FString UNGameUserSettingsLibrary::GetSelectionFromCurrentDisplayResolution()
{
	const UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (Settings == nullptr)
	{
		return FString();
	}
	return GetSelectionFromDisplayResolution(Settings->GetScreenResolution());
}

FString UNGameUserSettingsLibrary::GetSelectionFromDisplayResolution(const FIntPoint Resolution)
{
	return FString::Printf(TEXT("%ix%i"), Resolution.X, Resolution.Y);
}

FIntPoint UNGameUserSettingsLibrary::GetDisplayResolutionFromSelection(const FString& Selection)
{
	TArray<FString> Resolution;
	
	// Parse
	const int32 ElementCount = Selection.ParseIntoArray(Resolution, TEXT("x"), true);
	
	// Ensure we can convert to a FIntPoint (in-case the provided string isn't one we made)
	if (ElementCount < 2)
	{
		return FIntPoint(-1,-1);
	}
	
	// Return our value
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
		SupportedResolutions.Add(FString::Printf(TEXT("%ix%i"), Resolution.X, Resolution.Y));
	}
	return SupportedResolutions;
}

void UNGameUserSettingsLibrary::InitializeWindowModeComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent)
{
	N_VALIDATE_RETURN_VOID(LogNexusUI, ComboBox);
	ComboBox->ClearOptions();
	for (const FText& Text : DisplayModeTexts)
	{
		ComboBox->AddOption(Text.ToString());
	}
	if (bSelectCurrent)
	{
		ComboBox->SetSelectedOption_NoBroadcast(GetSelectionStringFromCurrentWindowMode());
	}
}

void UNGameUserSettingsLibrary::InitializeDisplayResolutionComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent)
{
	N_VALIDATE_RETURN_VOID(LogNexusUI, ComboBox);
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

