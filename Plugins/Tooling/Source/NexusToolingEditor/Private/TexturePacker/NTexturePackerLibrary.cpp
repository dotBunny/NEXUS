// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "TexturePacker/NTexturePackerLibrary.h"

#include "NToolingEditorMinimal.h"
#include "Engine/Texture2D.h"
#include "TexturePacker/NTexturePackerSettings.h"
#include "TexturePacker/NTexturePackerUtils.h"

#define LOCTEXT_NAMESPACE "NTexturePacker"

namespace NEXUS::ToolingEditor::TexturePacker::Library
{
	/** Reports a finished run to the log, so a batch driven from a script leaves a trail. */
	void LogResult(const TCHAR* Operation, const FNTexturePackerResult& Result)
	{
		for (const FText& Warning : Result.Warnings)
		{
			UE_LOG(LogNexusToolingEditor, Warning, TEXT("Texture Packer (%s): %s"), Operation, *Warning.ToString());
		}

		if (Result.bSucceeded)
		{
			UE_LOG(LogNexusToolingEditor, Log, TEXT("Texture Packer (%s): wrote %d texture(s)."),
				Operation, Result.Textures.Num());
			return;
		}

		UE_LOG(LogNexusToolingEditor, Error, TEXT("Texture Packer (%s): %s"), Operation, *Result.Error.ToString());
	}
}

FNTexturePackerResult UNTexturePackerLibrary::PackTextures(const TArray<UTexture2D*>& Textures)
{
	FNTexturePackerResult Result;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FNTexturePackRequest Request;
	if (FNTexturePackerJob::BuildAutoPackRequest(*Settings, Textures, Request, Result))
	{
		// The warnings the proposal raised are kept: Pack starts from a clean result, and a note about a
		// duplicate map is the sort of thing worth keeping hold of.
		const TArray<FText> ProposalWarnings = Result.Warnings;
		FNTexturePackerJob::Pack(Request, Result);
		Result.Warnings.Insert(ProposalWarnings, 0);
	}

	NEXUS::ToolingEditor::TexturePacker::Library::LogResult(TEXT("Pack"), Result);
	return Result;
}

FNTexturePackerResult UNTexturePackerLibrary::UnpackTexture(UTexture2D* Texture)
{
	FNTexturePackerResult Result;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	FNTextureUnpackRequest Request;
	if (FNTexturePackerJob::BuildAutoUnpackRequest(*Settings, Texture, Request, Result))
	{
		const TArray<FText> ProposalWarnings = Result.Warnings;
		FNTexturePackerJob::Unpack(Request, Result);
		Result.Warnings.Insert(ProposalWarnings, 0);
	}

	NEXUS::ToolingEditor::TexturePacker::Library::LogResult(TEXT("Unpack"), Result);
	return Result;
}

FNTexturePackerResult UNTexturePackerLibrary::UnpackTextures(const TArray<UTexture2D*>& Textures)
{
	FNTexturePackerResult Combined;

	const UNTexturePackerSettings* Settings = UNTexturePackerSettings::Get();

	for (UTexture2D* Texture : Textures)
	{
		if (Texture == nullptr) continue;

		FNTexturePackerResult Result;
		FNTextureUnpackRequest Request;
		if (FNTexturePackerJob::BuildAutoUnpackRequest(*Settings, Texture, Request, Result))
		{
			const TArray<FText> ProposalWarnings = Result.Warnings;
			FNTexturePackerJob::Unpack(Request, Result);
			Result.Warnings.Insert(ProposalWarnings, 0);
		}

		Combined.Warnings.Append(Result.Warnings);

		if (Result.bSucceeded)
		{
			Combined.Textures.Append(Result.Textures);
			Combined.bSucceeded = true;
			continue;
		}

		// One texture nobody can read is not a reason to abandon the rest of a folder, so a failure is
		// demoted to a warning and the batch carries on. The run only fails outright where none succeeded.
		Combined.Warnings.Add(FText::Format(LOCTEXT("Batch_Skipped", "'{0}' was skipped: {1}"),
			FText::FromString(Texture->GetName()), Result.Error));
		Combined.Error = Result.Error;
	}

	if (Combined.bSucceeded)
	{
		Combined.Error = FText::GetEmpty();
	}
	else if (Combined.Error.IsEmpty())
	{
		Combined.Error = LOCTEXT("Batch_Nothing", "No textures were given to unpack.");
	}

	NEXUS::ToolingEditor::TexturePacker::Library::LogResult(TEXT("Unpack"), Combined);
	return Combined;
}

FNTexturePackerResult UNTexturePackerLibrary::PackTexturesAdvanced(const FNTexturePackRequest& Request)
{
	FNTexturePackerResult Result;
	FNTexturePackerJob::Pack(Request, Result);
	NEXUS::ToolingEditor::TexturePacker::Library::LogResult(TEXT("Pack"), Result);
	return Result;
}

FNTexturePackerResult UNTexturePackerLibrary::UnpackTextureAdvanced(const FNTextureUnpackRequest& Request)
{
	FNTexturePackerResult Result;
	FNTexturePackerJob::Unpack(Request, Result);
	NEXUS::ToolingEditor::TexturePacker::Library::LogResult(TEXT("Unpack"), Result);
	return Result;
}

#undef LOCTEXT_NAMESPACE
