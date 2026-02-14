// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NToolingEditorSettings.h"
#include "Misc/DataValidation.h"

class FNToolingEditorUtils
{
public:

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
	
	static void ReplaceAppIcon(FSlateImageBrush* Icon);
	static void ReplaceAppIconSVG(FSlateVectorImageBrush* Icon);
	static bool ReplaceWindowIcon(const FString& IconPath);
};
