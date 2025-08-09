// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NFixersEditorSettings.h"
#include "Misc/DataValidation.h"


class FNFixersUtils
{
public:
	static void AddResponse(FDataValidationContext& Context, const ENValidatorSeverity& Level, const FText& Message)
	{
		switch (Level)
		{
		case NVS_Disable:
			break;
		case NVS_Warning:
		case NVS_WarningButValid:
			Context.AddWarning(Message);
			return;
		case NVS_Error:
			Context.AddError(Message);
			return;
		case NVS_Message:
			Context.AddMessage(EMessageSeverity::Type::Info, Message);
			break;
		}
	};

	static EDataValidationResult GetResult(const ENValidatorSeverity& Level)
	{
		switch (Level)
		{
		case NVS_Disable:
			return EDataValidationResult::NotValidated;
		case NVS_Warning:
			return EDataValidationResult::Invalid;
		case NVS_WarningButValid:
			return EDataValidationResult::Valid;
		case NVS_Error:
			return EDataValidationResult::Invalid;
		case NVS_Message:
			return EDataValidationResult::Valid;
		}
		return EDataValidationResult::NotValidated;
	}
};
