// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

/**
 * Detail-panel customization for editor-spawned cell proxies. Strips the cell proxy's details to
 * the minimal set an author should see/edit, hiding transient runtime state.
 */
class FNCellProxyCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization
};