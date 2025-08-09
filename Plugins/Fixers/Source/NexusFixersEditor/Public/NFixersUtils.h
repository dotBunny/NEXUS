// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NFixersEditorSettings.h"
#include "Misc/DataValidation.h"


class FNFixersUtils
{
public:
	static void AddResponse(FDataValidationContext& Context, const ENValidatorStrictness& Level, const FText& Message)
	{
		switch (Level)
		{
		case NVS_Disable:
			break;
		case NVS_Warning:
			Context.AddWarning(Message);
			return;
		case NVS_Error:
			Context.AddError(Message);
			return;
		case NVS_Message:
			Context.AddMessage(EMessageSeverity::Type::Info, Message);
			return;
		}
	};
};
