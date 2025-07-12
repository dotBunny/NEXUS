// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorModeManager.h"
#include "EdMode.h"
#include "Cell/NCellActor.h"

class FNProcGenEdMode final : public FEdMode
{
public:
	enum ENCellEdMode : uint8
	{
		NCell_Bounds = 0,
		NCell_Hull = 1
	};

	static FBox GetCachedBounds() { return CachedBounds; }	
	static const FLinearColor& GetCachedBoundsColor() { return CachedBoundsColor; }
	static const TArray<FVector>& GetCachedBoundsVertices() { return CachedBoundsVertices; }
	static const FLinearColor& GetCachedHullColor() { return CachedHullColor; }
	static const TArray<FVector>& GetCachedHullVertices() { return CachedHullVertices; }
	static ANCellActor* GetNCellActor() { return NCellActor; }
	static ENCellEdMode GetNCellEdMode() { return NCellEdMode; }
	static bool HasNCellActor() { return NCellActor != nullptr; }
	static bool IsActive() { return GLevelEditorModeTools().IsModeActive(Identifier); }
	static void SetCellEdMode(const ENCellEdMode InCellEdMode) { NCellEdMode = InCellEdMode; }
	
	const static FEditorModeID Identifier;
	const static FText DirtyMessage;
	const static FText AutoBoundsHullMessage;
	const static FText AutoBoundsMessage;
	const static FText AutoHullMessage;

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;

private:
	const int MessageSpacing = 20;
	static TArray<FVector> CachedHullVertices;
	static FLinearColor CachedHullColor;
	static FBox CachedBounds;
	static FLinearColor CachedBoundsColor;
	static TArray<FVector> CachedBoundsVertices;
	static ANCellActor* NCellActor;
	static ENCellEdMode NCellEdMode;
	bool bCanTick = false;
	bool bHasDirtyActors = false;
	bool bAutoBoundsDisabled = false;
	bool bAutoHullDisabled = false;
};
