// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

/**
 * Hit-proxy that represents an edge — a pair of indexed endpoints — rendered by a component visualizer.
 *
 * The StartIndex / EndIndex pair lets the owning FComponentVisualizer identify which edge was picked.
 */
struct NEXUSCOREEDITOR_API HNEdgeComponentVisProxy final : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HNEdgeComponentVisProxy(const UActorComponent* InComponent, const int32 InStartIndex, const int32 InEndIndex, const EMouseCursor::Type InCursor = EMouseCursor::ResizeLeftRight)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
		, Cursor(InCursor), StartIndex(InStartIndex), EndIndex(InEndIndex)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return Cursor;
	}

	/** Mouse cursor shown when the edge is hovered. */
	EMouseCursor::Type Cursor;

	/** Index of the edge's first endpoint within the owning component. */
	int32 StartIndex;

	/** Index of the edge's second endpoint within the owning component. */
	int32 EndIndex;
};