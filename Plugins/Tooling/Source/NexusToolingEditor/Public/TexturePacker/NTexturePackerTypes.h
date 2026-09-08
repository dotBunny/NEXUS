// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Engine/TextureDefines.h"
#include "NTexturePackerTypes.generated.h"

/**
 * Identifies one of the four channels of an RGBA texture, in memory order.
 *
 * The numeric values are deliberately 0-3 and are used directly as an index into FLinearColor::Component,
 * so nothing here may be reordered.
 */
UENUM()
enum class ENTextureChannel : uint8
{
	Red = 0,
	Green = 1,
	Blue = 2,
	Alpha = 3
};

/**
 * Which part of an assigned source texture feeds a single output channel.
 *
 * Auto is the value every slot starts on: it reads the red channel of a single-channel source (grayscale
 * masks, which is what most inputs to a pack are) and the matching channel of anything wider, so assigning
 * one RGB texture across R/G/B copies it straight through without three manual choices.
 */
UENUM()
enum class ENTextureChannelSource : uint8
{
	/** Red for single-channel sources, the matching channel for anything wider. */
	Auto = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Alpha = 4,
	/** Perceptual luminance of the source's RGB, for folding a color map down to one channel. */
	Luminance = 5,
	/** No texture is read; the slot's constant value fills the channel. */
	Constant = 6
};

/**
 * One semantic map type - roughness, ambient occlusion, base color - and everything the packer needs to
 * recognize it in an asset name and to configure a texture that holds it.
 *
 * Roles are the vocabulary both directions of the tool are written against: packing infers a role per input
 * from its filename suffix and looks the resulting signature up in the layout table; unpacking reads a
 * layout and writes one asset per role. The whole table is project-editable on UNTexturePackerSettings, so a
 * project with its own naming can retune it without a code change.
 */
USTRUCT()
struct NEXUSTOOLINGEDITOR_API FNTextureChannelRole
{
	GENERATED_BODY()

	/** Stable identifier referenced by FNTexturePackedLayout; never shown to the user. */
	UPROPERTY(EditAnywhere, config, Category = "Role")
	FName Id;

	/** Human-readable name shown in the tool's role pickers. */
	UPROPERTY(EditAnywhere, config, Category = "Role")
	FString DisplayName;

	/**
	 * Filename suffixes that identify this role, without the leading underscore and matched
	 * case-insensitively. The first entry is canonical - it is the one written onto generated assets, and
	 * the one used to compose a suffix for a channel signature no layout covers.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Role")
	TArray<FString> Suffixes;

	/** Marks the role as widely-used rather than project-specific; the tool's pickers label these "standard". */
	UPROPERTY(EditAnywhere, config, Category = "Role")
	bool bIndustryStandard = false;

	/** Whether a texture holding only this role is gamma-encoded. True for color data, false for everything measured. */
	UPROPERTY(EditAnywhere, config, Category = "Output")
	bool bOutputSRGB = false;

	/** Compression applied when this role is unpacked to a texture of its own, one channel wide. */
	UPROPERTY(EditAnywhere, config, Category = "Output")
	TEnumAsByte<TextureCompressionSettings> SingleChannelCompression = TC_Alpha;

	/** Compression applied when this role occupies several adjacent channels, as color and normal roles do. */
	UPROPERTY(EditAnywhere, config, Category = "Output")
	TEnumAsByte<TextureCompressionSettings> MultiChannelCompression = TC_Default;

	/** @return The suffix written onto generated assets for this role, without the leading underscore. */
	FString GetCanonicalSuffix() const
	{
		return Suffixes.Num() > 0 ? Suffixes[0] : Id.ToString();
	}

	/** @return DisplayName where one is set, falling back to the identifier so a half-filled row stays readable. */
	FText GetDisplayText() const
	{
		return FText::FromString(DisplayName.IsEmpty() ? Id.ToString() : DisplayName);
	}
};

/**
 * A known arrangement of four roles across an RGBA texture, and the suffix that names it.
 *
 * A role repeated across adjacent channels spans them: the CA layout is BaseColor/BaseColor/BaseColor/Opacity,
 * which unpacks to one RGB color map and one grayscale opacity map rather than four grayscales. Runs are
 * required to be adjacent, which every convention in practice satisfies.
 *
 * NAME_None means the channel carries nothing - packing leaves it at its constant, unpacking skips it.
 */
USTRUCT()
struct NEXUSTOOLINGEDITOR_API FNTexturePackedLayout
{
	GENERATED_BODY()

	/** Filename suffix identifying this layout, without the leading underscore ("ORM"). Matched case-insensitively. */
	UPROPERTY(EditAnywhere, config, Category = "Layout")
	FString Suffix;

	/** Human-readable description shown beside the suffix in the tool ("Occlusion / Roughness / Metallic"). */
	UPROPERTY(EditAnywhere, config, Category = "Layout")
	FString DisplayName;

	/** Role carried by the red channel, or None. */
	UPROPERTY(EditAnywhere, config, Category = "Channels")
	FName RedRole;

	/** Role carried by the green channel, or None. */
	UPROPERTY(EditAnywhere, config, Category = "Channels")
	FName GreenRole;

	/** Role carried by the blue channel, or None. */
	UPROPERTY(EditAnywhere, config, Category = "Channels")
	FName BlueRole;

	/** Role carried by the alpha channel, or None. */
	UPROPERTY(EditAnywhere, config, Category = "Channels")
	FName AlphaRole;

	/** Marks the layout as widely-used rather than project-specific; the tool's pickers label these "standard". */
	UPROPERTY(EditAnywhere, config, Category = "Layout")
	bool bIndustryStandard = false;

	/** Compression applied to a texture packed into this layout. */
	UPROPERTY(EditAnywhere, config, Category = "Output")
	TEnumAsByte<TextureCompressionSettings> Compression = TC_Masks;

	/** @return The role carried by Channel, or NAME_None where the channel is unused. */
	FName GetRole(const ENTextureChannel Channel) const
	{
		switch (Channel)
		{
		case ENTextureChannel::Red: return RedRole;
		case ENTextureChannel::Green: return GreenRole;
		case ENTextureChannel::Blue: return BlueRole;
		case ENTextureChannel::Alpha: return AlphaRole;
		default: return NAME_None;
		}
	}

	/** @return The four channel roles in RGBA order, unused channels included as NAME_None. */
	TArray<FName> GetRoles() const
	{
		return { RedRole, GreenRole, BlueRole, AlphaRole };
	}

	/** @return true where InRoles is exactly this layout's four roles, in order. */
	bool MatchesRoles(const TArray<FName>& InRoles) const
	{
		return InRoles.Num() == 4 && InRoles[0] == RedRole && InRoles[1] == GreenRole &&
			InRoles[2] == BlueRole && InRoles[3] == AlphaRole;
	}

	/** @return DisplayName where one is set, falling back to the suffix. */
	FText GetDisplayText() const
	{
		return FText::FromString(DisplayName.IsEmpty() ? Suffix : DisplayName);
	}
};

/**
 * A contiguous group of channels in a packed texture that carry the same role, produced when a layout is read
 * for unpacking.
 *
 * A run of one becomes a grayscale asset; a run of three becomes an RGB one. This is what stops a color map
 * packed across RGB from being unpacked into three meaningless grayscales.
 */
struct NEXUSTOOLINGEDITOR_API FNTextureChannelRun
{
	/** The role every channel in the run carries. */
	FName RoleId;

	/** First channel of the run. */
	ENTextureChannel FirstChannel = ENTextureChannel::Red;

	/** How many adjacent channels the run covers, 1-4. */
	int32 Length = 1;
};
