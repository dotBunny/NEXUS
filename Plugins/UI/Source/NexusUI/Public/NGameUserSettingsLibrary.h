// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NGameUserSettingsLibrary.generated.h"

class UNComboBoxString;

UCLASS()
class UNGameUserSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode From Selection (String)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static EWindowMode::Type GetWindowModeFromString(const FString& Selection);

	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode From Selection (Text)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static EWindowMode::Type GetWindowModeFromText(const FText& Selection);

	UFUNCTION(BlueprintCallable, DisplayName="Get Current WindowMode (String)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FString& GetSelectionStringFromCurrentWindowMode();

	UFUNCTION(BlueprintCallable, DisplayName="Get Current WindowMode (Text)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FText& GetSelectionTextFromCurrentWindowMode();
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From WindowMode (String)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FString& GetSelectionStringFromWindowMode(EWindowMode::Type Mode);

	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From WindowMode (Text)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FText& GetSelectionTextFromWindowMode(EWindowMode::Type Mode);

	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode Selections (String)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static TArray<FString>& GetWindowModeStringSelections() { return DisplayModeLabels; };

	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode Selections (Text)", Category = "NEXUS|User Interface|Game User Settings|Video")
	static TArray<FText>& GetWindowModeTextSelections() { return DisplayModeTexts; };

	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From Current Display Resolution", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FString GetSelectionFromCurrentDisplayResolution();
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From Display Resolution", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FString GetSelectionFromDisplayResolution(FIntPoint Resolution);

	UFUNCTION(BlueprintCallable, DisplayName="Get Display Resolution From Selection", Category = "NEXUS|User Interface|Game User Settings|Video")
	static FIntPoint GetDisplayResolutionFromSelection(const FString& Selection);
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Supported Display Resolutions", Category = "NEXUS|User Interface|Game User Settings|Video")
	static TArray<FString> GetSupportedDisplayResolutions();
	
	UFUNCTION(BlueprintCallable, DisplayName="Initialize DisplayMode Selector", Category = "NEXUS|User Interface|Game User Settings|Video")
	static void InitializeDisplayModeComboBox(UNComboBoxString* ComboBox);

	UFUNCTION(BlueprintCallable, DisplayName="Initialize Display Resolution Selector", Category = "NEXUS|User Interface|Game User Settings|Video")
	static void InitializeDisplayResolutionComboBox(UNComboBoxString* ComboBox);

private:
	
	static TArray<FText> DisplayModeTexts;
	static TArray<FString> DisplayModeLabels;
};