// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NToolingEditorSettings.h"
#include "Misc/DataValidation.h"

/**
 * Static helpers shared by the NexusTooling editor module and its validators.
 *
 * Bundles severity routing for FDataValidationContext, Starship icon replacement primitives, and
 * blueprint-editor selection utilities.
 */
class FNToolingEditorUtils
{
	friend class FNToolingEditorModule;
public:

	/**
	 * Forward NewMessage to Context using the bus that matches Level.
	 * @param Context The active data-validation context to add the message to.
	 * @param Level Severity that drives which message bus (warning/error/info) receives NewMessage.
	 * @param NewMessage Human-readable text surfaced in the data-validation panel.
	 */
	FORCEINLINE static void AddDataValidationResponse(FDataValidationContext& Context, const ENValidatorSeverity& Level, const FText& NewMessage)
	{
		switch (Level)
		{
			using enum ENValidatorSeverity;
		case Disable:
			break;
		case Warning:
		case WarningButValid:
			Context.AddWarning(NewMessage);
			return;
		case Error:
			Context.AddError(NewMessage);
			return;
		case Message:
			Context.AddMessage(EMessageSeverity::Type::Info, NewMessage);
			break;
		}
	};

	/**
	 * Map a NexusTooling severity onto the engine's EDataValidationResult verdict.
	 * @return Invalid for Warning/Error, Valid for WarningButValid/Message, NotValidated for Disable.
	 */
	FORCEINLINE static EDataValidationResult GetDataValidationResult(const ENValidatorSeverity& Level)
	{
		switch (Level)
		{
			using enum ENValidatorSeverity;
		case Disable:
			return EDataValidationResult::NotValidated;
		case Warning:
			return EDataValidationResult::Invalid;
		case WarningButValid:
			return EDataValidationResult::Valid;
		case Error:
			return EDataValidationResult::Invalid;
		case Message:
			return EDataValidationResult::Valid;
		}
		return EDataValidationResult::NotValidated;
	}

	/**
	 * Replace the Starship app icon with the supplied raster brush.
	 *
	 * Ownership transfers to the engine's shared app style set (FAppStyle), which deletes the brush when it is
	 * torn down. Pass a heap-allocated brush (e.g. new FSlateImageBrush(...)) and do not delete it afterwards.
	 * @note Set() overwrites the existing "AppIcon" entry without deleting the brush previously registered under
	 *       that key, so each swap orphans a single brush. Intended to be applied once at startup, not repeatedly.
	 */
	static void ReplaceAppIcon(FSlateImageBrush* Icon);

	/**
	 * Replace the Starship app icon with the supplied vector brush.
	 *
	 * Ownership transfers to the engine's shared app style set (FAppStyle), which deletes the brush when it is
	 * torn down. Pass a heap-allocated brush (e.g. new FSlateVectorImageBrush(...)) and do not delete it afterwards.
	 * @note Set() overwrites the existing "AppIcon" entry without deleting the brush previously registered under
	 *       that key, so each swap orphans a single brush. Intended to be applied once at startup, not repeatedly.
	 */
	static void ReplaceAppIconSVG(FSlateVectorImageBrush* Icon);

	/** Replace the platform-level editor window icon with the resource at IconPath (extensionless). */
	static bool ReplaceWindowIcon(const FString& IconPath);

	/** @return true if AssetType is one of the asset types that opens in a blueprint editor tab. */
	static bool IsBlueprintEditorAssetType(const FName AssetType) { return KnownBlueprintEditorAssetTypes.Contains(AssetType);}

	/**
	 * Try to get the currently selected nodes in the foreground Blueprint editor.
	 * @param OutSelection Populated with the selected graph panel nodes on success.
	 * @return true if a blueprint editor is in front and yielded a selection.
	 */
	static bool TryGetForegroundBlueprintEditorSelectedNodes(FGraphPanelSelectionSet& OutSelection);

private:
	/** Lazily populate KnownBlueprintEditorAssetTypes from UE's asset type registrations. */
	static void SetBlueprintEditorAssetTypes();

	/** Cached set of asset type FNames that open in blueprint editors. */
	static TArray<FName> KnownBlueprintEditorAssetTypes;
};
