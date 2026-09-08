// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Templates/SharedPointer.h"

class FSpawnTabArgs;
class SDockTab;
class SNTexturePacker;
class UTexture2D;

/**
 * The Texture Packer window's tab, and its entry in the NEXUS Tools menu.
 *
 * A nomad tab rather than a window of its own, so it docks where it is put and comes back there the next
 * time the editor starts.
 */
class NEXUSTOOLINGEDITOR_API FNTexturePackerTab
{
public:
	/** Registers the tab spawner and the Tools menu entry that opens it. */
	static void Register();

	/** Removes everything Register installed. */
	static void Unregister();

	/** Brings the window up, focusing it where it is already open. */
	static void Open();

	/** Brings the window up on its unpack half with Texture loaded. */
	static void OpenForUnpack(UTexture2D* Texture);

	/** Brings the window up on its pack half with Textures assigned to channels. */
	static void OpenForPack(const TArray<UTexture2D*>& Textures);

	/** Identifier the tab is registered and looked up under. */
	static const FName TabId;

private:
	/** Builds the tab's contents. */
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);

	/**
	 * The widget inside the open tab, so the Content Browser actions can push a selection into it.
	 * @note Weak: the tab owns the widget, and closing the tab has to be able to destroy it.
	 */
	static TWeakPtr<SNTexturePacker> ActiveWidget;
};
