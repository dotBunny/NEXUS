// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerJob.h"

#include "ImageCore.h"
#include "Engine/Texture2D.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "TexturePacker/NTexturePackerUtils.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker::Job
{
	/** The raw format and gamma one generated texture is written in, once precision and encoding are reconciled. */
	struct FResolvedFormat
	{
		ERawImageFormat::Type Format = ERawImageFormat::BGRA8;
		EGammaSpace Gamma = EGammaSpace::Linear;
		bool bForcedEightBit = false;
	};

	/**
	 * @return The format to write ChannelCount channels in.
	 * @note A gamma-encoded output is always 8-bit. ImageCore's wider formats are linear-only, so writing one
	 *       and flagging the texture sRGB would leave linear values labelled as encoded ones - wrong in the
	 *       one direction nobody would look for. The caller reports the demotion.
	 */
	FResolvedFormat ResolveOutputFormat(const bool bWantSixteenBit, const bool bSRGB, const int32 ChannelCount)
	{
		FResolvedFormat Resolved;
		Resolved.bForcedEightBit = bWantSixteenBit && bSRGB;

		const bool bSixteenBit = bWantSixteenBit && !bSRGB;
		if (ChannelCount <= 1)
		{
			Resolved.Format = bSixteenBit ? ERawImageFormat::G16 : ERawImageFormat::G8;
		}
		else
		{
			Resolved.Format = bSixteenBit ? ERawImageFormat::RGBA16 : ERawImageFormat::BGRA8;
		}

		Resolved.Gamma = bSRGB ? EGammaSpace::sRGB : EGammaSpace::Linear;
		return Resolved;
	}

	/** What one output channel of a pack reads, resolved once so the per-pixel loop carries no lookups. */
	struct FChannelPlan
	{
		const FImage* Image = nullptr;
		ENTextureChannelSource Source = ENTextureChannelSource::Constant;
		bool bInvert = false;
		bool bSingleChannelSource = false;
		float ConstantValue = 0.0f;
	};
}

void FNTexturePackerJob::Pack(const FNTexturePackRequest& Request, FNTexturePackerResult& OutResult)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::Job;

	OutResult = FNTexturePackerResult();

	if (Request.Slots.Num() != 4)
	{
		OutResult.Error = LOCTEXT("Pack_SlotCount", "A pack needs exactly four channel slots.");
		return;
	}

	if (!Request.bOverwriteExisting &&
		FNTexturePackerUtils::DoesAssetExist(Request.PackagePath, Request.AssetName))
	{
		OutResult.Error = FText::Format(LOCTEXT("Pack_Exists",
			"'{0}' already exists. Choose another name, or allow it to be overwritten."),
			FText::FromString(Request.AssetName));
		return;
	}

	// One read per texture-and-gamma pair rather than per channel, since packing three channels out of one
	// source is the common case for a mask that was authored as a color image.
	TMap<TPair<const UTexture2D*, bool>, TSharedPtr<FImage>> Sources;
	int32 Width = FMath::Max(Request.OutputSizeX, 0);
	int32 Height = FMath::Max(Request.OutputSizeY, 0);
	bool bAnyEncodedSource = false;

	for (const FNTexturePackSlot& Slot : Request.Slots)
	{
		if (!Slot.HasTexture()) continue;

		const TPair<const UTexture2D*, bool> Key(Slot.Texture.Get(), Slot.bTreatSourceAsLinear);
		if (Sources.Contains(Key)) continue;

		TSharedPtr<FImage> Image = MakeShared<FImage>();
		FText ReadError;
		if (!FNTexturePackerUtils::ReadTextureLinear(Slot.Texture, *Image, Slot.bTreatSourceAsLinear, ReadError))
		{
			OutResult.Error = ReadError;
			return;
		}

		if (Slot.Texture->SRGB && !Slot.bTreatSourceAsLinear)
		{
			bAnyEncodedSource = true;
		}

		Sources.Add(Key, Image);
	}

	if (Sources.Num() == 0)
	{
		OutResult.Error = LOCTEXT("Pack_NoSources", "At least one channel has to read a texture.");
		return;
	}

	if (Width <= 0 || Height <= 0)
	{
		for (const TPair<TPair<const UTexture2D*, bool>, TSharedPtr<FImage>>& Entry : Sources)
		{
			Width = FMath::Max(Width, Entry.Value->SizeX);
			Height = FMath::Max(Height, Entry.Value->SizeY);
		}
	}

	// Resample anything that is not already the output size. Doing it rather than refusing is deliberate:
	// a set where the height map was authored at half resolution is normal, and worth saying so about.
	for (TPair<TPair<const UTexture2D*, bool>, TSharedPtr<FImage>>& Entry : Sources)
	{
		FImage& Image = *Entry.Value;
		if (Image.SizeX == Width && Image.SizeY == Height) continue;

		OutResult.Warnings.Add(FText::Format(LOCTEXT("Pack_Resized",
			"'{0}' was resampled from {1}x{2} to {3}x{4}."),
			FText::FromString(Entry.Key.Key->GetName()), Image.SizeX, Image.SizeY, Width, Height));

		FImage Resized;
		Image.ResizeTo(Resized, Width, Height, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
		Image = MoveTemp(Resized);
	}

	TArray<FChannelPlan> Plans;
	Plans.SetNum(4);
	for (int32 i = 0; i < 4; i++)
	{
		const FNTexturePackSlot& Slot = Request.Slots[i];
		FChannelPlan& Plan = Plans[i];

		if (Slot.HasTexture())
		{
			const TPair<const UTexture2D*, bool> Key(Slot.Texture.Get(), Slot.bTreatSourceAsLinear);
			Plan.Image = Sources[Key].Get();
			Plan.Source = Slot.Source;
			Plan.bInvert = Slot.bInvert;
			Plan.bSingleChannelSource = FNTexturePackerUtils::IsSingleChannelSource(Slot.Texture);
		}
		else
		{
			Plan.ConstantValue = FMath::Clamp(Slot.ConstantValue, 0.0f, 1.0f);
		}
	}

	FImage Packed(Width, Height, 1, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
	const TArrayView64<FLinearColor> Destination = Packed.AsRGBA32F();

	for (int32 ChannelIndex = 0; ChannelIndex < 4; ChannelIndex++)
	{
		const FChannelPlan& Plan = Plans[ChannelIndex];
		const ENTextureChannel Channel = static_cast<ENTextureChannel>(ChannelIndex);

		if (Plan.Image == nullptr)
		{
			for (int64 Pixel = 0; Pixel < Destination.Num(); Pixel++)
			{
				Destination[Pixel].Component(ChannelIndex) = Plan.ConstantValue;
			}
			continue;
		}

		const TArrayView64<const FLinearColor> Source = Plan.Image->AsRGBA32F();
		for (int64 Pixel = 0; Pixel < Destination.Num(); Pixel++)
		{
			float Value = FNTexturePackerUtils::SampleChannel(Source[Pixel], Plan.Source, Channel,
				Plan.bSingleChannelSource);
			if (Plan.bInvert)
			{
				Value = 1.0f - Value;
			}
			Destination[Pixel].Component(ChannelIndex) = Value;
		}
	}

	const FResolvedFormat Resolved = ResolveOutputFormat(Request.bSixteenBit, Request.bSRGB, 4);
	if (Resolved.bForcedEightBit)
	{
		OutResult.Warnings.Add(LOCTEXT("Pack_ForcedEightBit",
			"The output is gamma-encoded, so it was written at 8 bits per channel."));
	}
	else if (bAnyEncodedSource && !Request.bSixteenBit)
	{
		OutResult.Warnings.Add(LOCTEXT("Pack_PrecisionLoss",
			"A source is gamma-encoded, and linearizing it spreads its dark values wider than 8 bits hold. "
			"Consider writing this one at 16 bits."));
	}

	FImage Final;
	Packed.CopyTo(Final, Resolved.Format, Resolved.Gamma);

	FText WriteError;
	UTexture2D* Written = FNTexturePackerUtils::CreateOrUpdateTexture(Request.PackagePath, Request.AssetName,
		Final, Request.bSRGB, Request.Compression, Request.Slots[0].Texture, Request.bCheckOut, WriteError);

	if (Written == nullptr)
	{
		OutResult.Error = WriteError;
		return;
	}

	OutResult.Textures.Add(Written);
	OutResult.bSucceeded = true;
}

void FNTexturePackerJob::Unpack(const FNTextureUnpackRequest& Request, FNTexturePackerResult& OutResult)
{
	using namespace NEXUS::ToolingEditor::TexturePacker::Job;

	OutResult = FNTexturePackerResult();

	if (Request.Texture == nullptr)
	{
		OutResult.Error = LOCTEXT("Unpack_NoTexture", "Select the texture to unpack.");
		return;
	}

	FImage Packed;
	FText ReadError;
	if (!FNTexturePackerUtils::ReadTextureLinear(Request.Texture, Packed, Request.bTreatSourceAsLinear, ReadError))
	{
		OutResult.Error = ReadError;
		return;
	}

	TArray<const FNTextureUnpackOutput*> Enabled;
	for (const FNTextureUnpackOutput& Output : Request.Outputs)
	{
		if (Output.bEnabled)
		{
			Enabled.Add(&Output);
		}
	}

	if (Enabled.Num() == 0)
	{
		OutResult.Error = LOCTEXT("Unpack_NoOutputs", "Switch on at least one channel to unpack.");
		return;
	}

	// Every name is checked before anything is written, so a run that would collide part-way through does not
	// leave half its textures behind.
	for (const FNTextureUnpackOutput* Output : Enabled)
	{
		if (Output->AssetName.IsEmpty())
		{
			OutResult.Error = LOCTEXT("Unpack_NoName", "Every channel being unpacked needs an asset name.");
			return;
		}

		if (!Request.bOverwriteExisting &&
			FNTexturePackerUtils::DoesAssetExist(Request.PackagePath, Output->AssetName))
		{
			OutResult.Error = FText::Format(LOCTEXT("Unpack_Exists",
				"'{0}' already exists. Rename it, or allow it to be overwritten."),
				FText::FromString(Output->AssetName));
			return;
		}
	}

	const TArrayView64<const FLinearColor> Source = Packed.AsRGBA32F();

	for (const FNTextureUnpackOutput* Output : Enabled)
	{
		const int32 First = FMath::Clamp(static_cast<int32>(Output->FirstChannel), 0, 3);
		const int32 Length = FMath::Clamp(Output->Length, 1, 4 - First);

		// Unwritten channels are black with an opaque alpha, which is what a two-channel normal wants and
		// what everything else is indifferent to.
		FImage Working(Packed.SizeX, Packed.SizeY, 1, ERawImageFormat::RGBA32F, EGammaSpace::Linear);
		const TArrayView64<FLinearColor> Destination = Working.AsRGBA32F();

		for (int64 Pixel = 0; Pixel < Destination.Num(); Pixel++)
		{
			FLinearColor Unpacked(0.0f, 0.0f, 0.0f, 1.0f);
			for (int32 Offset = 0; Offset < Length; Offset++)
			{
				Unpacked.Component(Offset) = Source[Pixel].Component(First + Offset);
			}
			Destination[Pixel] = Unpacked;
		}

		const FResolvedFormat Resolved = ResolveOutputFormat(Request.bSixteenBit, Output->bSRGB, Length);
		if (Resolved.bForcedEightBit)
		{
			OutResult.Warnings.Add(FText::Format(LOCTEXT("Unpack_ForcedEightBit",
				"'{0}' is gamma-encoded, so it was written at 8 bits per channel."),
				FText::FromString(Output->AssetName)));
		}

		FImage Final;
		Working.CopyTo(Final, Resolved.Format, Resolved.Gamma);

		FText WriteError;
		UTexture2D* Written = FNTexturePackerUtils::CreateOrUpdateTexture(Request.PackagePath,
			Output->AssetName, Final, Output->bSRGB, Output->Compression, Request.Texture,
			Request.bCheckOut, WriteError);

		if (Written == nullptr)
		{
			OutResult.Error = WriteError;
			return;
		}

		OutResult.Textures.Add(Written);
	}

	OutResult.bSucceeded = true;
}

bool FNTexturePackerJob::BuildAutoPackRequest(const UNTexturePackerSettings& Settings,
	const TArray<UTexture2D*>& Textures, FNTexturePackRequest& OutRequest, FNTexturePackerResult& OutResult)
{
	OutRequest = FNTexturePackRequest();
	OutRequest.Slots.SetNum(4);

	TArray<UTexture2D*> Valid;
	for (UTexture2D* Texture : Textures)
	{
		if (Texture != nullptr)
		{
			Valid.Add(Texture);
		}
	}

	if (Valid.Num() == 0 || Valid.Num() > 4)
	{
		OutResult.Error = LOCTEXT("Auto_Count", "Packing takes between one and four textures.");
		return false;
	}

	// Role first, name second: what a texture holds decides which channel it lands in, and the names only
	// decide what the result is called.
	TMap<FName, UTexture2D*> ByRole;
	TArray<FString> BaseNames;
	for (UTexture2D* Texture : Valid)
	{
		const FNTextureNameParts Parts = FNTexturePackerUtils::SplitAssetName(Settings, Texture->GetName());
		BaseNames.Add(Parts.BaseName);

		const FName Role = FNTexturePackerUtils::InferRole(Settings, Texture->GetName());
		if (Role.IsNone()) continue;

		if (ByRole.Contains(Role))
		{
			OutResult.Warnings.Add(FText::Format(LOCTEXT("Auto_DuplicateRole",
				"'{0}' holds the same map as a texture already assigned, and was skipped."),
				FText::FromString(Texture->GetName())));
			continue;
		}

		ByRole.Add(Role, Texture);
	}

	const FString BaseName = FNTexturePackerUtils::CommonBaseName(BaseNames);
	OutRequest.PackagePath = FNTexturePackerUtils::GetPackagePath(Valid[0]);
	OutRequest.bSixteenBit = Settings.bSixteenBitOutput;
	OutRequest.bCheckOut = Settings.bCheckOutAssets;

	// A layout is usable where it covers every texture given and asks for nothing that was not.
	const FNTexturePackedLayout* Chosen = nullptr;
	if (ByRole.Num() == Valid.Num())
	{
		for (const FNTexturePackedLayout& Layout : Settings.Layouts)
		{
			TSet<FName> Wanted;
			for (const FName& Role : Layout.GetRoles())
			{
				if (!Role.IsNone())
				{
					Wanted.Add(Role);
				}
			}

			if (Wanted.Num() != ByRole.Num()) continue;

			bool bCovers = true;
			for (const FName& Role : Wanted)
			{
				if (!ByRole.Contains(Role))
				{
					bCovers = false;
					break;
				}
			}

			if (bCovers)
			{
				Chosen = &Layout;
				break;
			}
		}
	}

	TArray<FName> AssignedRoles;
	AssignedRoles.Init(NAME_None, 4);

	if (Chosen != nullptr)
	{
		AssignedRoles = Chosen->GetRoles();
		OutRequest.Compression = Chosen->Compression;
		OutRequest.AssetName = FNTexturePackerUtils::ComposeAssetName(BaseName, Chosen->Suffix);
	}
	else
	{
		// Nothing recognized the arrangement, so the order the textures arrived in is the only intent there
		// is to honor. The suffix is composed from whatever roles were identified.
		for (int32 i = 0; i < Valid.Num(); i++)
		{
			FName Role = FNTexturePackerUtils::InferRole(Settings, Valid[i]->GetName());
			if (Role.IsNone())
			{
				// Nothing named it, so the asset stands for itself. Unique per texture, which is all the
				// run grouping below asks of a role identifier.
				Role = FName(*Valid[i]->GetName());
			}

			AssignedRoles[i] = Role;
			ByRole.Add(Role, Valid[i]);
		}

		OutRequest.Compression = TC_Masks;
		FString Suffix = FNTexturePackerUtils::ComposeSuffixForRoles(Settings, AssignedRoles);
		if (Suffix.IsEmpty())
		{
			Suffix = TEXT("Packed");
		}
		OutRequest.AssetName = FNTexturePackerUtils::ComposeAssetName(BaseName, Suffix);

		OutResult.Warnings.Add(LOCTEXT("Auto_NoLayout",
			"No known layout covers these maps, so they were assigned in the order given and the name was "
			"composed from what each one holds."));
	}

	const TArray<FNTextureChannelRun> Runs = FNTexturePackerUtils::BuildChannelRuns(AssignedRoles);
	for (const FNTextureChannelRun& Run : Runs)
	{
		UTexture2D** Found = ByRole.Find(Run.RoleId);
		if (Found == nullptr) continue;

		for (int32 Offset = 0; Offset < Run.Length; Offset++)
		{
			const int32 ChannelIndex = static_cast<int32>(Run.FirstChannel) + Offset;
			FNTexturePackSlot& Slot = OutRequest.Slots[ChannelIndex];
			Slot.Texture = *Found;
			// A run of one is a mask, and a mask keeps its data in red however many channels it was stored
			// in. A run that spans channels is a color or normal map, and lines up channel for channel.
			Slot.Source = Run.Length > 1 ? ENTextureChannelSource::Auto : ENTextureChannelSource::Red;
		}
	}

	if (OutRequest.AssetName.IsEmpty())
	{
		OutResult.Error = LOCTEXT("Auto_NoName", "Could not work out a name for the packed texture.");
		return false;
	}

	return true;
}

bool FNTexturePackerJob::BuildAutoUnpackRequest(const UNTexturePackerSettings& Settings, UTexture2D* Texture,
	FNTextureUnpackRequest& OutRequest, FNTexturePackerResult& OutResult)
{
	OutRequest = FNTextureUnpackRequest();

	if (Texture == nullptr)
	{
		OutResult.Error = LOCTEXT("AutoUnpack_NoTexture", "Select the texture to unpack.");
		return false;
	}

	OutRequest.Texture = Texture;
	OutRequest.PackagePath = FNTexturePackerUtils::GetPackagePath(Texture);
	OutRequest.bSixteenBit = Settings.bSixteenBitOutput;
	OutRequest.bCheckOut = Settings.bCheckOutAssets;

	FNTexturePackedLayout Layout;
	FString BaseName;
	if (!FNTexturePackerUtils::DetectLayout(Settings, Texture->GetName(), Layout, BaseName))
	{
		OutResult.Error = FText::Format(LOCTEXT("AutoUnpack_Unknown",
			"Nothing in '{0}' says what it holds. Open it in the Texture Packer and pick a layout."),
			FText::FromString(Texture->GetName()));
		return false;
	}

	for (const FNTextureChannelRun& Run : FNTexturePackerUtils::BuildChannelRuns(Layout.GetRoles()))
	{
		FNTextureUnpackOutput Output;
		Output.RoleId = Run.RoleId;
		Output.FirstChannel = Run.FirstChannel;
		Output.Length = Run.Length;
		ApplyRoleToOutput(Settings, BaseName, Output);
		OutRequest.Outputs.Add(Output);
	}

	if (OutRequest.Outputs.Num() == 0)
	{
		OutResult.Error = FText::Format(LOCTEXT("AutoUnpack_Empty",
			"'{0}' reads as a layout with no channels in it."), FText::FromString(Texture->GetName()));
		return false;
	}

	// A channel holding one value throughout carries nothing, and the unused alpha of an ORM texture is the
	// usual case. Reading the source to find out costs one decode, which is worth not writing dead assets for.
	if (Settings.bSkipUniformChannels)
	{
		FImage Packed;
		FText ReadError;
		if (FNTexturePackerUtils::ReadTextureLinear(Texture, Packed, false, ReadError))
		{
			for (FNTextureUnpackOutput& Output : OutRequest.Outputs)
			{
				bool bUniform = true;
				for (int32 Offset = 0; Offset < Output.Length && bUniform; Offset++)
				{
					const ENTextureChannel Channel =
						static_cast<ENTextureChannel>(static_cast<int32>(Output.FirstChannel) + Offset);
					bUniform = FNTexturePackerUtils::IsChannelUniform(Packed, Channel);
				}

				if (bUniform)
				{
					Output.bEnabled = false;
					OutResult.Warnings.Add(FText::Format(LOCTEXT("AutoUnpack_Uniform",
						"'{0}' holds one value throughout and was left switched off."),
						FText::FromString(Output.AssetName)));
				}
			}
		}
	}

	return true;
}

void FNTexturePackerJob::ApplyRoleToOutput(const UNTexturePackerSettings& Settings, const FString& BaseName,
	FNTextureUnpackOutput& Output)
{
	const FNTextureChannelRole* Role = Settings.FindRole(Output.RoleId);

	if (Role == nullptr)
	{
		// An unrecognized role still needs a name, and the channels it covers are the only thing left to
		// describe it by.
		static const TCHAR* ChannelLetters[] = { TEXT("R"), TEXT("G"), TEXT("B"), TEXT("A") };
		FString Letters;
		for (int32 Offset = 0; Offset < Output.Length; Offset++)
		{
			const int32 Index = FMath::Clamp(static_cast<int32>(Output.FirstChannel) + Offset, 0, 3);
			Letters += ChannelLetters[Index];
		}

		Output.AssetName = FNTexturePackerUtils::ComposeAssetName(BaseName, Letters);
		Output.bSRGB = false;
		Output.Compression = Output.Length > 1 ? TC_Masks : TC_Alpha;
		return;
	}

	Output.AssetName = FNTexturePackerUtils::ComposeAssetName(BaseName, Role->GetCanonicalSuffix());
	Output.bSRGB = Role->bOutputSRGB;
	Output.Compression = Output.Length > 1 ? Role->MultiChannelCompression : Role->SingleChannelCompression;
}

#undef LOCTEXT_NAMESPACE
