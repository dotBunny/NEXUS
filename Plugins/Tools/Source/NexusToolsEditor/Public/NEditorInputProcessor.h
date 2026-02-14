// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Framework/Application/IInputProcessor.h"

/**
 * An editor-focused IInputProcessor tracking the state of standard modifier keys and other defined keys.
 * @see <a href="https://nexus-framework.com/docs/plugins/tools/editor-types/input-processor/">FNEditorInputProcessor</a>
 */
class NEXUSTOOLSEDITOR_API FNEditorInputProcessor : public IInputProcessor
{
public:
	FNEditorInputProcessor() = default;
	virtual ~FNEditorInputProcessor() override = default;
	
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
		// We do nothing here
	};
	virtual const TCHAR* GetDebugName() const override { return TEXT("FNEditorInputProcessor"); }

	/**
	 * Is the left shift-key pressed?
	 * @return true/false the key is pressed on the keyboard.
	 */
	bool IsLeftShiftDown() const;

	/**
	 * Is the right shift-key pressed?
	 * @return true/false the key is pressed on the keyboard.
	 */
	bool IsRightShiftDown() const;

	/**
	 * Is either shift-key pressed?
	 * @return true/false either key is pressed on the keyboard.
	 */	
	bool IsShiftDown() const;

	/**
	 * Is the left control-key pressed?
	 * @return true/false the key is pressed on the keyboard.
	 */
	bool IsLeftControlDown() const;

	/**
	 * Is the right control-key pressed?
	 * @return true/false the key is pressed on the keyboard.
	 */	
	bool IsRightControlDown() const;

	/**
	 * Is either shift-key pressed?
	 * @return true/false either key is pressed on the keyboard.
	 */	
	bool IsControlDown() const;

	/**
	 * Is the space bar pressed?
	 * @return true/false the key is pressed on the keyboard.
	 */		
	bool IsSpaceBarDown() const;

	/**
	 * Is the left mouse-button pressed?
	 * @return true/false the mouse-button is pressed.
	 */	
	bool IsLeftMouseButtonDown() const;

	/**
	 * Is the right mouse-button pressed?
	 * @return true/false the mouse-button is pressed.
	 */		
	bool IsRightMouseButtonDown() const;

	/**
	 * Is the middle mouse-button pressed?
	 * @return true/false the mouse-button is pressed.
	 */	
	bool IsMiddleMouseButtonDown() const;

	/**
	 * Is any mouse-button pressed?
	 * @return true/false any of the three mouse-buttons are pressed.
	 */	
	bool IsAnyMouseButtonDown() const;

	bool bCachedGraphNavigationSpaceToPan;
	float CachedGraphNavigationPanSpeedMultiplier;

private:

	bool bLeftMouse = false;
	bool bRightMouse = false;
	bool bMiddleMouse = false;
	bool bLeftShift = false;
	bool bRightShift = false;
	bool bLeftControl = false;
	bool bRightControl = false;
	bool bSpaceBar = false;
};
