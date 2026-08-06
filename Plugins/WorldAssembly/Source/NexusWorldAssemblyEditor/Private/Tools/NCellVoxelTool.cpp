// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Tools/NCellVoxelTool.h"

#include "InteractiveToolManager.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Cell/NCellRootComponent.h"
#include "NWorldAssemblySettings.h"
#include "Macros/NFlagsMacros.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

UInteractiveTool* UNCellVoxelToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UNCellVoxelTool>(SceneState.ToolManager);
}

void UNCellVoxelTool::Setup()
{
	UNCellToolBase::Setup();

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);
	AddInputBehavior(ClickBehavior);

	GetToolManager()->DisplayMessage(
		LOCTEXT("NCellVoxelTool_Message", "Click a voxel to toggle it between occupied and empty."),
		EToolMessageLevel::UserNotification);
}

void UNCellVoxelTool::Shutdown(const EToolShutdownType ShutdownType)
{
	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	UNCellToolBase::Shutdown(ShutdownType);
}

FInputRayHit UNCellVoxelTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	double Distance = 0.0;
	if (FindVoxelUnderRay(ClickPos.WorldRay, Distance) != INDEX_NONE)
	{
		return FInputRayHit(static_cast<float>(Distance));
	}
	return FInputRayHit();
}

void UNCellVoxelTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	double Distance = 0.0;
	if (const int32 Index = FindVoxelUnderRay(ClickPos.WorldRay, Distance); Index != INDEX_NONE)
	{
		ToggleVoxel(Index);
	}
}

int32 UNCellVoxelTool::FindVoxelUnderRay(const FRay& Ray, double& OutDistance) const
{
	const FNCellVoxelData& VoxelData = UNWorldAssemblyEdMode::GetCachedVoxelData();
	if (!VoxelData.IsValid()) return INDEX_NONE;

	// Same anchoring the overlay draws with: the grid origin is already world-space, so no offset transform applies.
	const FVector UnitSize = UNWorldAssemblySettings::Get()->VoxelSize;
	const FVector HalfUnitSize = UnitSize * 0.5f;
	const FVector BaseOffset = VoxelData.GetOrigin();

	const int32 PointCount = static_cast<int32>(VoxelData.GetCount());

	int32 BestIndex = INDEX_NONE;
	double BestRayParameter = 0.0;

	for (int32 i = 0; i < PointCount; i++)
	{
		auto [x, y, z] = VoxelData.GetInverseIndex(i);
		const FVector VoxelCenter = BaseOffset + (FVector(x, y, z) * UnitSize) + HalfUnitSize;

		double RayParameter = 0.0;
		if (!IsPickedByRay(Ray, VoxelCenter, RayParameter)) continue;
		if (BestIndex != INDEX_NONE && RayParameter >= BestRayParameter) continue;

		BestIndex = i;
		BestRayParameter = RayParameter;
	}

	if (BestIndex != INDEX_NONE)
	{
		OutDistance = BestRayParameter;
	}
	return BestIndex;
}

void UNCellVoxelTool::ToggleVoxel(const int32 Index) const
{
	UNCellRootComponent* RootComponent = GetCellRoot();
	if (RootComponent == nullptr) return;

	uint8 Data = RootComponent->Details.VoxelData.GetData(Index);

	const bool bOccupied = N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::Occupied));
	const bool bEmpty = N_FLAGS_HAS(Data, static_cast<uint8>(ENCellVoxel::Empty));

	// Anything that is neither is not a voxel this tool authored a state for; leave it alone rather than guessing.
	if (!bOccupied && !bEmpty) return;

	const FScopedTransaction Transaction(bOccupied
		? LOCTEXT("NCellVoxelTool_SetEmpty", "Set Voxel Empty")
		: LOCTEXT("NCellVoxelTool_SetOccupied", "Set Voxel Occupied"));

	RootComponent->Modify();

	if (bOccupied)
	{
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::Occupied));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::Empty));
	}
	else
	{
		N_FLAGS_REMOVE(Data, static_cast<uint8>(ENCellVoxel::Empty));
		N_FLAGS_ADD(Data, static_cast<uint8>(ENCellVoxel::Occupied));
	}

	RootComponent->Details.VoxelData.SetData(Index, Data);

	// A hand-edited grid is no longer something the save-time pass should overwrite.
	RootComponent->Details.VoxelSettings.bCalculateOnSave = false;

	MarkCellDirty();
}

#undef LOCTEXT_NAMESPACE
