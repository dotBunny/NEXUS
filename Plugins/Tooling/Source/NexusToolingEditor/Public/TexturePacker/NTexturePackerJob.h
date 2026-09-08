// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "TexturePacker/NTexturePackerTypes.h"
#include "NTexturePackerJob.generated.h"

class UNTexturePackerSettings;
class UTexture2D;

/** One output channel's supply: a texture and which of its channels to read, or a flat value. */
USTRUCT(BlueprintType)
struct NEXUSTOOLINGEDITOR_API FNTexturePackSlot
{
	GENERATED_BODY()

	/** Texture read for this channel. Null makes the slot constant regardless of Source. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TObjectPtr<UTexture2D> Texture = nullptr;

	/** Which part of Texture feeds the channel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	ENTextureChannelSource Source = ENTextureChannelSource::Auto;

	/** Writes one minus the sampled value, for the roughness-against-gloss case. Ignored on a constant slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bInvert = false;

	/** Value written when the slot has no texture, in linear space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConstantValue = 0.0f;

	/** Takes the texture's stored values at face value, ignoring its sRGB flag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bTreatSourceAsLinear = false;

	/** @return true where this slot reads a texture rather than writing a flat value. */
	bool HasTexture() const
	{
		return Texture != nullptr && Source != ENTextureChannelSource::Constant;
	}
};

/** Everything one pack run needs: four channel supplies, and where the result goes. */
USTRUCT(BlueprintType)
struct NEXUSTOOLINGEDITOR_API FNTexturePackRequest
{
	GENERATED_BODY()

	/** The four output channels in RGBA order. Anything other than four entries is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TArray<FNTexturePackSlot> Slots;

	/** Content-browser folder the texture is written to ("/Game/Art/Rock"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	FString PackagePath;

	/** Name of the generated asset, suffix included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	FString AssetName;

	/** Output width, or 0 to take the widest input. Inputs of another size are resampled onto it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	int32 OutputSizeX = 0;

	/** Output height, or 0 to take the tallest input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	int32 OutputSizeY = 0;

	/** Writes 16 bits per channel instead of 8. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bSixteenBit = false;

	/** Whether the generated texture is gamma-encoded. Off for anything packed, which is all of it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bSRGB = false;

	/** Compression assigned to the generated texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TEnumAsByte<TextureCompressionSettings> Compression = TC_Masks;

	/** Check an existing asset out of revision control rather than failing on a read-only file. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bCheckOut = true;

	/** Replace an asset already standing at the output path. Without it, an existing asset is an error. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bOverwriteExisting = false;
};

/** One texture an unpack run will produce, covering a run of adjacent channels of the source. */
USTRUCT(BlueprintType)
struct NEXUSTOOLINGEDITOR_API FNTextureUnpackOutput
{
	GENERATED_BODY()

	/** Whether this output is written. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bEnabled = true;

	/** Role the channels carry; drives the suffix, sRGB flag and compression proposed for the asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	FName RoleId;

	/** First channel of the source this output reads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	ENTextureChannel FirstChannel = ENTextureChannel::Red;

	/** How many adjacent channels it reads, 1-4. Anything above one produces a color texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling", meta = (ClampMin = "1", ClampMax = "4"))
	int32 Length = 1;

	/** Name of the generated asset, suffix included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	FString AssetName;

	/** Whether the generated texture is gamma-encoded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bSRGB = false;

	/** Compression assigned to the generated texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TEnumAsByte<TextureCompressionSettings> Compression = TC_Alpha;
};

/** Everything one unpack run needs: the packed texture, and what to pull out of it. */
USTRUCT(BlueprintType)
struct NEXUSTOOLINGEDITOR_API FNTextureUnpackRequest
{
	GENERATED_BODY()

	/** The packed texture being taken apart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TObjectPtr<UTexture2D> Texture = nullptr;

	/** The textures to produce. An empty list is an error rather than a no-op. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	TArray<FNTextureUnpackOutput> Outputs;

	/** Content-browser folder the textures are written to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	FString PackagePath;

	/** Writes 16 bits per channel instead of 8. Ignored for a gamma-encoded output, which must be 8. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bSixteenBit = false;

	/** Takes the source's stored values at face value, ignoring its sRGB flag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bTreatSourceAsLinear = false;

	/** Check existing assets out of revision control rather than failing on read-only files. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bCheckOut = true;

	/** Replace assets already standing at the output paths. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NEXUS|Tooling")
	bool bOverwriteExisting = false;
};

/** What a run produced, and everything it wants to say about how it went. */
USTRUCT(BlueprintType)
struct NEXUSTOOLINGEDITOR_API FNTexturePackerResult
{
	GENERATED_BODY()

	/** Whether the run completed. A false result leaves nothing behind. */
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Tooling")
	bool bSucceeded = false;

	/** Textures written, in the order they were requested. */
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Tooling")
	TArray<TObjectPtr<UTexture2D>> Textures;

	/** Things worth knowing about a run that still succeeded - a resample, a precision loss, a guess. */
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Tooling")
	TArray<FText> Warnings;

	/** Why the run stopped, where it did. */
	UPROPERTY(BlueprintReadOnly, Category = "NEXUS|Tooling")
	FText Error;
};

/**
 * Executes the Texture Packer's two operations, and builds a request for either from asset names alone.
 *
 * Deliberately free of Slate: the window fills a request and calls in here, and so do the Content Browser
 * actions and UNTexturePackerLibrary. Everything that decides anything - what an output is called, what
 * compression it gets, which channels belong together - is in the Build* functions rather than in the UI, so
 * a batch run over a folder makes the same choices a person would see proposed in the window.
 */
class NEXUSTOOLINGEDITOR_API FNTexturePackerJob
{
public:
	/** Combines the request's four channel supplies into one texture. */
	static void Pack(const FNTexturePackRequest& Request, FNTexturePackerResult& OutResult);

	/** Writes one texture per enabled output of the request. */
	static void Unpack(const FNTextureUnpackRequest& Request, FNTexturePackerResult& OutResult);

	/**
	 * Proposes a pack of Textures, reading each one's role from its name and looking the result up as a layout.
	 *
	 * Channel order follows the matched layout where the roles are recognized; failing that the textures are
	 * assigned in the order given. The output lands beside the inputs, named for their common base.
	 *
	 * @return false with OutResult carrying the reason where no arrangement could be proposed.
	 */
	static bool BuildAutoPackRequest(const UNTexturePackerSettings& Settings,
		const TArray<UTexture2D*>& Textures, FNTexturePackRequest& OutRequest, FNTexturePackerResult& OutResult);

	/**
	 * Proposes an unpack of Texture, reading its layout from its name.
	 *
	 * Channels carrying the same role are grouped into one output, and a channel holding a single value
	 * throughout is left switched off where the settings ask for that - an unused alpha is not a map.
	 *
	 * @return false with OutResult carrying the reason where the name was not recognized.
	 */
	static bool BuildAutoUnpackRequest(const UNTexturePackerSettings& Settings, UTexture2D* Texture,
		FNTextureUnpackRequest& OutRequest, FNTexturePackerResult& OutResult);

	/**
	 * Fills in the name, sRGB flag and compression an output should carry for the role it holds.
	 * @param BaseName The source asset's name with its suffix removed.
	 */
	static void ApplyRoleToOutput(const UNTexturePackerSettings& Settings, const FString& BaseName,
		FNTextureUnpackOutput& Output);
};
