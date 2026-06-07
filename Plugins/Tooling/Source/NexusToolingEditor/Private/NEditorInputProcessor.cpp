// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorInputProcessor.h"
#include "NEditorUtils.h"

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

bool FNEditorInputProcessor::IsMouseEventLastSyntheticEvent(const FPointerEvent& MouseEvent)
{
	// Quick check for screenspace position as its cheap
	if (MouseEvent.GetScreenSpacePosition() != LastSyntheticEvent.GetScreenSpacePosition())
	{
		return false;
	}
	
	const TSet<FKey>& MouseEventButtons = MouseEvent.GetPressedButtons();
	const TSet<FKey>& LastSyntheticEventButtons = LastSyntheticEvent.GetPressedButtons();
	
	// Quick check the counts
	if (MouseEventButtons.Num() != LastSyntheticEventButtons.Num())
	{
		return false;
	}
	
	// Loop through and check that the keys are identical
	for (const FKey& Button : MouseEventButtons)
	{
		if (!LastSyntheticEventButtons.Contains(Button))
		{
			return false;
		}
	}

	return true;
}
bool FNEditorInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (bCachedGraphNavigationSpaceToPan && bSpaceBar && bLeftMouse && 
		!IsMouseEventLastSyntheticEvent(MouseEvent)) // Dont resend the last synthetic event
	{
		IAssetEditorInstance* ForegroundAssetEditor = FNEditorUtils::GetForegroundAssetEditor();
		if (ForegroundAssetEditor == nullptr) return false;
		
		// We're going to make a synthetic mouse event from the current event
		TSet<FKey> PressedButtons = MouseEvent.GetPressedButtons();
		PressedButtons.Remove(EKeys::LeftMouseButton);
		PressedButtons.Add(EKeys::RightMouseButton);
		
		// Build a synthetic event
     	LastSyntheticEvent = FPointerEvent(
		FSlateApplicationBase::CursorPointerIndex, 
		MouseEvent.GetScreenSpacePosition(), 
		MouseEvent.GetLastScreenSpacePosition(), 
		PressedButtons, 
		EKeys::RightMouseButton, 
		MouseEvent.GetWheelDelta(), 
		MouseEvent.GetModifierKeys());
		
		// The event CANNOT be synthetic for the SNodePanel to capture it.
		SlateApp.ProcessMouseMoveEvent(LastSyntheticEvent, false);
 		return true;
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