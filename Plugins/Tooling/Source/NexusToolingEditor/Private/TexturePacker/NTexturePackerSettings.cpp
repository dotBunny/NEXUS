// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerSettings.h"

#if WITH_EDITORONLY_DATA

namespace NEXUS::ToolingEditor::TexturePacker::Defaults
{
	/**
	 * Builds one role row. Compression defaults suit measured data: BC4 for a lone channel, and the
	 * uncompressed-alpha-aware masks profile where the role spans several.
	 */
	FNTextureChannelRole MakeRole(const FName Id, const TCHAR* DisplayName, const TArray<FString>& Suffixes,
		const bool bIndustryStandard, const bool bSRGB = false,
		const TextureCompressionSettings SingleChannel = TC_Alpha,
		const TextureCompressionSettings MultiChannel = TC_Masks)
	{
		FNTextureChannelRole Role;
		Role.Id = Id;
		Role.DisplayName = DisplayName;
		Role.Suffixes = Suffixes;
		Role.bIndustryStandard = bIndustryStandard;
		Role.bOutputSRGB = bSRGB;
		Role.SingleChannelCompression = SingleChannel;
		Role.MultiChannelCompression = MultiChannel;
		return Role;
	}

	/** Builds one layout row. Pass NAME_None for a channel the arrangement leaves unused. */
	FNTexturePackedLayout MakeLayout(const TCHAR* Suffix, const TCHAR* DisplayName, const FName Red,
		const FName Green, const FName Blue, const FName Alpha, const bool bIndustryStandard)
	{
		FNTexturePackedLayout Layout;
		Layout.Suffix = Suffix;
		Layout.DisplayName = DisplayName;
		Layout.RedRole = Red;
		Layout.GreenRole = Green;
		Layout.BlueRole = Blue;
		Layout.AlphaRole = Alpha;
		Layout.bIndustryStandard = bIndustryStandard;
		Layout.Compression = TC_Masks;
		return Layout;
	}
}

UNTexturePackerSettings::UNTexturePackerSettings()
{
	using namespace NEXUS::ToolingEditor::TexturePacker::Defaults;

	const FName BaseColor = TEXT("BaseColor");
	const FName Normal = TEXT("Normal");
	const FName Roughness = TEXT("Roughness");
	const FName Metallic = TEXT("Metallic");
	const FName AmbientOcclusion = TEXT("AmbientOcclusion");
	const FName Height = TEXT("Height");
	const FName Specular = TEXT("Specular");
	const FName Emissive = TEXT("Emissive");
	const FName Opacity = TEXT("Opacity");
	const FName Smoothness = TEXT("Smoothness");
	const FName Translucency = TEXT("Translucency");
	const FName Mask = TEXT("Mask");

	// Color roles are the only ones written back gamma-encoded; everything else is measured data that a
	// material samples raw, so it round-trips through the packer linear and sRGB-free.
	Roles = {
		MakeRole(BaseColor, TEXT("Base Color"), {TEXT("BC"), TEXT("D"), TEXT("C"), TEXT("Albedo"), TEXT("Diffuse")},
			true, true, TC_Default, TC_Default),
		MakeRole(Normal, TEXT("Normal"), {TEXT("N"), TEXT("NRM"), TEXT("Normal")},
			true, false, TC_Normalmap, TC_Normalmap),
		MakeRole(Roughness, TEXT("Roughness"), {TEXT("R"), TEXT("Rough")}, true),
		MakeRole(Metallic, TEXT("Metallic"), {TEXT("M"), TEXT("Metal")}, true),
		MakeRole(AmbientOcclusion, TEXT("Ambient Occlusion"), {TEXT("AO"), TEXT("O"), TEXT("Occlusion")}, true),
		MakeRole(Height, TEXT("Height"), {TEXT("H"), TEXT("Disp"), TEXT("Displacement")}, true),
		MakeRole(Specular, TEXT("Specular"), {TEXT("S"), TEXT("Spec")}, true),
		MakeRole(Emissive, TEXT("Emissive"), {TEXT("E"), TEXT("Emissive")}, true, true, TC_Default, TC_Default),
		MakeRole(Opacity, TEXT("Opacity"), {TEXT("A"), TEXT("Alpha"), TEXT("Opacity")}, true),
		MakeRole(Smoothness, TEXT("Smoothness"), {TEXT("Smooth"), TEXT("Gloss")}, true),
		MakeRole(TEXT("BentNormal"), TEXT("Bent Normal"), {TEXT("BN")}, false, false, TC_Normalmap, TC_Normalmap),
		MakeRole(TEXT("Subsurface"), TEXT("Subsurface"), {TEXT("SSS")}, false),
		MakeRole(TEXT("Cavity"), TEXT("Cavity"), {TEXT("CAV")}, false),
		MakeRole(TEXT("Curvature"), TEXT("Curvature"), {TEXT("CV")}, false),
		MakeRole(Translucency, TEXT("Translucency"), {TEXT("T"), TEXT("Trans")}, false),
		MakeRole(Mask, TEXT("Mask"), {TEXT("MSK")}, false)
	};

	// Where two rows describe the same arrangement, the one the project should propose comes first: ORM
	// before its ARM synonym, MRA before MRAO. Suffix lookups reach either.
	Layouts = {
		MakeLayout(TEXT("ORM"), TEXT("Occlusion / Roughness / Metallic"),
			AmbientOcclusion, Roughness, Metallic, NAME_None, true),
		MakeLayout(TEXT("ARM"), TEXT("Occlusion / Roughness / Metallic"),
			AmbientOcclusion, Roughness, Metallic, NAME_None, true),
		MakeLayout(TEXT("RMA"), TEXT("Roughness / Metallic / Occlusion"),
			Roughness, Metallic, AmbientOcclusion, NAME_None, true),
		MakeLayout(TEXT("MRA"), TEXT("Metallic / Roughness / Occlusion"),
			Metallic, Roughness, AmbientOcclusion, NAME_None, true),
		MakeLayout(TEXT("MRAO"), TEXT("Metallic / Roughness / Occlusion"),
			Metallic, Roughness, AmbientOcclusion, NAME_None, true),
		MakeLayout(TEXT("ORMH"), TEXT("Occlusion / Roughness / Metallic / Height"),
			AmbientOcclusion, Roughness, Metallic, Height, true),
		MakeLayout(TEXT("RMAH"), TEXT("Roughness / Metallic / Occlusion / Height"),
			Roughness, Metallic, AmbientOcclusion, Height, false),
		MakeLayout(TEXT("MSR"), TEXT("Metallic / Specular / Roughness"),
			Metallic, Specular, Roughness, NAME_None, false),
		MakeLayout(TEXT("CA"), TEXT("Color / Alpha"),
			BaseColor, BaseColor, BaseColor, Opacity, false),
		MakeLayout(TEXT("NAH"), TEXT("Normal / Occlusion / Height"),
			Normal, Normal, AmbientOcclusion, Height, false),
		MakeLayout(TEXT("NT"), TEXT("Normal / Translucency"),
			Normal, Normal, Normal, Translucency, false),
		MakeLayout(TEXT("MSK"), TEXT("Mask"),
			Mask, Mask, Mask, Mask, false)
	};
}

const FNTextureChannelRole* UNTexturePackerSettings::FindRole(const FName Id) const
{
	if (Id.IsNone()) return nullptr;

	return Roles.FindByPredicate([Id](const FNTextureChannelRole& Role) { return Role.Id == Id; });
}

const FNTextureChannelRole* UNTexturePackerSettings::FindRoleBySuffix(const FString& Suffix) const
{
	if (Suffix.IsEmpty()) return nullptr;

	for (const FNTextureChannelRole& Role : Roles)
	{
		for (const FString& Candidate : Role.Suffixes)
		{
			if (Candidate.Equals(Suffix, ESearchCase::IgnoreCase))
			{
				return &Role;
			}
		}
	}

	return nullptr;
}

const FNTexturePackedLayout* UNTexturePackerSettings::FindLayoutBySuffix(const FString& Suffix) const
{
	if (Suffix.IsEmpty()) return nullptr;

	return Layouts.FindByPredicate([&Suffix](const FNTexturePackedLayout& Layout)
		{
			return Layout.Suffix.Equals(Suffix, ESearchCase::IgnoreCase);
		});
}

const FNTexturePackedLayout* UNTexturePackerSettings::FindLayoutByRoles(const TArray<FName>& InRoles) const
{
	if (InRoles.Num() != 4) return nullptr;

	// An all-empty signature would match a layout of four Nones, which is not a thing anybody means by it.
	bool bHasAnyRole = false;
	for (const FName& Role : InRoles)
	{
		if (!Role.IsNone())
		{
			bHasAnyRole = true;
			break;
		}
	}
	if (!bHasAnyRole) return nullptr;

	return Layouts.FindByPredicate([&InRoles](const FNTexturePackedLayout& Layout)
		{
			return Layout.MatchesRoles(InRoles);
		});
}

void UNTexturePackerSettings::GetAllSuffixes(TArray<FString>& OutSuffixes) const
{
	OutSuffixes.Reset();

	for (const FNTextureChannelRole& Role : Roles)
	{
		for (const FString& Suffix : Role.Suffixes)
		{
			OutSuffixes.AddUnique(Suffix);
		}
	}

	for (const FNTexturePackedLayout& Layout : Layouts)
	{
		OutSuffixes.AddUnique(Layout.Suffix);
	}

	// Longest first, so a greedy walk over a composite suffix takes AO before it takes A.
	OutSuffixes.Sort([](const FString& A, const FString& B) { return A.Len() > B.Len(); });
}

#endif // WITH_EDITORONLY_DATA
