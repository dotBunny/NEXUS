// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorModeManager.h"
#include "EdMode.h"
#include "Cell/NCellActor.h"

class FNProcGenOperationContext;
class UNProcGenOperation;

class FNProcGenEdMode final : public FEdMode
{
public:
	enum class ENCellEdMode : uint8
	{
		Bounds = 0,
		Hull = 1,
		Voxel = 2
	};
	enum class ENCellVoxelMode : uint8
	{
		None = 0,
		Grid = 1,
		Points = 2
	};

	
	static FBox GetCachedBounds() { return CachedBounds; }	
	static const FLinearColor& GetCachedBoundsColor() { return CachedBoundsColor; }
	static const TArray<FVector>& GetCachedBoundsVertices() { return CachedBoundsVertices; }
	static const FLinearColor& GetCachedHullColor() { return CachedHullColor; }
	static const FNCellVoxelData& GetCachedVoxelData() { return CachedVoxelData; }
	static const TArray<FVector>& GetCachedHullVertices() { return CachedHullVertices; }
	static ANCellActor* GetCellActor() { return CellActor; }
	static ENCellEdMode GetCellEdMode() { return CellEdMode; }
	static void ProtectCellEdMode();
	
	
	static ENCellVoxelMode GetCellVoxelMode() { return CellVoxelMode; }
	static void SetCellVoxelMode(const ENCellVoxelMode InCellVoxelMode) { CellVoxelMode = InCellVoxelMode; }
	
	static bool HasCellActor() { return CellActor != nullptr; }
	static bool IsActive() { return GLevelEditorModeTools().IsModeActive(Identifier); }
	static void SetCellEdMode(const ENCellEdMode InCellEdMode) { CellEdMode = InCellEdMode; }
	
	const static FEditorModeID Identifier;
	const static FText DirtyMessage;
	const static FText AutoBoundsHullMessage;
	const static FText AutoBoundsMessage;
	const static FText AutoHullMessage;
	const static FText AutoVoxelMessage;

	virtual ~FNProcGenEdMode() override;
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
	static FNCellVoxelData CachedVoxelData;
	static FLinearColor CachedBoundsColor;
	static TArray<FVector> CachedBoundsVertices;
	static ANCellActor* CellActor;
	static ENCellEdMode CellEdMode;
	static ENCellVoxelMode CellVoxelMode;
	
	TObjectPtr<UNProcGenOperation> OrganGenerator;
	bool bCanTick = false;
	bool bHasDirtyActors = false;
	bool bAutoBoundsDisabled = false;
	bool bAutoHullDisabled = false;
	bool bAutoVoxelDisabled = false;
	
	uint32 PreviousSelectedOrganHash = 0;
};
