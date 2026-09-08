// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class UToolMenu;

/**
 * The Texture Packer's entries on the Content Browser's texture context menu.
 *
 * Two of the four decide nothing: they read the selection's names, work out the arrangement and write the
 * result, which is what makes unpacking a folder of packed textures one action rather than one per texture.
 * The other two open the window on the selection for the cases where a name says nothing useful.
 */
class FNTexturePackerAssetActions
{
public:
	/** Installs the entries on the texture asset context menu. */
	static void Register();

	/** Removes everything Register installed. */
	static void Unregister();

private:
	/** Populates the Texture Packer submenu, which is where the selection is finally available. */
	static void BuildSubMenu(UToolMenu* Menu);
};
