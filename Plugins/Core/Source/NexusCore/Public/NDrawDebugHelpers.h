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
	 * 
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
	
	static void DrawBoxSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition, 
		const FQuat& Quat, const FVector& HalfSize, const FColor& Color, bool bPersistentLines = false, 
		float LifeTime = -1.f, uint8 DepthPriority = SDPG_World, float Thickness = 2.f);
	
	static void DrawSphereSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition, 
		float Radius, const FColor& Color, bool bPersistentLines = false, float LifeTime = -1.f, 
		uint8 DepthPriority = SDPG_World, float Thickness = 2.f);
	
	static void DrawCapsuleSweep(const UWorld* InWorld, const FVector& StartPosition, const FVector& EndPosition, 
		const FQuat& Quat, float HalfHeight, float Radius, const FColor& Color, bool bPersistentLines = false, 
		float LifeTime = -1.f, uint8 DepthPriority = SDPG_World, float Thickness = 2.f);
	
	static void DrawCollisionShape(const UWorld* InWorld, const FVector& Position, const FQuat& Quat,
		const FCollisionShape& Shape, const FColor& Color, bool bPersistentLines = false, float LifeTime = -1.f, 
		uint8 DepthPriority = SDPG_World, float Thickness = 2.f);
	
// #SONARQUBE-ENABLE
};