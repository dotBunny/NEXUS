// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "TexturePacker/NTexturePackerJob.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NTexturePackerLibrary.generated.h"

class UTexture2D;

/**
 * The Texture Packer without its window: pack and unpack driven from a script, a commandlet or an editor
 * utility widget.
 *
 * The Auto entry points make the same choices the window proposes - roles read from asset names, channels
 * grouped by the layout that matches, outputs named and configured for what each one holds - so a batch run
 * over a folder produces what opening each texture in turn would have. Where a name says nothing the run
 * fails rather than guessing; the window is for that case.
 *
 * @note Generated assets are left dirty rather than saved, exactly as the window leaves them.
 * @see <a href="https://nexus-framework.com/docs/tooling/texture-packer/">Texture Packer</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Texture Packer Library")
class NEXUSTOOLINGEDITOR_API UNTexturePackerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Packs up to four textures into one, working out the arrangement and the name from what they are called.
	 * @param Textures The maps to pack, in the order they should fall where no layout recognizes them.
	 * @return What was produced, or a failure carrying why.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Pack Textures", Category = "NEXUS|Tooling",
		meta = (DocsURL = "https://nexus-framework.com/docs/tooling/texture-packer/"))
	static FNTexturePackerResult PackTextures(const TArray<UTexture2D*>& Textures);

	/**
	 * Splits a packed texture into one texture per map it holds, working out what it holds from its name.
	 * @param Texture The packed texture to take apart.
	 * @return What was produced, or a failure carrying why.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Unpack Texture", Category = "NEXUS|Tooling",
		meta = (DocsURL = "https://nexus-framework.com/docs/tooling/texture-packer/"))
	static FNTexturePackerResult UnpackTexture(UTexture2D* Texture);

	/**
	 * Unpacks every texture given, carrying on past the ones whose names say nothing.
	 * @param Textures The packed textures to take apart.
	 * @return The union of what every run produced; succeeds where at least one texture was unpacked.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Unpack Textures", Category = "NEXUS|Tooling",
		meta = (DocsURL = "https://nexus-framework.com/docs/tooling/texture-packer/"))
	static FNTexturePackerResult UnpackTextures(const TArray<UTexture2D*>& Textures);

	/** Packs exactly what Request describes, deciding nothing. */
	UFUNCTION(BlueprintCallable, DisplayName = "Pack Textures (Advanced)", Category = "NEXUS|Tooling",
		meta = (DocsURL = "https://nexus-framework.com/docs/tooling/texture-packer/"))
	static FNTexturePackerResult PackTexturesAdvanced(const FNTexturePackRequest& Request);

	/** Unpacks exactly what Request describes, deciding nothing. */
	UFUNCTION(BlueprintCallable, DisplayName = "Unpack Texture (Advanced)", Category = "NEXUS|Tooling",
		meta = (DocsURL = "https://nexus-framework.com/docs/tooling/texture-packer/"))
	static FNTexturePackerResult UnpackTextureAdvanced(const FNTextureUnpackRequest& Request);
};
