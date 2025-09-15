// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreLibrary.h"

EWindowMode::Type UNCoreLibrary::GetWindowModeFromSelection(const FString& Selection)
{
	if (Selection == TEXT("Windowed"))
	{
		return EWindowMode::Type::Windowed;
	}
	else if (Selection == TEXT("Fullscreen Window"))
	{
		return EWindowMode::Type::WindowedFullscreen;
	}
	return EWindowMode::Type::Fullscreen;
}

TArray<FString> UNCoreLibrary::GetWindowModeSelections()
{
	TArray<FString> ReturnValues;
	ReturnValues.Add(TEXT("Fullscreen"));
	ReturnValues.Add(TEXT("Fullscreen Window"));
	ReturnValues.Add(TEXT("Windowed"));
	return ReturnValues;
}
