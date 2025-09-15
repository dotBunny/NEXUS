// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NSettingsLibrary.generated.h"

UCLASS()
class UNSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode From Selection", Category = "NEXUS|Core|Settings")
	static EWindowMode::Type GetWindowModeFromSelection(const FString& Selection);

	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From WindowMode", Category = "NEXUS|Core|Settings")
	static FString GetSelectionFromWindowMode(EWindowMode::Type Mode);

	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode Selections", Category = "NEXUS|Core|Settings")
	static TArray<FString> GetWindowModeSelections();

	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From Display Resolution", Category = "NEXUS|Core|Settings")
	static FString GetSelectionFromDisplayResolution(FIntPoint Resolution);

	UFUNCTION(BlueprintCallable, DisplayName="Get Supported Display Resolutions", Category = "NEXUS|Core|Settings")
	static TArray<FString>GetSupportedDisplayResolutions();
};