// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NColor.h"

FLinearColor FNColor::GetLinearColor(const ENColor& Color)
{
	switch (Color)
	{
	case NC_Black:
		return FLinearColor::Black;
	case NC_White:
		return FLinearColor::White;
	case NC_BlueLight:
		return BlueLight;
	case NC_BlueMid:
		return BlueMid;
	case NC_BlueDark:
		return BlueDark;
	case NC_GreenLight:
		return GreenLight;
	case NC_GreenMid:
		return GreenMid;
	case NC_GreenDark:
		return GreenDark;		
	case NC_Red:
		return FLinearColor::Red;
	case NC_Green:
		return FLinearColor::Green;
	case NC_GreyLight:
		return GreyLight;
	case NC_GreyDark:
		return GreyDark;
	case NC_Yellow:
		return Yellow;
	case NC_Orange:
		return Orange;
	case NC_NexusDarkBlue:
		return NexusDarkBlue;
	case NC_NexusLightBlue:
		return NexusLightBlue;
	case NC_NexusBlack:
		return NexusBlack;
	case NC_NexusPink:
		return NexusPink;
	case NC_HalfBlack:
		return HalfBlack;
	case NC_Pink:
		break;
	}
	return Pink;
}

FColor FNColor::GetColor(const ENColor& Color)
{
	return GetLinearColor(Color).ToFColor(true);
}
