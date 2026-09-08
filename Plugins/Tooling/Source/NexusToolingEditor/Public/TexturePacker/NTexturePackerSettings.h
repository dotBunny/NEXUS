// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "TexturePacker/NTexturePackerTypes.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NTexturePackerSettings.generated.h"

/**
 * The naming conventions the Texture Packer reads and writes, plus the defaults its two modes start on.
 *
 * Everything the tool knows about what a suffix means lives here rather than in code, because the answer is
 * per-project: the roles table maps filename suffixes onto semantic map types, and the layouts table maps a
 * suffix onto the arrangement of roles a packed texture holds. A project with its own vocabulary retunes
 * both without touching C++.
 *
 * @note Order matters in Layouts. Several conventions describe the same arrangement under different names
 *       (ORM and ARM are both occlusion/roughness/metallic), so a lookup by channel roles returns the first
 *       row that matches - put the spelling the project prefers above its synonyms. Lookups by suffix are
 *       unambiguous and unaffected.
 *
 * @see <a href="https://nexus-framework.com/docs/tooling/texture-packer/">Texture Packer</a>
 */
UCLASS(config = NexusEditor, defaultconfig)
class NEXUSTOOLINGEDITOR_API UNTexturePackerSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_EDITOR_SETTINGS_BASE(UNTexturePackerSettings, "Texture Packer",
		"Naming conventions and defaults used by the NEXUS: Texture Packer window.");

	UNTexturePackerSettings();

#if WITH_EDITORONLY_DATA

	/**
	 * Every semantic map type the tool can recognize in an asset name, and the texture settings each implies.
	 * The first suffix on a role is the one written onto assets the tool generates.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Conventions", DisplayName = "Channel Roles",
		meta = (ToolTip = "Maps filename suffixes onto semantic map types. The first suffix listed for a role is the one written onto generated assets.", TitleProperty = "Id"))
	TArray<FNTextureChannelRole> Roles;

	/**
	 * Known packed-texture arrangements. A role repeated across adjacent channels spans them, which is how a
	 * color map packed across RGB unpacks as one RGB asset rather than three grayscales.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Conventions", DisplayName = "Packed Layouts",
		meta = (ToolTip = "Maps a filename suffix onto the arrangement of roles a packed texture holds. Where two rows describe the same arrangement, the first is the one the tool proposes.", TitleProperty = "Suffix"))
	TArray<FNTexturePackedLayout> Layouts;

	/**
	 * Written at 16 bits per channel rather than 8 by default.
	 *
	 * 8 bits is right for almost everything; the case for 16 is a channel whose source was gamma-encoded,
	 * since linearizing it on the way in spreads its dark values across a range 8 bits cannot hold. The tool
	 * detects that and says so rather than deciding for you.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Defaults", DisplayName = "Sixteen Bit Output",
		meta = (ToolTip = "Write generated textures at 16 bits per channel instead of 8."))
	bool bSixteenBitOutput = false;

	/** Check generated assets out of revision control (adding new ones) rather than failing on a read-only file. */
	UPROPERTY(EditAnywhere, config, Category = "Defaults", DisplayName = "Check Out Assets",
		meta = (ToolTip = "Check generated assets out of revision control, adding files that are not yet under it."))
	bool bCheckOutAssets = true;

	/** Select what a run produced in the Content Browser once it finishes. */
	UPROPERTY(EditAnywhere, config, Category = "Defaults", DisplayName = "Select Results",
		meta = (ToolTip = "Select the generated textures in the Content Browser when a pack or unpack finishes."))
	bool bSelectResults = true;

	/**
	 * Skip a channel whose every pixel is the same value when unpacking.
	 *
	 * The unused alpha of an ORM texture is the case this exists for - it carries no information, and writing
	 * it out produces an asset nothing will ever sample.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Defaults", DisplayName = "Skip Uniform Channels",
		meta = (ToolTip = "When unpacking, leave channels whose pixels are all the same value switched off."))
	bool bSkipUniformChannels = true;

	/** @return The role registered under Id, or nullptr where nothing matches. */
	const FNTextureChannelRole* FindRole(const FName Id) const;

	/** @return The first role listing Suffix (matched case-insensitively, without a leading underscore), or nullptr. */
	const FNTextureChannelRole* FindRoleBySuffix(const FString& Suffix) const;

	/** @return The layout named by Suffix (matched case-insensitively, without a leading underscore), or nullptr. */
	const FNTexturePackedLayout* FindLayoutBySuffix(const FString& Suffix) const;

	/**
	 * @return The first layout whose four channel roles are exactly Roles, or nullptr.
	 * @note First, not only - see the ordering note on this class.
	 */
	const FNTexturePackedLayout* FindLayoutByRoles(const TArray<FName>& InRoles) const;

	/** @return Every suffix in the roles and layouts tables, longest first, for greedy suffix matching. */
	void GetAllSuffixes(TArray<FString>& OutSuffixes) const;

#endif // WITH_EDITORONLY_DATA
};
