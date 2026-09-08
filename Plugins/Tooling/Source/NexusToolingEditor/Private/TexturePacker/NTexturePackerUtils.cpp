// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserDataSubsystem.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserDataModule.h"
#include "IContentBrowserSingleton.h"
#include "ImageCore.h"
#include "NEditorUtils.h"
#include "SourceControlHelpers.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker
{
	/** How far apart two channel values may sit and still read as uniform: one step of an 8-bit encoding. */
	constexpr float UniformTolerance = 1.0f / 255.0f;

	/** Rec. 709 luma weights, used when a color source is folded down to a single channel. */
	constexpr float LuminanceRed = 0.2126f;
	constexpr float LuminanceGreen = 0.7152f;
	constexpr float LuminanceBlue = 0.0722f;
}

FNTextureNameParts FNTexturePackerUtils::SplitAssetName(const UNTexturePackerSettings& Settings,
	const FString& AssetName)
{
	FNTextureNameParts Parts;
	Parts.BaseName = AssetName;

	int32 UnderscoreIndex = INDEX_NONE;
	if (!AssetName.FindLastChar(TEXT('_'), UnderscoreIndex) || UnderscoreIndex >= AssetName.Len() - 1)
	{
		return Parts;
	}

	const FString Candidate = AssetName.RightChop(UnderscoreIndex + 1);

	// A trailing token is only taken as a suffix where the conventions actually name it. Anything else is
	// part of what the asset is called - "T_Rock_01" must not come back as "T_Rock" holding a "01" map.
	if (Settings.FindRoleBySuffix(Candidate) == nullptr && Settings.FindLayoutBySuffix(Candidate) == nullptr)
	{
		return Parts;
	}

	Parts.BaseName = AssetName.Left(UnderscoreIndex);
	Parts.Suffix = Candidate;
	return Parts;
}

FString FNTexturePackerUtils::CommonBaseName(const TArray<FString>& BaseNames)
{
	if (BaseNames.Num() == 0) return FString();

	TArray<FString> Shared;
	BaseNames[0].ParseIntoArray(Shared, TEXT("_"), false);

	for (int32 i = 1; i < BaseNames.Num(); i++)
	{
		TArray<FString> Tokens;
		BaseNames[i].ParseIntoArray(Tokens, TEXT("_"), false);

		int32 Matched = 0;
		while (Matched < Shared.Num() && Matched < Tokens.Num() &&
			Shared[Matched].Equals(Tokens[Matched], ESearchCase::IgnoreCase))
		{
			Matched++;
		}

		Shared.SetNum(Matched);
		if (Shared.Num() == 0) break;
	}

	return FString::Join(Shared, TEXT("_"));
}

FString FNTexturePackerUtils::ComposeAssetName(const FString& BaseName, const FString& Suffix)
{
	if (Suffix.IsEmpty()) return BaseName;
	if (BaseName.IsEmpty()) return Suffix;
	return FString::Printf(TEXT("%s_%s"), *BaseName, *Suffix);
}

FString FNTexturePackerUtils::ComposeSuffixForRoles(const UNTexturePackerSettings& Settings,
	const TArray<FName>& InRoles)
{
	FString Suffix;

	for (const FNTextureChannelRun& Run : BuildChannelRuns(InRoles))
	{
		if (const FNTextureChannelRole* Role = Settings.FindRole(Run.RoleId))
		{
			Suffix += Role->GetCanonicalSuffix();
		}
	}

	return Suffix;
}

bool FNTexturePackerUtils::DecomposeSuffixToRoles(const UNTexturePackerSettings& Settings,
	const FString& Suffix, TArray<FName>& OutRoles)
{
	OutRoles.Init(NAME_None, 4);
	if (Suffix.IsEmpty()) return false;

	TArray<FString> Candidates;
	Settings.GetAllSuffixes(Candidates);

	TArray<FName> Found;
	int32 Position = 0;
	while (Position < Suffix.Len())
	{
		const FString Remaining = Suffix.RightChop(Position);

		const FNTextureChannelRole* Matched = nullptr;
		int32 MatchedLength = 0;
		for (const FString& Candidate : Candidates)
		{
			// Candidates arrive longest-first, so the first hit is the longest one and the walk cannot take
			// A out of AO and then fail on the O.
			if (Candidate.Len() <= Remaining.Len() && Remaining.StartsWith(Candidate, ESearchCase::IgnoreCase))
			{
				if (const FNTextureChannelRole* Role = Settings.FindRoleBySuffix(Candidate))
				{
					Matched = Role;
					MatchedLength = Candidate.Len();
					break;
				}
			}
		}

		if (Matched == nullptr || MatchedLength == 0) return false;

		Found.Add(Matched->Id);
		if (Found.Num() > 4) return false;

		Position += MatchedLength;
	}

	if (Found.Num() == 0) return false;

	for (int32 i = 0; i < Found.Num(); i++)
	{
		OutRoles[i] = Found[i];
	}

	return true;
}

FName FNTexturePackerUtils::InferRole(const UNTexturePackerSettings& Settings, const FString& AssetName)
{
	const FNTextureNameParts Parts = SplitAssetName(Settings, AssetName);
	if (Parts.Suffix.IsEmpty()) return NAME_None;

	const FNTextureChannelRole* Role = Settings.FindRoleBySuffix(Parts.Suffix);
	return Role != nullptr ? Role->Id : NAME_None;
}

bool FNTexturePackerUtils::DetectLayout(const UNTexturePackerSettings& Settings, const FString& AssetName,
	FNTexturePackedLayout& OutLayout, FString& OutBaseName)
{
	OutLayout = FNTexturePackedLayout();

	const FNTextureNameParts Parts = SplitAssetName(Settings, AssetName);
	OutBaseName = Parts.BaseName;

	if (Parts.Suffix.IsEmpty()) return false;

	if (const FNTexturePackedLayout* Known = Settings.FindLayoutBySuffix(Parts.Suffix))
	{
		OutLayout = *Known;
		return true;
	}

	// A suffix naming a single role is a texture holding one map across every channel it uses, not a packed
	// one - an unpack of it is a no-op, but reading it as its own role is still the honest answer.
	if (const FNTextureChannelRole* Role = Settings.FindRoleBySuffix(Parts.Suffix))
	{
		OutLayout.Suffix = Parts.Suffix;
		OutLayout.DisplayName = Role->DisplayName;
		OutLayout.RedRole = Role->Id;
		OutLayout.Compression = Role->SingleChannelCompression;
		return true;
	}

	TArray<FName> Decomposed;
	if (!DecomposeSuffixToRoles(Settings, Parts.Suffix, Decomposed)) return false;

	OutLayout.Suffix = Parts.Suffix;
	OutLayout.RedRole = Decomposed[0];
	OutLayout.GreenRole = Decomposed[1];
	OutLayout.BlueRole = Decomposed[2];
	OutLayout.AlphaRole = Decomposed[3];
	return true;
}

TArray<FNTextureChannelRun> FNTexturePackerUtils::BuildChannelRuns(const TArray<FName>& InRoles)
{
	TArray<FNTextureChannelRun> Runs;

	for (int32 i = 0; i < InRoles.Num(); i++)
	{
		if (InRoles[i].IsNone()) continue;

		if (Runs.Num() > 0)
		{
			FNTextureChannelRun& Last = Runs.Last();
			if (Last.RoleId == InRoles[i] && static_cast<int32>(Last.FirstChannel) + Last.Length == i)
			{
				Last.Length++;
				continue;
			}
		}

		FNTextureChannelRun Run;
		Run.RoleId = InRoles[i];
		Run.FirstChannel = static_cast<ENTextureChannel>(i);
		Run.Length = 1;
		Runs.Add(Run);
	}

	return Runs;
}

bool FNTexturePackerUtils::ReadTextureLinear(UTexture2D* Texture, FImage& OutImage,
	const bool bTreatAsLinear, FText& OutError)
{
	if (Texture == nullptr)
	{
		OutError = LOCTEXT("Read_NoTexture", "No texture was supplied.");
		return false;
	}

	FTextureSource& Source = Texture->Source;
	if (!Source.IsValid())
	{
		OutError = FText::Format(LOCTEXT("Read_NoSource",
			"'{0}' has no source art. Textures created at runtime or stripped of their source cannot be read."),
			FText::FromString(Texture->GetName()));
		return false;
	}

	if (Source.GetNumBlocks() > 1)
	{
		OutError = FText::Format(LOCTEXT("Read_Blocked",
			"'{0}' is a UDIM texture, which the Texture Packer does not handle."),
			FText::FromString(Texture->GetName()));
		return false;
	}

	FImage Raw;
	if (!Source.GetMipImage(Raw, 0))
	{
		OutError = FText::Format(LOCTEXT("Read_MipFailed", "Could not read the source art of '{0}'."),
			FText::FromString(Texture->GetName()));
		return false;
	}

	// The gamma the bytes are actually in is the texture's SRGB flag, not the source format's default - a
	// material sampler decodes on that flag, and reading it any other way would hand back values the shader
	// never sees. Formats that cannot hold gamma (16-bit and float) are always linear regardless.
	EGammaSpace Gamma = EGammaSpace::Linear;
	if (!bTreatAsLinear && Texture->SRGB && ERawImageFormat::GetFormatNeedsGammaSpace(Raw.Format))
	{
		Gamma = EGammaSpace::sRGB;
	}

	const FImageView View(Raw.RawData.GetData(), Raw.SizeX, Raw.SizeY, Raw.NumSlices, Raw.Format, Gamma);
	View.CopyTo(OutImage, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
	return true;
}

bool FNTexturePackerUtils::IsChannelUniform(const FImage& LinearImage, const ENTextureChannel Channel)
{
	const TArrayView64<const FLinearColor> Pixels = LinearImage.AsRGBA32F();
	if (Pixels.Num() == 0) return true;

	const int32 Index = static_cast<int32>(Channel);
	const float First = Pixels[0].Component(Index);

	for (int64 i = 1; i < Pixels.Num(); i++)
	{
		if (FMath::Abs(Pixels[i].Component(Index) - First) > NEXUS::ToolingEditor::TexturePacker::UniformTolerance)
		{
			return false;
		}
	}

	return true;
}

float FNTexturePackerUtils::SampleChannel(const FLinearColor& Pixel, const ENTextureChannelSource Source,
	const ENTextureChannel TargetChannel, const bool bSourceIsSingleChannel)
{
	using namespace NEXUS::ToolingEditor::TexturePacker;

	switch (Source)
	{
	case ENTextureChannelSource::Red: return Pixel.R;
	case ENTextureChannelSource::Green: return Pixel.G;
	case ENTextureChannelSource::Blue: return Pixel.B;
	case ENTextureChannelSource::Alpha: return Pixel.A;
	case ENTextureChannelSource::Luminance:
		return Pixel.R * LuminanceRed + Pixel.G * LuminanceGreen + Pixel.B * LuminanceBlue;
	case ENTextureChannelSource::Auto:
		// A grayscale source keeps everything it has in red; a wider one lines up channel for channel.
		return bSourceIsSingleChannel ? Pixel.R : Pixel.Component(static_cast<int32>(TargetChannel));
	default:
		return 0.0f;
	}
}

bool FNTexturePackerUtils::IsSingleChannelSource(const UTexture2D* Texture)
{
	if (Texture == nullptr) return false;

	switch (Texture->Source.GetFormat())
	{
	case TSF_G8:
	case TSF_G16:
	case TSF_R16F:
	case TSF_R32F:
		return true;
	default:
		return false;
	}
}

UTexture2D* FNTexturePackerUtils::CreateOrUpdateTexture(const FString& PackagePath, const FString& AssetName,
	const FImage& Image, const bool bSRGB, const TextureCompressionSettings Compression,
	const UTexture2D* SettingsTemplate, const bool bCheckOut, FText& OutError)
{
	if (PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		OutError = LOCTEXT("Write_NoPath", "An output path and asset name are both required.");
		return nullptr;
	}

	const FString PackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *AssetName);
	if (!FPackageName::IsValidLongPackageName(PackageName))
	{
		OutError = FText::Format(LOCTEXT("Write_BadPath", "'{0}' is not a valid asset path."),
			FText::FromString(PackageName));
		return nullptr;
	}

	UPackage* Package = CreatePackage(*PackageName);
	if (Package == nullptr)
	{
		OutError = FText::Format(LOCTEXT("Write_NoPackage", "Could not create a package at '{0}'."),
			FText::FromString(PackageName));
		return nullptr;
	}

	Package->FullyLoad();

	UTexture2D* Texture = FindObject<UTexture2D>(Package, *AssetName);
	const bool bIsNew = Texture == nullptr;

	if (bIsNew)
	{
		// Something that is not a texture standing on the name is a refusal rather than an overwrite - the
		// tool replaces the contents of textures, it does not replace assets with textures.
		if (FindObject<UObject>(Package, *AssetName) != nullptr)
		{
			OutError = FText::Format(LOCTEXT("Write_NotATexture",
				"'{0}' already exists and is not a Texture2D."), FText::FromString(PackageName));
			return nullptr;
		}

		Texture = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone);
		if (Texture == nullptr)
		{
			OutError = FText::Format(LOCTEXT("Write_NoTexture", "Could not create a texture at '{0}'."),
				FText::FromString(PackageName));
			return nullptr;
		}
	}
	else if (bCheckOut)
	{
		// Only worth asking for a file that is actually on disk; a package created moments ago has none, and
		// a checkout of a path that does not exist reports a failure that means nothing.
		const FString Filename = USourceControlHelpers::PackageFilename(Package);
		if (!Filename.IsEmpty() && FPaths::FileExists(Filename) &&
			!USourceControlHelpers::CheckOutOrAddFile(Filename, true) &&
			IFileManager::Get().IsReadOnly(*Filename))
		{
			OutError = FText::Format(LOCTEXT("Write_ReadOnly",
				"'{0}' is read-only and could not be checked out."), FText::FromString(Filename));
			return nullptr;
		}
	}

	// Init is required to sit inside a PreEdit/PostEdit pair - it re-hashes the source and the texture has to
	// be told to rebuild its platform data off the result.
	Texture->PreEditChange(nullptr);

	if (SettingsTemplate != nullptr && bIsNew)
	{
		Texture->MipGenSettings = SettingsTemplate->MipGenSettings;
		Texture->AddressX = SettingsTemplate->AddressX;
		Texture->AddressY = SettingsTemplate->AddressY;
		Texture->Filter = SettingsTemplate->Filter;
		Texture->LODGroup = SettingsTemplate->LODGroup;
	}

	Texture->Source.Init(Image);
	Texture->SRGB = bSRGB;
	Texture->CompressionSettings = Compression;
	Texture->PostEditChange();

	if (bIsNew)
	{
		FAssetRegistryModule::AssetCreated(Texture);
	}

	Package->MarkPackageDirty();
	return Texture;
}

FString FNTexturePackerUtils::GetPackagePath(const UObject* Asset)
{
	if (Asset == nullptr) return FString();

	const UPackage* Package = Asset->GetOutermost();
	if (Package == nullptr) return FString();

	return FPackageName::GetLongPackagePath(Package->GetName());
}

bool FNTexturePackerUtils::DoesAssetExist(const FString& PackagePath, const FString& AssetName)
{
	if (PackagePath.IsEmpty() || AssetName.IsEmpty()) return false;

	const FString PackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *AssetName);
	if (!FPackageName::IsValidLongPackageName(PackageName)) return false;

	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const FAssetData Existing = AssetRegistryModule.Get().GetAssetByObjectPath(
		FSoftObjectPath(FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName)));

	return Existing.IsValid();
}

void FNTexturePackerUtils::SelectAssetsInContentBrowser(const TArray<UObject*>& Assets)
{
	if (Assets.Num() == 0) return;

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
}

FString FNTexturePackerUtils::GetContentBrowserPath()
{
	// The Content Browser deals in VIRTUAL paths - a folder at the root of Content is "/All/Game/Test" to it,
	// not "/Game/Test" - and nothing that writes an asset can use one. The subsystem is what knows the
	// mapping, since which prefixes exist depends on the browser's own settings.
	const UContentBrowserDataSubsystem* DataSubsystem = IContentBrowserDataModule::Get().GetSubsystem();
	if (DataSubsystem == nullptr) return FString();

	// Taking the first that converts rather than the first selected: a purely virtual folder ("/All", or a
	// grouping node) has no internal path at all, and skipping past one is better than reporting nothing.
	for (const FString& VirtualPath : FNEditorUtils::GetSelectedContentBrowserPaths())
	{
		FString InternalPath;
		if (DataSubsystem->TryConvertVirtualPath(VirtualPath, InternalPath) == EContentBrowserPathType::Internal)
		{
			return InternalPath;
		}
	}

	return FString();
}

#undef LOCTEXT_NAMESPACE
