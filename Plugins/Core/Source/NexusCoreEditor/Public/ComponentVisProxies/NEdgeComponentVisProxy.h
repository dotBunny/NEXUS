// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"

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

	EMouseCursor::Type Cursor;
	int32 StartIndex;
	int32 EndIndex;
};

/*	ResizeUpDown,
ResizeSouthEast,

ResizeSouthWest,*/