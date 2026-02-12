// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NGameUserSettingsLibrary.generated.h"

class UNComboBoxString;

/**
 * A collection of functionality used to interact with a games' UGameUserSettings helping build UI components around them.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/">UNGameUserSettingsLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Game User Settings Library")
class UNGameUserSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the associated EWindowMode::Type for the provided FString.
	 * @remark The string should be generated from this library's functionality.
	 * @param Selection The string to find an enumeration for.
	 * @return The corresponding EWindowMode::Type.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode From Selection (String)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-windowmode-from-selection-string"))
	static EWindowMode::Type GetWindowModeFromString(const FString& Selection);

	/**
	 * Get the associated EWindowMode::Type for the provided FText.
	 * @remark The FText should be generated from this library's functionality.
	 * @param Selection The FText to find an enumeration for.
	 * @return The corresponding EWindowMode::Type.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode From Selection (Text)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-windowmode-from-selection-text"))
	static EWindowMode::Type GetWindowModeFromText(const FText& Selection);

	/**
	 * Get the current window modes selection FString.
	 * @return The cached FString representing the current EWindowMode::Type.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Current WindowMode (String)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-current-windowmode-string"))
	static FString& GetSelectionStringFromCurrentWindowMode();

	/**
	 * Get the current window modes selection FText.
	 * @return The cached FText representing the current EWindowMode::Type.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Current WindowMode (Text)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-current-windowmode-text"))
	static FText& GetSelectionTextFromCurrentWindowMode();

	/**
	 * Get the cached selection string from a EWindowMode::Type.
	 * @param Mode the EWindowMode::Type to find the associated FString for.
	 * @return The cached FString representing the target EWindowMode::Type. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From WindowMode (String)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-selection-from-windowmode-string"))
	static FString& GetSelectionStringFromWindowMode(EWindowMode::Type Mode);

	/**
	 * Get the cached selection text from a EWindowMode::Type.
	 * @param Mode the EWindowMode::Type to find the associated FText for.
	 * @return The cached FText representing the target EWindowMode::Type. 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From WindowMode (Text)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-selection-from-windowmode-text"))
	static FText& GetSelectionTextFromWindowMode(EWindowMode::Type Mode);

	/**
	 * Get the cached EWindowMode::Type selection FStrings.
	 * @return The FString array holding the different cached selection strings.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode Selections (String)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-windowmode-selections-string"))
	static TArray<FString>& GetWindowModeStringSelections() { return DisplayModeLabels; };

	/**
	 * Get the cached EWindowMode::Type selection FTexts.
	 * @return The FTexts array holding the different cached selection strings.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get WindowMode Selections (Text)", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-windowmode-selections-text"))
	static TArray<FText>& GetWindowModeTextSelections() { return DisplayModeTexts; };

	/**
	 * Get the selection string from the current display resolution.
	 * @return A FString representing the resolution.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From Current Display Resolution", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-selection-from-current-display-resolution"))
	static FString GetSelectionFromCurrentDisplayResolution();

	/**
	 * Get the selection string from the current display resolution.
	 * @return A FString representing the resolution.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName="Get Selection From Display Resolution", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-selection-from-display-resolution"))
	static FString GetSelectionFromDisplayResolution(FIntPoint Resolution);

	/**
	 * Get the display resolution from a selection string.
	 * @param Selection A FString representing a display resolution (Width x Height)
	 * @return A display resolution as a FIntPoint.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Display Resolution From Selection", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-display-resolution-from-selection"))
	static FIntPoint GetDisplayResolutionFromSelection(const FString& Selection);

	/**
	 * Gets the supported display resolutions for the current platform as strings.
	 * @return An array of FStrings containing all supported resolutions.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Supported Display Resolutions", Category = "NEXUS|User Interface|Game User Settings|Video",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#get-supported-display-resolutions"))
	static TArray<FString> GetSupportedDisplayResolutions();

	/**
	 * Clears and populates a NComboBoxString with the selectable WindowMode types. 
	 * @param ComboBox Target combobox to fill and setup.
	 * @param bSelectCurrent Should the currently used WindowMode be selected?
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Initialize WindowMode Selector (NComboBoxString)", Category = "NEXUS|User Interface|Game User Settings|Initialize",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#initialize-windowmode-selector-ncomboboxstring"))
	static void InitializeWindowModeComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent = true);

	/**
	* Clears and populates a NComboBoxString with the selectable Display Resolutions. 
	* @param ComboBox Target combobox to fill and setup.
	* @param bSelectCurrent Should the current display resolution be selected?
	*/
	UFUNCTION(BlueprintCallable, DisplayName="Initialize Display Resolution Selector (NComboBoxString)", Category = "NEXUS|User Interface|Game User Settings|Initialize",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/#initialize-display-resolution-selector-ncomboboxstring"))
	static void InitializeDisplayResolutionComboBoxString(UNComboBoxString* ComboBox, const bool bSelectCurrent = true);

private:
	
	static TArray<FText> DisplayModeTexts;
	static TArray<FString> DisplayModeLabels;
};