// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// #SONARQUBE-DISABLE-CPP_S107 Necessary verbosity of methods to fully convey the drawing methods

/**
 * A set of functionality made to extend DrawDebug commands.
 */
class NEXUSCORE_API FNDrawDebugHelpers
{
public:
	/**
	 * Draws a string using the FNPrimitiveFont.
	 * @param InWorld Which world to operate in.
	 * @param String The string to draw out.
	 * @param Position The world position to start drawing the string at.
	 * @param Rotation The world rotation to apply to the drawing, the base orientation is backwards facing. 
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long should the lines last if not permanent?
	 * @param DepthPriority What priority should they be drawn at?
	 * @param ForegroundColor The color to use when drawing the lines for the string.
	 * @param Scale The multiplier to apply to glyph size.
	 * @param LineHeight The height used to represent a line.
	 * @param Thickness The thickness of the lines used to draw glyphs.
	 * @param bInvertLineFeed Should new lines be stacked on top of older lines?
	 * @param bDrawBelowPosition Should the top of the first line align with the position?
	 */
	static void DrawString(const UWorld* InWorld, FString& String, const FVector& Position,
		const FRotator& Rotation, bool bPersistentLines = false, float LifeTime=-1.f, uint8 DepthPriority = SDPG_World, 
		FLinearColor ForegroundColor = FLinearColor::White, float Scale = 1, float LineHeight = 4.f, 
		float Thickness = 8.f, const bool bInvertLineFeed = false, const bool bDrawBelowPosition = true);
	
	/**
	 * Draws a swept FCollisionShape from a start to an end position, dispatching to the correct shape helper.
	 * @param InWorld Which world to operate in.
	 * @param StartPosition World position where the sweep begins.
	 * @param EndPosition World position where the sweep ends.
	 * @param Quat Rotation of the swept shape.
	 * @param Shape The collision shape to sweep (Line, Box, Sphere or Capsule).
	 * @param Color Color used for the drawn debug lines.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long the lines should last if not permanent.
	 * @param DepthPriority The scene depth priority group to draw into.
	 * @param Thickness Line thickness in pixels.
	 */
	FORCEINLINE static void DrawSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition,
		const FQuat& Quat, const FCollisionShape& Shape, const FColor& Color, const bool bPersistentLines = false,
		const float LifeTime = -1.f, const uint8 DepthPriority = SDPG_World, const float Thickness = 2.f)
	{
		switch (Shape.ShapeType)
		{
		case ECollisionShape::Line:
			DrawDebugLine(InWorld, StartPosition, EndPosition, Color, bPersistentLines, LifeTime,
				DepthPriority, Thickness);
			break;
		case ECollisionShape::Box:
			DrawBoxSweep(InWorld, StartPosition, EndPosition, Quat, Shape.GetBox(), Color, bPersistentLines,
				LifeTime, DepthPriority, Thickness);
			break;
		case ECollisionShape::Sphere:
			DrawSphereSweep(InWorld, StartPosition, EndPosition, Shape.GetSphereRadius(), Color, bPersistentLines,
				LifeTime, DepthPriority, Thickness);
			break;
		case ECollisionShape::Capsule:
			DrawCapsuleSweep(InWorld, StartPosition, EndPosition, Quat, Shape.GetCapsuleHalfHeight(),
				Shape.GetCapsuleRadius(), Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
			break;
		default:
			break;
		}
	}

	/**
	 * Draws a swept box from a start to an end position, including the box at both endpoints and connecting edges.
	 * @param InWorld Which world to operate in.
	 * @param StartPosition World position where the sweep begins.
	 * @param EndPosition World position where the sweep ends.
	 * @param Quat Rotation of the swept box.
	 * @param HalfSize Half-extents of the box.
	 * @param Color Color used for the drawn debug lines.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long the lines should last if not permanent.
	 * @param DepthPriority The scene depth priority group to draw into.
	 * @param Thickness Line thickness in pixels.
	 */
	static void DrawBoxSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition,
		const FQuat& Quat, const FVector& HalfSize, const FColor& Color, bool bPersistentLines = false,
		float LifeTime = -1.f, uint8 DepthPriority = SDPG_World, float Thickness = 2.f);

	/**
	 * Draws a swept sphere from a start to an end position with connecting capsule-style tangent lines.
	 * @param InWorld Which world to operate in.
	 * @param StartPosition World position where the sweep begins.
	 * @param EndPosition World position where the sweep ends.
	 * @param Radius Sphere radius.
	 * @param Color Color used for the drawn debug lines.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long the lines should last if not permanent.
	 * @param DepthPriority The scene depth priority group to draw into.
	 * @param Thickness Line thickness in pixels.
	 */
	static void DrawSphereSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition,
		float Radius, const FColor& Color, bool bPersistentLines = false, float LifeTime = -1.f,
		uint8 DepthPriority = SDPG_World, float Thickness = 2.f);

	/**
	 * Draws a swept capsule from a start to an end position, including the capsule at both endpoints.
	 * @param InWorld Which world to operate in.
	 * @param StartPosition World position where the sweep begins.
	 * @param EndPosition World position where the sweep ends.
	 * @param Quat Rotation of the swept capsule.
	 * @param HalfHeight Half-height of the capsule along its local Z axis.
	 * @param Radius Radius of the capsule.
	 * @param Color Color used for the drawn debug lines.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long the lines should last if not permanent.
	 * @param DepthPriority The scene depth priority group to draw into.
	 * @param Thickness Line thickness in pixels.
	 */
	static void DrawCapsuleSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition,
		const FQuat& Quat, float HalfHeight, float Radius, const FColor& Color, bool bPersistentLines = false,
		float LifeTime = -1.f, uint8 DepthPriority = SDPG_World, float Thickness = 2.f);

	/**
	 * Draws a static (non-swept) collision shape at a given position and orientation.
	 * @param InWorld Which world to operate in.
	 * @param Position World position to draw the shape at.
	 * @param Quat Rotation applied to the shape.
	 * @param Shape The collision shape to render.
	 * @param Color Color used for the drawn debug lines.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long the lines should last if not permanent.
	 * @param DepthPriority The scene depth priority group to draw into.
	 * @param Thickness Line thickness in pixels.
	 */
	static void DrawCollisionShape(const UWorld* InWorld, const FVector& Position, const FQuat& Quat,
		const FCollisionShape& Shape, const FColor& Color, bool bPersistentLines = false, float LifeTime = -1.f,
		uint8 DepthPriority = SDPG_World, float Thickness = 2.f);
	
// #SONARQUBE-ENABLE
};