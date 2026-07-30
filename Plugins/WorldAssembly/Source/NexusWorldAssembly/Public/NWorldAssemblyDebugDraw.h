// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "PrimitiveDrawInterface.h"
#include "Cell/NCellJunctionDetails.h"
#include "Cell/NCellVoxelData.h"
#include "Engine/EngineTypes.h"

struct FNRawMesh;

/**
 * Parameters describing how a junction socket should be rendered by FNWorldAssemblyDebugDraw::DrawSocket.
 *
 * Carries both the authored unit size and the world size it resolves to, the junction type (which
 * decides what supplementary geometry is drawn), the draw colour, and whether the socket is connected.
 */
struct NEXUSWORLDASSEMBLY_API FNDrawSocketSettings
{
	/** A unit representation of size (width(x) and height(y)). No concept of depth. */
	FIntVector2 UnitSize;

	/** The actual world size of each unit dimension (width(x) and height(y)). */
	FVector2D SocketSize;

	/** An indicator of the socket type determining what sort of additional information is drawn. */
	ENCellJunctionType SocketType;

	/** The color to draw the socket. */
	FLinearColor Color;

	bool bIsConnected = false;

	bool bDrawBox = true;

	bool bDrawCornerLines = true;

	bool bDrawFillDepth = false;

	ENCellJunctionFillDepthMode FillDepthMode = ENCellJunctionFillDepthMode::DefaultForward;

	float FillDepth = 10.f;
};

/**
 * Shared debug-draw helpers for cell/junction/voxel overlays used across editor-mode visualizers.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-debug-draw/">FNWorldAssemblyDebugDraw</a>
 */
class NEXUSWORLDASSEMBLY_API FNWorldAssemblyDebugDraw
{
public:
	/**
	 * Draws a rectangular representation of the provided socket, rotated 90-degrees as to better represent the socket.
	 * @param PDI Drawing Interface
	 * @param Location The World Location that is the center of the drawn rectangle
	 * @param Rotation The World Rotation that represents the forward direction of the socket.
	 * @param DrawSettings Settings used for drawing the socket.
	 */
	static void DrawSocket(FPrimitiveDrawInterface* PDI, const FVector& Location, const FRotator& Rotation, const FNDrawSocketSettings& DrawSettings);

	/**
	 * Draw the edges of Mesh as dashed line segments, offset and rotated into world space.
	 * @param PDI The draw interface to submit line segments to.
	 * @param Mesh The mesh whose edges are drawn; its vertices are in local space.
	 * @param Rotation Rotation applied to Mesh's vertices before drawing.
	 * @param Offset World-space translation applied after Rotation.
	 * @param Color Colour of the dashed segments.
	 * @param DashSize Length of each dash, in world units.
	 * @param Priority Scene depth priority group the segments are drawn in.
	 */
	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);

	/**
	 * Draw the edges of Mesh as dashed line segments using pre-transformed world vertices.
	 * @param PDI The draw interface to submit line segments to.
	 * @param Mesh The mesh supplying edge topology; its own vertices are ignored in favour of WorldVertices.
	 * @param WorldVertices Vertex positions already in world space; must parallel Mesh's vertex array.
	 * @param Color Colour of the dashed segments.
	 * @param DashSize Length of each dash, in world units.
	 * @param Priority Scene depth priority group the segments are drawn in.
	 */
	static void DrawDashedRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices,
		FLinearColor Color, float DashSize = 2, ESceneDepthPriorityGroup Priority = SDPG_World);


	static void DrawRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const FRotator& Rotation, const FVector& Offset,
		FLinearColor Color, ESceneDepthPriorityGroup Priority = SDPG_World);

	static void DrawRawMesh(FPrimitiveDrawInterface* PDI, const FNRawMesh& Mesh, const TArray<FVector>& WorldVertices,
		FLinearColor Color, ESceneDepthPriorityGroup Priority = SDPG_World);

	/**
	 * Render every occupied voxel as a wire box in SDPG_World.
	 * @param PDI The draw interface to submit primitives to.
	 * @param VoxelData Occupancy grid to render; voxel size is read from UNWorldAssemblySettings::VoxelSize.
	 * @param Offset World-space translation added to the grid's own Origin.
	 * @param Rotation Currently unused — see #ROTATE-VOXELS.
	 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-debug-draw/#drawing-voxels">Drawing Voxels</a>
	 */
	static void DrawVoxelDataGrid(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);

	/**
	 * Render every occupied voxel as a point in SDPG_Foreground, plus a yellow wire box marking voxel
	 * (0,0,0) whether or not it is occupied.
	 * @param PDI The draw interface to submit primitives to.
	 * @param VoxelData Occupancy grid to render; voxel size is read from UNWorldAssemblySettings::VoxelSize.
	 * @param Offset World-space translation added to the grid's own Origin.
	 * @param Rotation Currently unused — see #ROTATE-VOXELS.
	 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-debug-draw/#drawing-voxels">Drawing Voxels</a>
	 */
	static void DrawVoxelDataPoints(FPrimitiveDrawInterface* PDI, const FNCellVoxelData& VoxelData, const FVector& Offset, const FRotator& Rotation);
};