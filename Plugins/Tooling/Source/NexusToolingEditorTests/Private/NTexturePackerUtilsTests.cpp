// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "TexturePacker/NTexturePackerUtils.h"
#include "UObject/StrongObjectPtr.h"

namespace NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness
{
	/**
	 * @return A settings object holding exactly the shipped defaults.
	 * @note NewObject rather than the CDO on purpose. Config is loaded onto the CDO, so a project that has
	 *       retuned its conventions would otherwise be what these tests measure.
	 */
	TStrongObjectPtr<UNTexturePackerSettings> MakeSettings()
	{
		return TStrongObjectPtr<UNTexturePackerSettings>(NewObject<UNTexturePackerSettings>());
	}
}

N_TEST_HIGH(FNTexturePackerUtilsTests_SplitAssetName_KnownSuffix,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::SplitAssetName::KnownSuffix",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const FNTextureNameParts Parts = FNTexturePackerUtils::SplitAssetName(*Settings, TEXT("T_Rock_01_ORM"));

	CHECK_EQUALS("The base name should be everything before the suffix.", Parts.BaseName, FString(TEXT("T_Rock_01")))
	CHECK_EQUALS("The suffix should be recognized.", Parts.Suffix, FString(TEXT("ORM")))
}

N_TEST_HIGH(FNTexturePackerUtilsTests_SplitAssetName_UnknownSuffix,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::SplitAssetName::UnknownSuffix",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a trailing token nothing recognizes is left on the base name rather than eaten, which is
	// what stops a numbered asset losing its number.
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const FNTextureNameParts Parts = FNTexturePackerUtils::SplitAssetName(*Settings, TEXT("T_Rock_01"));

	CHECK_EQUALS("An unrecognized token stays on the base name.", Parts.BaseName, FString(TEXT("T_Rock_01")))
	CHECK_MESSAGE(TEXT("No suffix should be reported."), Parts.Suffix.IsEmpty())
}

N_TEST_MEDIUM(FNTexturePackerUtilsTests_SplitAssetName_TrailingUnderscore,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::SplitAssetName::TrailingUnderscore",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const FNTextureNameParts Parts = FNTexturePackerUtils::SplitAssetName(*Settings, TEXT("T_Rock_"));

	CHECK_EQUALS("A name ending in an underscore has no suffix to take.", Parts.BaseName, FString(TEXT("T_Rock_")))
	CHECK_MESSAGE(TEXT("No suffix should be reported."), Parts.Suffix.IsEmpty())
}

N_TEST_HIGH(FNTexturePackerUtilsTests_InferRole_FromSuffix,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::InferRole::FromSuffix",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	CHECK_EQUALS("_R should read as roughness.",
		FNTexturePackerUtils::InferRole(*Settings, TEXT("T_Rock_01_R")).ToString(), FString(TEXT("Roughness")))
	CHECK_EQUALS("_AO should read as ambient occlusion.",
		FNTexturePackerUtils::InferRole(*Settings, TEXT("T_Rock_01_AO")).ToString(), FString(TEXT("AmbientOcclusion")))
	CHECK_EQUALS("_d should match case-insensitively.",
		FNTexturePackerUtils::InferRole(*Settings, TEXT("T_Rock_01_d")).ToString(), FString(TEXT("BaseColor")))
	CHECK_EQUALS("A layout suffix is not a role.",
		FNTexturePackerUtils::InferRole(*Settings, TEXT("T_Rock_01_ORM")).ToString(), FString(TEXT("None")))
}

N_TEST_HIGH(FNTexturePackerUtilsTests_CommonBaseName_TokenWise,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::CommonBaseName::TokenWise",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that names are compared a token at a time, so two differently-numbered assets do not share a
	// half-written number the way a character prefix would give them.
	const TArray<FString> Numbered = { TEXT("T_Rock_01"), TEXT("T_Rock_02") };
	CHECK_EQUALS("Differing numbers should be dropped whole.",
		FNTexturePackerUtils::CommonBaseName(Numbered), FString(TEXT("T_Rock")))

	const TArray<FString> Identical = { TEXT("T_Rock_01"), TEXT("T_Rock_01"), TEXT("T_Rock_01") };
	CHECK_EQUALS("Identical names share all of themselves.",
		FNTexturePackerUtils::CommonBaseName(Identical), FString(TEXT("T_Rock_01")))

	const TArray<FString> Unrelated = { TEXT("T_Rock_01"), TEXT("M_Metal_01") };
	CHECK_MESSAGE(TEXT("Names sharing nothing should produce nothing."),
		FNTexturePackerUtils::CommonBaseName(Unrelated).IsEmpty())

	CHECK_MESSAGE(TEXT("An empty list should produce nothing."),
		FNTexturePackerUtils::CommonBaseName({}).IsEmpty())
}

N_TEST_MEDIUM(FNTexturePackerUtilsTests_ComposeAssetName_Joins,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::ComposeAssetName::Joins",
	N_TEST_CONTEXT_EDITOR)
{
	CHECK_EQUALS("A base and a suffix join with an underscore.",
		FNTexturePackerUtils::ComposeAssetName(TEXT("T_Rock_01"), TEXT("ORM")), FString(TEXT("T_Rock_01_ORM")))
	CHECK_EQUALS("An empty suffix leaves the base alone.",
		FNTexturePackerUtils::ComposeAssetName(TEXT("T_Rock_01"), TEXT("")), FString(TEXT("T_Rock_01")))
	CHECK_EQUALS("An empty base leaves the suffix alone.",
		FNTexturePackerUtils::ComposeAssetName(TEXT(""), TEXT("ORM")), FString(TEXT("ORM")))
}

N_TEST_CRITICAL(FNTexturePackerUtilsTests_DetectLayout_KnownLayout,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::DetectLayout::KnownLayout",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	FNTexturePackedLayout Layout;
	FString BaseName;

	CHECK_MESSAGE(TEXT("An ORM texture should be recognized."),
		FNTexturePackerUtils::DetectLayout(*Settings, TEXT("T_Rock_01_ORM"), Layout, BaseName))
	CHECK_EQUALS("The base name should come back without the suffix.", BaseName, FString(TEXT("T_Rock_01")))
	CHECK_EQUALS("Red should hold occlusion.", Layout.RedRole.ToString(), FString(TEXT("AmbientOcclusion")))
	CHECK_EQUALS("Green should hold roughness.", Layout.GreenRole.ToString(), FString(TEXT("Roughness")))
	CHECK_EQUALS("Blue should hold metallic.", Layout.BlueRole.ToString(), FString(TEXT("Metallic")))
	CHECK_EQUALS("Alpha should hold nothing.", Layout.AlphaRole.ToString(), FString(TEXT("None")))
}

N_TEST_HIGH(FNTexturePackerUtilsTests_DetectLayout_SingleRole,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::DetectLayout::SingleRole",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a texture named for one map reads as that map in red rather than as an unknown.
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	FNTexturePackedLayout Layout;
	FString BaseName;

	CHECK_MESSAGE(TEXT("A roughness map should be recognized."),
		FNTexturePackerUtils::DetectLayout(*Settings, TEXT("T_Rock_01_R"), Layout, BaseName))
	CHECK_EQUALS("Red should hold roughness.", Layout.RedRole.ToString(), FString(TEXT("Roughness")))
	CHECK_EQUALS("Green should hold nothing.", Layout.GreenRole.ToString(), FString(TEXT("None")))
}

N_TEST_HIGH(FNTexturePackerUtilsTests_DetectLayout_Unrecognized,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::DetectLayout::Unrecognized",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	FNTexturePackedLayout Layout;
	FString BaseName;

	CHECK_FALSE_MESSAGE(TEXT("A name saying nothing should not be guessed at."),
		FNTexturePackerUtils::DetectLayout(*Settings, TEXT("T_Rock_01"), Layout, BaseName))
	CHECK_EQUALS("The whole name is the base name.", BaseName, FString(TEXT("T_Rock_01")))
}

N_TEST_CRITICAL(FNTexturePackerUtilsTests_BuildChannelRuns_GroupsAdjacent,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::BuildChannelRuns::GroupsAdjacent",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the rule the whole unpack side rests on: a color map across RGB is one texture to recover.
	const TArray<FName> Roles = { TEXT("BaseColor"), TEXT("BaseColor"), TEXT("BaseColor"), TEXT("Opacity") };
	const TArray<FNTextureChannelRun> Runs = FNTexturePackerUtils::BuildChannelRuns(Roles);

	CHECK_EQUALS("Color and opacity make two runs.", Runs.Num(), 2)
	if (Runs.Num() != 2)
	{
		ADD_ERROR("Run count is wrong, so the rest of the assertions cannot be made.");
		return;
	}

	CHECK_EQUALS("The color run holds base color.", Runs[0].RoleId.ToString(), FString(TEXT("BaseColor")))
	CHECK_EQUALS("The color run starts at red.", static_cast<int32>(Runs[0].FirstChannel), 0)
	CHECK_EQUALS("The color run spans three channels.", Runs[0].Length, 3)
	CHECK_EQUALS("The opacity run starts at alpha.", static_cast<int32>(Runs[1].FirstChannel), 3)
	CHECK_EQUALS("The opacity run spans one channel.", Runs[1].Length, 1)
}

N_TEST_HIGH(FNTexturePackerUtilsTests_BuildChannelRuns_SkipsUnused,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::BuildChannelRuns::SkipsUnused",
	N_TEST_CONTEXT_EDITOR)
{
	const TArray<FName> Roles = { TEXT("AmbientOcclusion"), TEXT("Roughness"), TEXT("Metallic"), NAME_None };
	const TArray<FNTextureChannelRun> Runs = FNTexturePackerUtils::BuildChannelRuns(Roles);

	CHECK_EQUALS("An unused alpha contributes no run.", Runs.Num(), 3)
}

N_TEST_HIGH(FNTexturePackerUtilsTests_BuildChannelRuns_SeparatesNonAdjacent,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::BuildChannelRuns::SeparatesNonAdjacent",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that a role appearing twice with something between it stays two runs; grouping them would
	// claim two unrelated channels are one map.
	const TArray<FName> Roles = { TEXT("Roughness"), TEXT("Metallic"), TEXT("Roughness"), NAME_None };
	const TArray<FNTextureChannelRun> Runs = FNTexturePackerUtils::BuildChannelRuns(Roles);

	CHECK_EQUALS("Split occurrences stay separate.", Runs.Num(), 3)
}

N_TEST_MEDIUM(FNTexturePackerUtilsTests_ComposeSuffixForRoles_CollapsesRuns,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::ComposeSuffixForRoles::CollapsesRuns",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const TArray<FName> Roles = { TEXT("BaseColor"), TEXT("BaseColor"), TEXT("BaseColor"), TEXT("Opacity") };

	CHECK_EQUALS("A three-channel color contributes one letter.",
		FNTexturePackerUtils::ComposeSuffixForRoles(*Settings, Roles), FString(TEXT("BCA")))
}

N_TEST_HIGH(FNTexturePackerUtilsTests_DecomposeSuffixToRoles_LongestFirst,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::DecomposeSuffixToRoles::LongestFirst",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the greedy walk takes AO whole rather than splitting it into an opacity and an occlusion.
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	TArray<FName> Roles;
	CHECK_MESSAGE(TEXT("RMAOH should decompose."),
		FNTexturePackerUtils::DecomposeSuffixToRoles(*Settings, TEXT("RMAOH"), Roles))
	CHECK_EQUALS("Four roles always come back.", Roles.Num(), 4)
	CHECK_EQUALS("Red holds roughness.", Roles[0].ToString(), FString(TEXT("Roughness")))
	CHECK_EQUALS("Green holds metallic.", Roles[1].ToString(), FString(TEXT("Metallic")))
	CHECK_EQUALS("Blue holds occlusion, not opacity.", Roles[2].ToString(), FString(TEXT("AmbientOcclusion")))
	CHECK_EQUALS("Alpha holds height.", Roles[3].ToString(), FString(TEXT("Height")))
}

N_TEST_MEDIUM(FNTexturePackerUtilsTests_DecomposeSuffixToRoles_Rejects,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::DecomposeSuffixToRoles::Rejects",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	TArray<FName> Roles;
	CHECK_FALSE_MESSAGE(TEXT("A suffix that cannot be consumed is refused."),
		FNTexturePackerUtils::DecomposeSuffixToRoles(*Settings, TEXT("ZZZ"), Roles))
	CHECK_FALSE_MESSAGE(TEXT("An empty suffix is refused."),
		FNTexturePackerUtils::DecomposeSuffixToRoles(*Settings, TEXT(""), Roles))
	CHECK_FALSE_MESSAGE(TEXT("More than four roles is refused."),
		FNTexturePackerUtils::DecomposeSuffixToRoles(*Settings, TEXT("RMHSE"), Roles))
}

N_TEST_HIGH(FNTexturePackerSettingsTests_FindLayoutByRoles_FirstWins,
	"NEXUS::UnitTests::NTooling::UNTexturePackerSettings::FindLayoutByRoles::FirstWins",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the documented ordering rule: ORM and ARM describe the same arrangement, and the first row is
	// the spelling the tool proposes.
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const TArray<FName> Roles = { TEXT("AmbientOcclusion"), TEXT("Roughness"), TEXT("Metallic"), NAME_None };
	const FNTexturePackedLayout* Layout = Settings->FindLayoutByRoles(Roles);

	if (Layout == nullptr)
	{
		ADD_ERROR("The occlusion/roughness/metallic arrangement should be recognized.");
		return;
	}

	CHECK_EQUALS("ORM is proposed ahead of its synonyms.", Layout->Suffix, FString(TEXT("ORM")))
}

N_TEST_MEDIUM(FNTexturePackerSettingsTests_FindLayoutByRoles_EmptySignature,
	"NEXUS::UnitTests::NTooling::UNTexturePackerSettings::FindLayoutByRoles::EmptySignature",
	N_TEST_CONTEXT_EDITOR)
{
	const TStrongObjectPtr<UNTexturePackerSettings> Settings =
		NEXUS::UnitTests::NTooling::FNTexturePackerUtilsHarness::MakeSettings();

	const TArray<FName> Empty = { NAME_None, NAME_None, NAME_None, NAME_None };

	if (Settings->FindLayoutByRoles(Empty) != nullptr)
	{
		ADD_ERROR("Four unassigned channels should not match any layout.");
	}
}

N_TEST_HIGH(FNTexturePackerUtilsTests_SampleChannel_Selects,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::SampleChannel::Selects",
	N_TEST_CONTEXT_EDITOR)
{
	const FLinearColor Pixel(0.1f, 0.2f, 0.3f, 0.4f);

	CHECK_EQUALS("Red reads red.",
		FNTexturePackerUtils::SampleChannel(Pixel, ENTextureChannelSource::Red, ENTextureChannel::Blue, false),
		0.1f)
	CHECK_EQUALS("Alpha reads alpha.",
		FNTexturePackerUtils::SampleChannel(Pixel, ENTextureChannelSource::Alpha, ENTextureChannel::Red, false),
		0.4f)
	CHECK_EQUALS("Auto on a wide source reads the matching channel.",
		FNTexturePackerUtils::SampleChannel(Pixel, ENTextureChannelSource::Auto, ENTextureChannel::Blue, false),
		0.3f)
	CHECK_EQUALS("Auto on a grayscale source reads red.",
		FNTexturePackerUtils::SampleChannel(Pixel, ENTextureChannelSource::Auto, ENTextureChannel::Blue, true),
		0.1f)
	CHECK_EQUALS("A constant slot samples nothing.",
		FNTexturePackerUtils::SampleChannel(Pixel, ENTextureChannelSource::Constant, ENTextureChannel::Red, false),
		0.0f)
}

N_TEST_MEDIUM(FNTexturePackerUtilsTests_SampleChannel_Luminance,
	"NEXUS::UnitTests::NTooling::FNTexturePackerUtils::SampleChannel::Luminance",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the Rec. 709 weighting, checked against a flat white pixel where the weights must sum to one.
	const FLinearColor White(1.0f, 1.0f, 1.0f, 1.0f);
	const float Luminance = FNTexturePackerUtils::SampleChannel(White, ENTextureChannelSource::Luminance,
		ENTextureChannel::Red, false);

	CHECK_MESSAGE(TEXT("White should fold down to one."), FMath::IsNearlyEqual(Luminance, 1.0f, KINDA_SMALL_NUMBER))

	const FLinearColor Green(0.0f, 1.0f, 0.0f, 1.0f);
	const float GreenLuminance = FNTexturePackerUtils::SampleChannel(Green, ENTextureChannelSource::Luminance,
		ENTextureChannel::Red, false);

	CHECK_MESSAGE(TEXT("Green should carry the largest share."),
		FMath::IsNearlyEqual(GreenLuminance, 0.7152f, KINDA_SMALL_NUMBER))
}

#endif //WITH_TESTS
