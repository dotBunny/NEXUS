// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

UENUM(BlueprintType)
enum ENColor : uint8
{
	NC_Black				UMETA(DisplayName = "Black"),
	NC_White				UMETA(DisplayName = "White"),

	NC_BlueLight			UMETA(DisplayName = "Light Blue"),
	NC_BlueMid				UMETA(DisplayName = "Mid Blue"),
	NC_BlueDark				UMETA(DisplayName = "Dark Blue"),

	NC_GreenLight			UMETA(DisplayName = "Light Green"),
	NC_GreenMid				UMETA(DisplayName = "Mid Green"),
	NC_GreenDark			UMETA(DisplayName = "Dark Green"),

	NC_GreyLight			UMETA(DisplayName = "Light Grey"),
	NC_GreyDark				UMETA(DisplayName = "Dark Grey"),
	
	NC_Red					UMETA(DisplayName = "Red"),
	NC_Orange				UMETA(DisplayName = "Orange"),
	NC_Yellow				UMETA(DisplayName = "Yellow"),
	NC_Green				UMETA(DisplayName = "Green"),
	NC_Pink					UMETA(DisplayName = "Pink"),


	NC_NexusDarkBlue		UMETA(DisplayName = "NEXUS: Dark Blue"),
	NC_NexusLightBlue		UMETA(DisplayName = "NEXUS: Light Blue"),
	NC_NexusBlack			UMETA(DisplayName = "NEXUS: Black"),
	NC_NexusPink			UMETA(DisplayName = "NEXUS: Pink"),

	NC_HalfBlack			UMETA(DisplayName = "Half Black")
};

/**
 * A collection of color values used throughout the Nexus modules.
 */
class NEXUSCORE_API FNColor
{
public:
	static FLinearColor GetLinearColor(const ENColor& Color);
	static FColor GetColor(const ENColor& Color);

	
	static constexpr FLinearColor BlueDark = FLinearColor(0.0352941176470588f, 0.0509803921568627f, 0.1450980392156863f, 1.0f);
	static constexpr FLinearColor BlueMid = FLinearColor(0.1058823529411765f, 0.2745098039215686f, 0.6156862745098039f, 1.0f);
	static constexpr FLinearColor BlueLight = FLinearColor(0.1019607843137255f, 0.8f, 0.9333333333333333f, 1.0f);


	static constexpr FLinearColor GreenDark = FLinearColor(0.0352941176470588f, 0.2009803921568627f, 0.0450980392156863f, 1.0f);
	static constexpr FLinearColor GreenMid = FLinearColor(0.1607843137254902f, 0.5254901960784314f, 0.1254901960784314, 1.0f);
	static constexpr FLinearColor GreenLight = FLinearColor(0.1019607843137255f, 0.9f, 0.1019607843137255f, 1.0f);

	static constexpr FLinearColor Pink = FLinearColor(0.8196078431372549f, 0.3529411764705882f, 0.8941176470588235f, 1.0f);
	static constexpr FLinearColor GreyLight = FLinearColor(0.123264f, 0.123264f, 0.123264f, 1.0f);
	static constexpr FLinearColor GreyDark = FLinearColor(0.029514f, 0.029514f, 0.029514f, 1.0f);
	static constexpr FLinearColor Yellow = FLinearColor(1.f, 1.f, 0.0f, 1.0f);
	static constexpr FLinearColor Orange = FLinearColor(1.f, 0.5f, 0.f, 1.0f);

	static constexpr FLinearColor NexusDarkBlue = FLinearColor(0.09765625f, 0.16015625f, 0.31640625f, 1.f);
	static constexpr FLinearColor NexusLightBlue =FLinearColor(0.29296875f, 0.4375f, 0.7578125f, 1.f);
	static constexpr FLinearColor NexusBlack = FLinearColor(0.f,0.f,0.00390625f, 1.f);
	static constexpr FLinearColor NexusPink = FLinearColor(0.6328125f,0.4609375f,0.8671875f, 1.f);

	static constexpr FLinearColor HalfBlack = FLinearColor(0.f,0.f,0.f, 0.5f);
};