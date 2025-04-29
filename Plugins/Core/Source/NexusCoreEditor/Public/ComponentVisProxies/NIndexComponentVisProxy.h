// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

struct NEXUSCOREEDITOR_API HNIndexComponentVisProxy final : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	
	HNIndexComponentVisProxy(const UActorComponent* InComponent, const int32 InVertexIndex)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
		, Index(InVertexIndex)
	{}
	
	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::Crosshairs;
	}
	
	int32 Index;
};