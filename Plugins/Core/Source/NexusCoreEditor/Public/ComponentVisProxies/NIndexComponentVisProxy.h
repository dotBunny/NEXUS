// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

/**
 * Hit-proxy that identifies an individual indexed element (e.g. a vertex) rendered by a
 * component visualizer.
 *
 * The Index value lets the owning FComponentVisualizer route picks back to a specific element.
 */
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

	/** Index of the element this proxy represents within its owning component. */
	int32 Index;
};