// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Framework/Application/IInputProcessor.h"

class NEXUSCOREEDITOR_API FNEditorInputProcessor : public IInputProcessor
{
public:
	FNEditorInputProcessor() = default;
	virtual ~FNEditorInputProcessor() override = default;
	
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { };
	virtual const TCHAR* GetDebugName() const override { return TEXT("FNEditorInputProcessor"); }

	bool IsLeftShiftDown() const;
	bool IsRightShiftDown() const;
	bool IsShiftDown() const;
	bool IsLeftControlDown() const;
	bool IsRightControlDown() const;
	bool IsControlDown() const;
	bool IsSpaceBarDown() const;
	bool IsLeftMouseButtonDown() const;
	bool IsRightMouseButtonDown() const;
	bool IsMiddleMouseButtonDown() const;
	bool IsAnyMouseButtonDown() const;

	bool bCachedPanSetting;
	

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
