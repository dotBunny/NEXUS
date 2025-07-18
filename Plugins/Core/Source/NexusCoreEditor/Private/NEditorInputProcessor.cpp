﻿#include "NEditorInputProcessor.h"

#include "BlueprintEditor.h"
#include "NEditorUserSettings.h"
#include "WidgetBlueprintEditorUtils.h"

bool FNEditorInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	const FKey TargetKey = InKeyEvent.GetKey();
	if (TargetKey == EKeys::SpaceBar)
	{
		bSpaceBar = true;
		return false;
	}
	if (TargetKey == EKeys::LeftShift)
	{
		bLeftShift = true;
		return false;
	}
	if (TargetKey == EKeys::LeftControl)
	{
		bLeftControl = true;
		return false;
	}
	if (TargetKey == EKeys::RightShift)
	{
		bRightShift = true;
		return false;
	}
	if (TargetKey == EKeys::RightControl)
	{
		bRightControl = true;
		return false;
	}
	return false;
}
bool FNEditorInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	const FKey TargetKey = InKeyEvent.GetKey();
	if (TargetKey == EKeys::SpaceBar)
	{
		bSpaceBar = false;
		return false;
	}
	if (TargetKey == EKeys::LeftShift)
	{
		bLeftShift = false;
		return false;
	}
	if (TargetKey == EKeys::LeftControl)
	{
		bLeftControl = false;
		return false;
	}
	if (TargetKey == EKeys::RightShift)
	{
		bRightShift = false;
		return false;
	}
	if (TargetKey == EKeys::RightControl)
	{
		bRightControl = false;
		return false;
	}
	return false;
}

bool FNEditorInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (bCachedPanSetting && bSpaceBar && bLeftMouse)
	{
		if (FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor(); Editor != nullptr)
		{
			
			// Move The Graph
			FVector2D ViewLocation;
			float ViewZoom;
			Editor->GetViewLocation(ViewLocation, ViewZoom);
  			ViewLocation += -MouseEvent.GetCursorDelta();
			Editor->SetViewLocation(ViewLocation, ViewZoom);
			return true;
		}

		// TODO? Designer?
	}
	return false;
}

bool FNEditorInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	const FKey TargetKey = MouseEvent.GetEffectingButton();
	if (TargetKey == EKeys::LeftMouseButton)
	{
		bLeftMouse = true;
		return false;
	}
	if (TargetKey == EKeys::RightMouseButton)
	{
		bRightMouse = true;
		return false;
	}
	if (TargetKey == EKeys::MiddleMouseButton)
	{
		bMiddleMouse = true;
		return false;
	}
	return false;
}

bool FNEditorInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	const FKey TargetKey = MouseEvent.GetEffectingButton();
	if (TargetKey == EKeys::LeftMouseButton)
	{
		bLeftMouse = false;
		return false;
	}
	if (TargetKey == EKeys::RightMouseButton)
	{
		bRightMouse = false;
		return false;
	}
	if (TargetKey == EKeys::MiddleMouseButton)
	{
		bMiddleMouse = false;
		return false;
	}
	return false;
}

bool FNEditorInputProcessor::IsLeftShiftDown() const
{
	return bLeftShift;
}

bool FNEditorInputProcessor::IsRightShiftDown() const
{
	return bRightShift;
}

bool FNEditorInputProcessor::IsShiftDown() const
{
	return bLeftShift || bRightShift;
}

bool FNEditorInputProcessor::IsLeftControlDown() const
{
	return bLeftControl;
}

bool FNEditorInputProcessor::IsRightControlDown() const
{
	return bRightControl;
}

bool FNEditorInputProcessor::IsControlDown() const
{
	return bLeftControl || bRightControl;
}

bool FNEditorInputProcessor::IsSpaceBarDown() const
{
	return bSpaceBar;
}

bool FNEditorInputProcessor::IsLeftMouseButtonDown() const
{
	return bLeftMouse;
}

bool FNEditorInputProcessor::IsRightMouseButtonDown() const
{
	return bRightMouse;
}

bool FNEditorInputProcessor::IsMiddleMouseButtonDown() const
{
	return bMiddleMouse;
}

bool FNEditorInputProcessor::IsAnyMouseButtonDown() const
{
	return bLeftMouse || bRightMouse || bMiddleMouse;
}

