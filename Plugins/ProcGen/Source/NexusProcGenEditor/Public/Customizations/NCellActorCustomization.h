// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FNCellActorCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};