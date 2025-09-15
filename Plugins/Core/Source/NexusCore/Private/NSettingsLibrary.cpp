// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSettingsLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

EWindowMode::Type UNSettingsLibrary::GetWindowModeFromSelection(const FString& Selection)
{
	if (Selection == TEXT("Windowed"))
	{
		return EWindowMode::Type::Windowed;
	}
	if (Selection == TEXT("Fullscreen Window"))
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
}

FString UNSettingsLibrary::GetSelectionFromWindowMode(EWindowMode::Type Mode)
{
	switch(Mode)
	{
	case EWindowMode::WindowedFullscreen:
		return TEXT("Fullscreen Window");
	case EWindowMode::Windowed:
		return TEXT("Windowed");
	default:
		return TEXT("Fullscreen");
	}
}

TArray<FString> UNSettingsLibrary::GetWindowModeSelections()
{
	TArray<FString> ReturnValues;
	ReturnValues.Add(TEXT("Fullscreen"));
	ReturnValues.Add(TEXT("Fullscreen Window"));
	ReturnValues.Add(TEXT("Windowed"));
	return ReturnValues;
}

FString UNSettingsLibrary::GetSelectionFromDisplayResolution(const FIntPoint Resolution)
{
	return FString::Printf(TEXT("%ix%i"), Resolution.X, Resolution.Y);
}

FIntPoint UNSettingsLibrary::GetDisplayResolutionFromSelection(const FString& Selection)
{
	TArray<FString> Resolution;
	Selection.ParseIntoArray(Resolution, TEXT("x"));
	return FIntPoint(
		FCString::Atoi(*Resolution[0]),
		FCString::Atoi(*Resolution[1])
		);
}

TArray<FString> UNSettingsLibrary::GetSupportedDisplayResolutions()
{
	TArray<FIntPoint> AllResolutions;
	TArray<FString> SupportedResolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(AllResolutions);
	for (const FIntPoint& Resolution : AllResolutions)
	{
		SupportedResolutions.Add(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
	}
	return SupportedResolutions;
}

