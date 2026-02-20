// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDrawDebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Developer/NPrimitiveFont.h"
#include "Components/LineBatchComponent.h"

// #SONARQUBE-DISABLE-CPP_S107 Verbosity necessary, most are defaulted regardless.

void FNDrawDebugHelpers::DrawString(const UWorld* InWorld, FString& String, const FVector& Position,
	const FRotator& Rotation, const bool bPersistentLines, const float LifeTime, const uint8 DepthPriority, const FLinearColor ForegroundColor,
	const float Scale, const float LineHeight, const float Thickness, const bool bInvertLineFeed, const bool bDrawBelowPosition)
{
#if ENABLE_DRAW_DEBUG
	// Ensure we are not a dedicated server who doesn't have a visual side
	if (GEngine->GetNetMode(InWorld) == NM_DedicatedServer)
	{
		return;
	}
	
	// Find the appropriate batching component
	ULineBatchComponent* LineBatcher = nullptr;
	if (InWorld)
	{
		if (bPersistentLines || LifeTime > 0.f)
		{
			LineBatcher = DepthPriority ? InWorld->GetLineBatcher(UWorld::ELineBatcherType::ForegroundPersistent) : InWorld->GetLineBatcher(UWorld::ELineBatcherType::WorldPersistent);
		}
		else
		{
			LineBatcher =  DepthPriority ? InWorld->GetLineBatcher(UWorld::ELineBatcherType::Foreground) : InWorld->GetLineBatcher(UWorld::ELineBatcherType::World);
		}
	}
	
	// If we can't find one, we shouldn't be trying to draw anything
	if (LineBatcher == nullptr)
	{
		return;
	}

	// Determine line life-time and override as needed
	float LineLifeTime = LineBatcher->DefaultLifeTime;
	if (LifeTime > 0.f)
	{
		LineLifeTime = LifeTime;
	}
	if (bPersistentLines)
	{
		LineLifeTime = -1.0f;
	}
	
	FNPrimitiveFont::DrawBatchString(LineBatcher, String, Position, Rotation, ForegroundColor, Scale, LineHeight, Thickness, LineLifeTime);

#endif // ENABLE_DRAW_DEBUG
}

void FNDrawDebugHelpers::DrawBoxSweep(const UWorld* InWorld, const FVector& StartPosition,
	const FVector& EndPosition, const FQuat& Quat, const FVector& HalfSize, const FColor& Color,
	const bool bPersistentLines, const float LifeTime, const uint8 DepthPriority, const float Thickness)
{
	const FVector Direction = EndPosition - StartPosition;
	FVector Vertices[8];
	Vertices[0] = StartPosition + Quat.RotateVector(FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Vertices[1] = StartPosition + Quat.RotateVector(FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z));
	Vertices[2] = StartPosition + Quat.RotateVector(FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z));
	Vertices[3] = StartPosition + Quat.RotateVector(FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z));
	Vertices[4] = StartPosition + Quat.RotateVector(FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Vertices[5] = StartPosition + Quat.RotateVector(FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z));
	Vertices[6] = StartPosition + Quat.RotateVector(FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z));
	Vertices[7] = StartPosition + Quat.RotateVector(FVector(HalfSize.X, HalfSize.Y, HalfSize.Z));

	DrawDebugBox(InWorld, StartPosition, HalfSize, Quat, Color, bPersistentLines, LifeTime, 
		DepthPriority, Thickness);
	DrawDebugBox(InWorld, EndPosition, HalfSize, Quat, Color, bPersistentLines, LifeTime, 
		DepthPriority, Thickness);
	for (int32 i = 0; i < 8; ++i)
	{
		DrawDebugLine(InWorld, Vertices[i], Vertices[i] + Direction, Color, bPersistentLines, LifeTime, 
			DepthPriority, Thickness);
	}
}

void FNDrawDebugHelpers::DrawSphereSweep(const UWorld* InWorld, const FVector& StartPosition,
	const FVector& EndPosition, const float Radius, const FColor& Color, const bool bPersistentLines, 
	const float LifeTime, const uint8 DepthPriority, const float Thickness)
{
	const FVector Direction = EndPosition - StartPosition;
	const FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
	
	DrawDebugCapsule(InWorld, StartPosition + (Direction * 0.5f), (Direction.Size() * 0.5f) + Radius, 
		Radius, CapsuleRotation, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
	
	DrawCircle(InWorld, StartPosition, CapsuleRotation.GetAxisY(), CapsuleRotation.GetAxisZ(), Color, Radius, 
		24, bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawCircle(InWorld, StartPosition, CapsuleRotation.GetAxisX(), CapsuleRotation.GetAxisZ(), Color, Radius, 
		24, bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawCircle(InWorld, EndPosition, CapsuleRotation.GetAxisY(), -CapsuleRotation.GetAxisZ(), Color, Radius, 
		24, bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawCircle(InWorld, EndPosition, CapsuleRotation.GetAxisX(), -CapsuleRotation.GetAxisZ(), Color, Radius, 
		24, bPersistentLines, LifeTime, DepthPriority, Thickness);
}

void FNDrawDebugHelpers::DrawCapsuleSweep(const UWorld* InWorld, const FVector& StartPosition,
	const FVector& EndPosition, const FQuat& Quat, const float HalfHeight, const float Radius, const FColor& Color,
	const bool bPersistentLines, const float LifeTime, const uint8 DepthPriority, const float Thickness)
{
	DrawDebugCapsule(InWorld, StartPosition, HalfHeight, Radius, Quat, Color, bPersistentLines, LifeTime, 
		DepthPriority, Thickness);
	DrawDebugCapsule(InWorld, EndPosition, HalfHeight, Radius, Quat, Color, bPersistentLines, LifeTime, 
		DepthPriority, Thickness);

	const float HalfLength = HalfHeight - Radius;
	
	const FVector ZAxis = Quat.GetUpVector();
	const FMatrix Matrix = FRotationMatrix::MakeFromZX(EndPosition - StartPosition, ZAxis);
	const FVector YAxis = Matrix.GetUnitAxis(EAxis::Y);
	const FVector XAxis =  Matrix.GetUnitAxis(EAxis::X);
	
	const FVector TopStartPosition = StartPosition + HalfLength * ZAxis;
	const FVector TopEndPosition = EndPosition + HalfLength * ZAxis;
	
	DrawDebugLine(InWorld, TopStartPosition + (Radius * XAxis), TopEndPosition + (Radius * XAxis), Color, 
		bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawDebugLine(InWorld, TopStartPosition + (Radius * YAxis), TopEndPosition + (Radius * YAxis), Color, 
		bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawDebugLine(InWorld, TopStartPosition - (Radius * YAxis), TopEndPosition - (Radius * YAxis), Color, 
		bPersistentLines, LifeTime, DepthPriority, Thickness);

	const FVector BottomStartPosition = StartPosition - HalfLength * ZAxis;
	const FVector BottomEndPosition = EndPosition - HalfLength * ZAxis;
	
	DrawDebugLine(InWorld, BottomStartPosition - (Radius * XAxis), BottomEndPosition - (Radius * XAxis), Color, 
		bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawDebugLine(InWorld, BottomStartPosition + (Radius * YAxis), BottomEndPosition + (Radius * YAxis), Color,
		bPersistentLines, LifeTime, DepthPriority, Thickness);
	DrawDebugLine(InWorld, BottomStartPosition - (Radius * YAxis), BottomEndPosition - (Radius * YAxis), Color, 
		bPersistentLines, LifeTime, DepthPriority, Thickness);
}

void FNDrawDebugHelpers::DrawCollisionShape(const UWorld* InWorld, const FVector& Position, const FQuat& Quat,
	const FCollisionShape& Shape, const FColor& Color, const bool bPersistentLines, const float LifeTime, 
	const uint8 DepthPriority, const float Thickness)
{
	switch (Shape.ShapeType)
	{
	case ECollisionShape::Box:
		DrawDebugBox(InWorld, Position, Shape.GetBox(), Quat, Color, bPersistentLines, LifeTime, 
			DepthPriority, Thickness);
		break;
	case ECollisionShape::Sphere:
		DrawCircle(InWorld, Position, Quat.GetAxisX(), Quat.GetAxisY(), Color, Shape.GetSphereRadius(), 
			24, bPersistentLines, LifeTime, DepthPriority, Thickness);
		DrawCircle(InWorld, Position, Quat.GetAxisX(), Quat.GetAxisZ(), Color, Shape.GetSphereRadius(), 
			24, bPersistentLines, LifeTime, DepthPriority, Thickness);
		break;
	case ECollisionShape::Capsule:
		DrawDebugCapsule(InWorld, Position, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), Quat, Color, 
			bPersistentLines, LifeTime, DepthPriority, Thickness);
		break;
	case ECollisionShape::Line:
	default:
		break;
	}
}

// #SONARQUBE-ENABLE
