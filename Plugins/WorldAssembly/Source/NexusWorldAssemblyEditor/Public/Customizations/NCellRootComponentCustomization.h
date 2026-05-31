// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

/**
 * Detail-panel customization for UNCellRootComponent. Adds the cell-editor-specific buttons
 * (regenerate bounds/hull/voxel, save side-car) and routes edits back through the sub-mode's
 * auto-regeneration gates.
 */
class FNCellRootComponentCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization
};