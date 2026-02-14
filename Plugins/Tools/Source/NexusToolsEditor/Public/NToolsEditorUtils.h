// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NToolsEditorSettings.h"
#include "Misc/DataValidation.h"

class FNToolsEditorUtils
{
public:
	static void AddResponse(FDataValidationContext& Context, const ENValidatorSeverity& Level, const FText& NewMessage)
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

	static EDataValidationResult GetResult(const ENValidatorSeverity& Level)
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
};
