// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "TexturePacker/NTexturePackerTypes.h"

struct FImage;
class UNTexturePackerSettings;
class UTexture2D;

/** An asset name split into the part that names the thing and the part that names what it holds. */
struct FNTextureNameParts
{
	/** Everything before the recognized suffix, with no trailing underscore ("T_Rock_01"). */
	FString BaseName;

	/** The recognized suffix without its underscore ("ORM"), or empty where the name ended in nothing known. */
	FString Suffix;
};

/**
 * The Texture Packer's naming logic and its pixel plumbing, as free functions.
 *
 * Everything here is deliberately stateless and takes its conventions as a parameter rather than reaching
 * for the settings CDO, which is what makes the naming half testable in isolation - a test builds a
 * UNTexturePackerSettings with NewObject to get the shipped defaults without whatever the project's ini has
 * done to them.
 */
class NEXUSTOOLINGEDITOR_API FNTexturePackerUtils
{
public:
	//~Naming
	/**
	 * Splits an asset name into its base and a trailing suffix the conventions recognize.
	 * @note An unrecognized trailing token is left on the base name rather than guessed at, so "T_Rock_01"
	 *       comes back whole instead of losing its "01".
	 */
	static FNTextureNameParts SplitAssetName(const UNTexturePackerSettings& Settings, const FString& AssetName);

	/**
	 * @return The longest run of leading underscore-separated tokens shared by every name given.
	 * @note Compared token-wise rather than character-wise, so "T_Rock_01" and "T_Rock_02" share "T_Rock"
	 *       rather than the "T_Rock_0" a character prefix would produce.
	 */
	static FString CommonBaseName(const TArray<FString>& BaseNames);

	/** @return BaseName and Suffix joined with an underscore, tolerating either being empty. */
	static FString ComposeAssetName(const FString& BaseName, const FString& Suffix);

	/**
	 * @return A suffix describing InRoles, for a channel signature no layout covers.
	 * @note Adjacent repeats collapse, so a normal map spanning red and green contributes one N.
	 */
	static FString ComposeSuffixForRoles(const UNTexturePackerSettings& Settings, const TArray<FName>& InRoles);

	/**
	 * Reads a suffix no layout covers as a run of role suffixes ("RMAH"), longest match first.
	 * @return true where the whole suffix was consumed into four or fewer roles; OutRoles is always four long.
	 * @remark A last resort behind the layout table - it cannot tell AO from A-then-O, and takes the longer.
	 */
	static bool DecomposeSuffixToRoles(const UNTexturePackerSettings& Settings, const FString& Suffix,
		TArray<FName>& OutRoles);

	/** @return The role a single texture holds, inferred from its name, or NAME_None. */
	static FName InferRole(const UNTexturePackerSettings& Settings, const FString& AssetName);

	/**
	 * Works out what a packed texture holds from its name.
	 * @param OutLayout Receives the matched layout, or one synthesized from a decomposed suffix.
	 * @param OutBaseName Receives the name with the suffix removed.
	 * @return true where the suffix was recognized; false leaves OutLayout empty and the caller to ask.
	 */
	static bool DetectLayout(const UNTexturePackerSettings& Settings, const FString& AssetName,
		FNTexturePackedLayout& OutLayout, FString& OutBaseName);

	/**
	 * @return InRoles collapsed into adjacent same-role groups, skipping unused channels.
	 * @note This is what keeps a color map packed across RGB from unpacking into three grayscales.
	 */
	static TArray<FNTextureChannelRun> BuildChannelRuns(const TArray<FName>& InRoles);
	//End Naming

	//~Pixels
	/**
	 * Reads a texture's authoring data as linear 32-bit float RGBA, the space every channel operation works in.
	 *
	 * @param bTreatAsLinear Ignores the texture's SRGB flag and takes its stored values at face value. Off by
	 *        default, so a gamma-encoded source is decoded exactly as a material sampler would decode it.
	 * @return false with OutError set where the texture has no source data, or has more than one UDIM block.
	 */
	static bool ReadTextureLinear(UTexture2D* Texture, FImage& OutImage, bool bTreatAsLinear,
		FText& OutError);

	/** @return true where every pixel of Channel holds the same value, within a tolerance of one 8-bit step. */
	static bool IsChannelUniform(const FImage& LinearImage, ENTextureChannel Channel);

	/**
	 * @return The value Source selects from Pixel.
	 * @param TargetChannel Used only by Auto, which reads the matching channel of a multi-channel source.
	 * @param bSourceIsSingleChannel Steers Auto onto red, which is where a grayscale source keeps its data.
	 */
	static float SampleChannel(const FLinearColor& Pixel, ENTextureChannelSource Source,
		ENTextureChannel TargetChannel, bool bSourceIsSingleChannel);

	/** @return true where Format carries one meaningful channel, and Auto should therefore read red. */
	static bool IsSingleChannelSource(const UTexture2D* Texture);
	//End Pixels

	//~Assets
	/**
	 * Writes Image into the texture at PackagePath/AssetName, creating it where it does not exist.
	 *
	 * An existing asset is checked out (or added) first where bCheckOut is set, because a texture already
	 * under revision control is read-only on disk and Source.Init would otherwise fail late. The package is
	 * left dirty rather than saved - what the tool produced is reviewable before it is committed to.
	 *
	 * @param SettingsTemplate Optional texture whose mip and address settings the result inherits.
	 * @return The texture written, or nullptr with OutError set.
	 */
	static UTexture2D* CreateOrUpdateTexture(const FString& PackagePath, const FString& AssetName,
		const FImage& Image, bool bSRGB, TextureCompressionSettings Compression,
		const UTexture2D* SettingsTemplate, bool bCheckOut, FText& OutError);

	/** @return The long package path (folder) holding Asset, or empty where it has no package. */
	static FString GetPackagePath(const UObject* Asset);

	/** @return true where an asset already exists at PackagePath/AssetName. */
	static bool DoesAssetExist(const FString& PackagePath, const FString& AssetName);

	/** Shows Assets in the Content Browser, replacing its current selection. */
	static void SelectAssetsInContentBrowser(const TArray<UObject*>& Assets);

	/** @return The folder currently selected in the Content Browser, or empty where nothing is. */
	static FString GetContentBrowserPath();
	//End Assets
};
