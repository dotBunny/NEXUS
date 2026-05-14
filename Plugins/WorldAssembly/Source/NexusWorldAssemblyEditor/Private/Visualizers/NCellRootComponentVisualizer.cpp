// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Visualizers/NCellRootComponentVisualizer.h"
#include "Cell/NCellRootComponent.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblySettings.h"
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
	
	// We're not going to draw proxies, but its unlikely this is gonna be ever hit
	const ANCellActor* CellActor = CellRootComponent->GetNCellActor();
	if (CellActor != nullptr && CellActor->WasSpawnedFromProxy())
	{
		return;
	}
	

	// We need to draw the base wireframes
	if (!FNWorldAssemblyEdMode::IsActive())
	{
		CellRootComponent->DrawDebugPDI(PDI, static_cast<uint8>(FNWorldAssemblyEdMode::GetCellVoxelMode()));
		return;
	}

	if (FNWorldAssemblyEdMode::GetCellEdMode() == FNWorldAssemblyEdMode::ENCellEdMode::Bounds)
	{
		const FBox Bounds = FNWorldAssemblyEdMode::GetCachedBounds();
		const TArray<FVector> BoundsVertices = FNWorldAssemblyEdMode::GetCachedBoundsVertices();
		
		// Draw Min Max
		PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, 0));
		PDI->DrawPoint(Bounds.Min, FNWorldAssemblyEdMode::GetCachedBoundsColor(), PointSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
		PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, 1));
		PDI->DrawPoint(Bounds.Max, FNWorldAssemblyEdMode::GetCachedBoundsColor(), PointSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);

	}
	else if (FNWorldAssemblyEdMode::GetCellEdMode() == FNWorldAssemblyEdMode::ENCellEdMode::Hull)
	{
		const TArray<FVector> WorldVertices = FNWorldAssemblyEdMode::GetCachedHullVertices();
		const int32 VertCount = WorldVertices.Num();
		for (int32 i = 0; i < VertCount; i++)
		{
			PDI->SetHitProxy(new HNIndexComponentVisProxy(Component, i));
			PDI->DrawPoint(WorldVertices[i], FNWorldAssemblyEdMode::GetCachedHullColor(), PointSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);
		}
	}
	else if (FNWorldAssemblyEdMode::GetCellEdMode() == FNWorldAssemblyEdMode::ENCellEdMode::Voxel)
	{
		// Forcibly disable drawing of the voxel Mode
		if (FNWorldAssemblyEdMode::GetCellVoxelMode() != FNWorldAssemblyEdMode::ENCellVoxelMode::None)
		{
			FNWorldAssemblyEdMode::SetCellVoxelMode(FNWorldAssemblyEdMode::ENCellVoxelMode::None);
		}
		
		const FNCellVoxelData CachedData = FNWorldAssemblyEdMode::GetCachedVoxelData();
		if (!CachedData.IsValid())
		{
			return;
		}

		const size_t PointCount = CachedData.GetCount();
		const UNWorldAssemblySettings* Settings = GetDefault<UNWorldAssemblySettings>();
		const FVector UnitSize = Settings->VoxelSize;
		const FVector HalfUnitSize = UnitSize * 0.5f;
		const FVector BaseOffset = CachedData.Origin;
		for (int32 i = 0; i < PointCount; i++)
		{
			auto [x,y,z] = CachedData.GetInverseIndex(i);
	
			// TODO: #ROTATE-VOXELS Rotation needs to actually rotated to the nearest grid???
			FVector VoxelCenter = BaseOffset + ((FVector(x, y, z) * UnitSize) + HalfUnitSize);
	
			if (N_FLAGS_HAS(CachedData.GetData(i), static_cast<uint8>(ENCellVoxel::Occupied)))
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
			
			using enum FNWorldAssemblyEdMode::ENCellEdMode;
			if (FNWorldAssemblyEdMode::GetCellEdMode() == Bounds)
			{
				return EditBoundsVertex(IndexComponent, Proxy->Index);
			}
			if (FNWorldAssemblyEdMode::GetCellEdMode() == Hull)
			{
				return EditHullVertex(IndexComponent, Proxy->Index);
			}
			if (FNWorldAssemblyEdMode::GetCellEdMode() == Voxel)
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
	CurrentEditMode = ENCellEditMode::HullVertex;
	RootComponent = Component;
	VertexIndex = Index;
	return true;
}

bool FNCellRootComponentVisualizer::EditBoundsVertex(UNCellRootComponent* Component, int32 Index)
{
	CurrentEditMode = ENCellEditMode::BoundsVertex;
	RootComponent = Component;
	VertexIndex = Index;
	return true;
}

bool FNCellRootComponentVisualizer::ToggleVoxelPoint(UNCellRootComponent* Component, const int32 Index)
{
	uint8 Data = Component->Details.VoxelData.GetData(Index);
	
	// Handle Occupied
	if (N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::Occupied)))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNCellRootComponentVisualizer_Voxel_Empty", "Set Voxel Empty"));
		Component->Modify();
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::Occupied));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::Empty));
		Component->Details.VoxelData.SetData(Index, Data);
		Component->Details.VoxelSettings.bCalculateOnSave = false;
		Component->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	// Handle Empty
	if (N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::Empty)))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNCellRootComponentVisualizer_Voxel_Occupied", "Set Voxel Occupied"));
		Component->Modify();
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::Empty));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::Occupied));
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
	CurrentEditMode = ENCellEditMode::None;
	RootComponent = nullptr;
	VertexIndex = -1;
	FComponentVisualizer::EndEditing();
}

bool FNCellRootComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (RootComponent == nullptr) return false;
	
	if (CurrentEditMode == ENCellEditMode::HullVertex)
	{
		const FScopedTransaction HullVertexTransaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNCellRootComponentVisualizer_AdjustHullVertex", "Adjust Hull Vertex"));
	
		RootComponent->Modify();
		RootComponent->Details.HullSettings.bCalculateOnSave = false;
		RootComponent->Details.Hull.bIsChaosGenerated = false;
		
		const FVector PreviousPosition = RootComponent->Details.Hull.Vertices[VertexIndex];
		RootComponent->Details.Hull.Vertices[VertexIndex] += DeltaTranslate;
		RootComponent->Details.Hull.Validate();
		RootComponent->Details.Hull.CalculateCenterAndBounds();
		if (!RootComponent->Details.Hull.IsConvex())
		{
			RootComponent->Details.Hull.Vertices[VertexIndex] = PreviousPosition;
		}
		RootComponent->GetNCellActor()->SetActorDirty();
		return true;
	}
	
	if (CurrentEditMode == ENCellEditMode::BoundsVertex)
	{
		const FScopedTransaction HullVertexTransaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNCellRootComponentVisualizer_AdjustBoundsVertex", "Adjust Bounds Vertex"));
	
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
	if (CurrentEditMode == ENCellEditMode::None) return false;

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
	case ENCellEditMode::None:
		return false;
	case ENCellEditMode::HullVertex:
		OutLocation = FNWorldAssemblyEdMode::GetCachedHullVertices()[VertexIndex];
		return true;
	case ENCellEditMode::BoundsVertex:
		if (VertexIndex == 0)
		{
			OutLocation = FNWorldAssemblyEdMode::GetCachedBounds().Min;
		}
		else
		{
			OutLocation = FNWorldAssemblyEdMode::GetCachedBounds().Max;
		}
		return true;
	default:
		return false;
	}
}