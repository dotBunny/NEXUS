// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NCellRootComponentVisualizer.h"
#include "Cell/NCellRootComponent.h"
#include "NProcGenEdMode.h"
#include "NProcGenSettings.h"
#include "ComponentVisProxies/NEdgeComponentVisProxy.h"
#include "ComponentVisProxies/NIndexComponentVisProxy.h"

void FNCellRootComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// First we check that we indeed have an actor component
	const UNCellRootComponent* CellRootComponent = Cast<UNCellRootComponent>(const_cast<UActorComponent*>(Component));
	if (!CellRootComponent)
	{
		return;
	}

	// We need to draw the base wireframes
	if (!FNProcGenEdMode::IsActive())
	{
		CellRootComponent->DrawDebugPDI(PDI, FNProcGenEdMode::GetCellVoxelMode());
		return;
	}

	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Bounds)
	{
		const FBox Bounds = FNProcGenEdMode::GetCachedBounds();
		const TArray<FVector> BoundsVertices = FNProcGenEdMode::GetCachedBoundsVertices();
		
		// Draw Min Max
		PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, 0));
		PDI->DrawPoint(Bounds.Min, FNProcGenEdMode::GetCachedBoundsColor(), PointSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
		PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, 1));
		PDI->DrawPoint(Bounds.Max, FNProcGenEdMode::GetCachedBoundsColor(), PointSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);

	}
	else if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Hull)
	{
		const TArray<FVector> WorldVertices = FNProcGenEdMode::GetCachedHullVertices();
		const int VertCount = WorldVertices.Num();
		for (int i = 0; i < VertCount; i++)
		{
			PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, i));
			PDI->DrawPoint(WorldVertices[i], FNProcGenEdMode::GetCachedHullColor(), PointSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);
		}
	}
	else if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Voxel)
	{
		// Forcibly disable drawing of the voxel Mode
		if (FNProcGenEdMode::GetCellVoxelMode() != FNProcGenEdMode::ENCellVoxelMode::CVM_None)
		{
			FNProcGenEdMode::SetCellVoxelMode(FNProcGenEdMode::ENCellVoxelMode::CVM_None);
		}
		
		const FNCellVoxelData CachedData = FNProcGenEdMode::GetCachedVoxelData();
		if (!CachedData.IsValid())
		{
			return;
		}

		const size_t PointCount = CachedData.GetCount();
		const UNProcGenSettings* Settings = GetDefault<UNProcGenSettings>();
		const FVector UnitSize = Settings->UnitSize;
		const FVector HalfUnitSize = UnitSize * 0.5f;
		const FVector BaseOffset = CachedData.Origin;
		for (int i = 0; i < PointCount; i++)
		{
			auto [x,y,z] = CachedData.GetInverseIndex(i);
	
			// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
			FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
	
			if (N_FLAGS_HAS(CachedData.GetData(i), static_cast<uint8>(ENCellVoxel::CVD_Occupied)))
			{
				PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, i));
				PDI->DrawPoint(VoxelCenter, FColor::Blue, PointSize, SDPG_Foreground);
				PDI->SetHitProxy(nullptr);
			}
			else
			{
				PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, i));
				PDI->DrawPoint(VoxelCenter, FColor::Green, PointSize, SDPG_Foreground);
				PDI->SetHitProxy(nullptr);
			}
		}
	}
	
}

bool FNCellRootComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (Click.GetKey() == EKeys::LeftMouseButton && VisProxy && VisProxy->Component.IsValid())
	{
		if (VisProxy->IsA(HNIndexComponentVisProxy::StaticGetType()))
		{
			const auto IndexComponent = const_cast<UNCellRootComponent*>(Cast<UNCellRootComponent>(VisProxy->Component.Get()));
			const HNIndexComponentVisProxy* Proxy = static_cast<HNIndexComponentVisProxy*>(VisProxy);
			if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Bounds)
			{
				return EditBoundsVertex(IndexComponent, Proxy->Index);
			}
			if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Hull)
			{
				return EditHullVertex(IndexComponent, Proxy->Index);
			}
			if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::CEM_Voxel)
			{
				return ToggleVoxelPoint(IndexComponent, Proxy->Index);
			}
			return false;
		}
	}
	return false;
}

bool FNCellRootComponentVisualizer::EditHullVertex(UNCellRootComponent* Component, int32 Index)
{
	CurrentEditMode = EM_HullVertex;
	RootComponent = Component;
	VertexIndex = Index;
	return true;
}

bool FNCellRootComponentVisualizer::EditBoundsVertex(UNCellRootComponent* Component, int32 Index)
{
	CurrentEditMode = EM_BoundsVertex;
	RootComponent = Component;
	VertexIndex = Index;
	return true;
}

bool FNCellRootComponentVisualizer::ToggleVoxelPoint(UNCellRootComponent* Component, const int32 Index)
{
	uint8 Data = Component->Details.VoxelData.GetData(Index);
	
	// Handle Occupied
	if (N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::CVD_Occupied)))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusProcGenEditor", "FNCellRootComponentVisualizer_Voxel_Empty", "Set Voxel Empty"));
		Component->Modify();
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::CVD_Empty));
		Component->Details.VoxelData.SetData(Index, Data);
		Component->Details.VoxelSettings.bCalculateOnSave = false;
		Component->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	// Handle Empty
	if (N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::CVD_Empty)))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusProcGenEditor", "FNCellRootComponentVisualizer_Voxel_Occupied", "Set Voxel Occupied"));
		Component->Modify();
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::CVD_Empty));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
		Component->Details.VoxelData.SetData(Index, Data);
		Component->Details.VoxelSettings.bCalculateOnSave = false;
		Component->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	return true;
}

void FNCellRootComponentVisualizer::EndEditing()
{
	// Should validate that the new positions are convex if hull?
	CurrentEditMode = EM_None;
	RootComponent = nullptr;
	VertexIndex = -1;
	FComponentVisualizer::EndEditing();
}

bool FNCellRootComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (RootComponent == nullptr) return false;
	
	if (CurrentEditMode == EM_HullVertex)
	{
		const FScopedTransaction HullVertexTransaction(NSLOCTEXT("NexusProcGenEditor", "FNCellRootComponentVisualizer_AdjustHullVertex", "Adjust Hull Vertex"));
	
		RootComponent->Modify();
		RootComponent->Details.HullSettings.bCalculateOnSave = false;
		RootComponent->Details.Hull.bIsChaosGenerated = false;
		
		const FVector PreviousPosition = RootComponent->Details.Hull.Vertices[VertexIndex];
		RootComponent->Details.Hull.Vertices[VertexIndex] += DeltaTranslate;
		RootComponent->Details.Hull.Validate();
		if (!RootComponent->Details.Hull.IsConvex())
		{
			RootComponent->Details.Hull.Vertices[VertexIndex] = PreviousPosition;
		}
		RootComponent->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	if (CurrentEditMode == EM_BoundsVertex)
	{
		const FScopedTransaction HullVertexTransaction(NSLOCTEXT("NexusProcGenEditor", "FNCellRootComponentVisualizer_AdjustBoundsVertex", "Adjust Bounds Vertex"));
	
		RootComponent->Modify();
		RootComponent->Details.BoundsSettings.bCalculateOnSave = false;
		if (VertexIndex == 0)
		{
			RootComponent->Details.Bounds.Min += DeltaTranslate;
		}
		else
		{
			RootComponent->Details.Bounds.Max += DeltaTranslate;
		}
		RootComponent->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	return false;
}

bool FNCellRootComponentVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (CurrentEditMode == EM_None) return false;

	if (Key == EKeys::Escape && Event == EInputEvent::IE_Pressed)
	{
		EndEditing();
		return true;
	}

	return false;
}

bool FNCellRootComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	switch (CurrentEditMode)
	{
	case EM_None:
		return false;
	case EM_HullVertex:
		OutLocation = FNProcGenEdMode::GetCachedHullVertices()[VertexIndex];
		return true;
	case EM_BoundsVertex:
		if (VertexIndex == 0)
		{
			OutLocation = FNProcGenEdMode::GetCachedBounds().Min;
		}
		else
		{
			OutLocation = FNProcGenEdMode::GetCachedBounds().Max;
		}
		return true;
	default:
		return false;
	}
}