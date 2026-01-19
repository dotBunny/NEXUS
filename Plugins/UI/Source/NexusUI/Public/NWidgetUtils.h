// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Components/SlateWrapperTypes.h"

class FNWidgetUtils
{
public:
	static EVisibility ToEVisibility(const ESlateVisibility Visibility)
	{
		switch ( Visibility )
		{
		case ESlateVisibility::Visible:
			return EVisibility::Visible;
		case ESlateVisibility::Collapsed:
			return EVisibility::Collapsed;
		case ESlateVisibility::Hidden:
			return EVisibility::Hidden;
		case ESlateVisibility::HitTestInvisible:
			return EVisibility::HitTestInvisible;
		case ESlateVisibility::SelfHitTestInvisible:
			return EVisibility::SelfHitTestInvisible;
		default:
			return EVisibility::Visible;
		}
	}
};
