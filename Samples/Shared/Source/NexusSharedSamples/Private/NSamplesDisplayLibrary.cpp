#include "NSamplesDisplayLibrary.h"

#include "NColor.h"
#include "NSamplesDisplayActor.h"
FMatrix UNSamplesDisplayLibrary::BaseDrawMatrix = FRotationMatrix::MakeFromYZ(FVector::ForwardVector, FVector::LeftVector);

void UNSamplesDisplayLibrary::TimerDrawPoint(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
	const int TimerIntervals)
{
	DrawDebugPoint(SamplesDisplay->GetWorld(), Location, 15.f, FNColor::GetColor(ENColor::NC_Red),
	false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World);
}

void UNSamplesDisplayLibrary::TimerDrawPoints(ANSamplesDisplayActor* SamplesDisplay, const TArray<FVector>& Locations,
	const int TimerIntervals)
{
	for (const FVector& Location : Locations)
	{
		DrawDebugPoint(SamplesDisplay->GetWorld(), Location, 15.f, FNColor::GetColor(ENColor::NC_Red),
			false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World);
	}
}

void UNSamplesDisplayLibrary::TimerDrawSphere(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const float Radius,
                                              const int TimerIntervals)
{
	DrawDebugSphere(SamplesDisplay->GetWorld(), Location, Radius,24,
	FNColor::GetColor(ENColor::NC_White), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World);
}

void UNSamplesDisplayLibrary::TimerDrawComboSphere(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
                                                   const FVector2D InnerOuter, const int TimerIntervals)
{
	DrawDebugSphere(SamplesDisplay->GetWorld(), Location, InnerOuter.X, 24,
		FNColor::GetColor(ENColor::NC_Black), false,SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World, NEXUS::Samples::TimerDrawThickness);
	DrawDebugSphere(SamplesDisplay->GetWorld(), Location, InnerOuter.Y, 24,
		FNColor::GetColor(ENColor::NC_White), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World, NEXUS::Samples::TimerDrawThickness);
}

void UNSamplesDisplayLibrary::TimerDrawBox(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const FBox& Dimensions,
	const int TimerIntervals)
{
	DrawDebugBox(SamplesDisplay->GetWorld(), Location, Dimensions.GetExtent(),
		FNColor::GetColor(ENColor::NC_White), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals,
		SDPG_World, NEXUS::Samples::TimerDrawThickness);
}

void UNSamplesDisplayLibrary::TimerDrawComboBox(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
	const FBox& InnerDimensions, const FBox& OuterDimensions, const int TimerIntervals)
{
	DrawDebugBox(SamplesDisplay->GetWorld(), Location, InnerDimensions.GetExtent(),
			FNColor::GetColor(ENColor::NC_Black), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals,
			SDPG_World, NEXUS::Samples::TimerDrawThickness);

	DrawDebugBox(SamplesDisplay->GetWorld(), Location, OuterDimensions.GetExtent(),
	FNColor::GetColor(ENColor::NC_White), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals,
SDPG_World, NEXUS::Samples::TimerDrawThickness);	
}

void UNSamplesDisplayLibrary::TimerDrawCircle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location, const float& Radius,
	const FRotator& Rotation, const int TimerIntervals)
{
	FMatrix DrawMatrix = BaseDrawMatrix * FRotationMatrix(Rotation);
	DrawMatrix.SetOrigin(Location);
		
	DrawDebugCircle(SamplesDisplay->GetWorld(), DrawMatrix, Radius, 24,
		FNColor::GetColor(ENColor::NC_White), false,SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World,
		NEXUS::Samples::TimerDrawThickness,false);
}

void UNSamplesDisplayLibrary::TimerDrawComboCircle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
	const FVector2D& InnerOuter, const FRotator& Rotation, const int TimerIntervals)
{
	FMatrix DrawMatrix = BaseDrawMatrix * FRotationMatrix(Rotation);
	DrawMatrix.SetOrigin(Location);

	DrawDebugCircle(SamplesDisplay->GetWorld(), DrawMatrix, InnerOuter.X, 24,
		FNColor::GetColor(ENColor::NC_Black), false, SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals,
		SDPG_World, NEXUS::Samples::TimerDrawThickness, false);

	DrawDebugCircle(SamplesDisplay->GetWorld(), DrawMatrix, InnerOuter.Y, 24,
		FNColor::GetColor(ENColor::NC_White), false,SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals, SDPG_World,
		NEXUS::Samples::TimerDrawThickness, false);
}

void UNSamplesDisplayLibrary::TimerDrawSpline(ANSamplesDisplayActor* SamplesDisplay, const USplineComponent* Spline,
	const int TimerIntervals)
{
	const UWorld* World = SamplesDisplay->GetWorld();
	const float LifeTime = SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals;
	TArray<FVector> SplinePoints;
	const float SplineLength = Spline->GetSplineLength();
	const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); // One point every ~20 units
	const float DistancePerSegment = SplineLength / NumSegments;
	SplinePoints.Reserve(NumSegments);
	for (int32 i = 0; i <= NumSegments; ++i)
	{
		const float Distance = DistancePerSegment * i;
		const FVector Point = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		SplinePoints.Add(Point);
	}
	for (int32 i = 0; i < SplinePoints.Num() - 1; ++i)
	{
		DrawDebugLine(World, SplinePoints[i], SplinePoints[i + 1], FNColor::GetColor(ENColor::NC_White),
			false, LifeTime, SDPG_World, NEXUS::Samples::TimerDrawThickness);
	}
}

void UNSamplesDisplayLibrary::TimerDrawRectangle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
	const FVector2D& Dimensions, const FRotator& Rotation, int TimerIntervals)
{
	const UWorld* World = SamplesDisplay->GetWorld();
	const float LifeTime = SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals;
	TArray<FVector> RectanglePoints;
	RectanglePoints.Reserve(5);
		
	const float ExtentX = Dimensions.X * 0.5f;
	const float ExtentY = Dimensions.Y * 0.5f;

	if (Rotation.IsZero())
	{
		RectanglePoints.Add(Location + FVector(-ExtentX, -ExtentY, 0));
		RectanglePoints.Add(Location + FVector(-ExtentX, ExtentY, 0));
		RectanglePoints.Add(Location + FVector(ExtentX, ExtentY, 0));
		RectanglePoints.Add(Location + FVector(ExtentX, -ExtentY, 0));
		
		RectanglePoints.Add(Location + FVector(-ExtentX, -ExtentY, 0));
	}
	else
	{
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, -ExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, ExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(ExtentX, ExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(ExtentX, -ExtentY, 0)));
		
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-ExtentX, -ExtentY, 0)));
	}
	for (int32 i = 0; i < 4; ++i)
	{
		DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(ENColor::NC_White),
			false, LifeTime, SDPG_World, NEXUS::Samples::TimerDrawThickness);
	}
}

void UNSamplesDisplayLibrary::TimerDrawComboRectangle(ANSamplesDisplayActor* SamplesDisplay, const FVector Location,
	const FVector2D& InnerDimensions, const FVector2D& OuterDimensions, const FRotator& Rotation,
	const int TimerIntervals)
{
	const UWorld* World = SamplesDisplay->GetWorld();
	const float LifeTime = SamplesDisplay->TimerSettings.TimerDuration * TimerIntervals;
	TArray<FVector> RectanglePoints;
	RectanglePoints.Reserve(5);
	
	const float InnerExtentX = InnerDimensions.X * 0.5f;
	const float InnerExtentY = InnerDimensions.Y * 0.5f;
	const float OuterExtentX = OuterDimensions.X * 0.5f;
	const float OuterExtentY = OuterDimensions.Y * 0.5f;

	if (Rotation.IsZero())
	{
		RectanglePoints.Add(Location + FVector(-InnerExtentX, -InnerExtentY, 0));
		RectanglePoints.Add(Location + FVector(-InnerExtentX, InnerExtentY, 0));
		RectanglePoints.Add(Location + FVector(InnerExtentX, InnerExtentY, 0));
		RectanglePoints.Add(Location + FVector(InnerExtentX, -InnerExtentY, 0));

		RectanglePoints.Add(Location + FVector(-InnerExtentX, -InnerExtentY, 0));
	}
	else
	{
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, -InnerExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, InnerExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(InnerExtentX, InnerExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(InnerExtentX, -InnerExtentY, 0)));

		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-InnerExtentX, -InnerExtentY, 0)));
	}
	
	for (int32 i = 0; i < 4; ++i)
	{
		DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(ENColor::NC_Black),
			false, LifeTime, SDPG_World, NEXUS::Samples::TimerDrawThickness);
	}

	RectanglePoints.Empty();

	if (Rotation.IsZero())
	{
		RectanglePoints.Add(Location + FVector(-OuterExtentX, -OuterExtentY, 0));
		RectanglePoints.Add(Location + FVector(-OuterExtentX, OuterExtentY, 0));
		RectanglePoints.Add(Location + FVector(OuterExtentX, OuterExtentY, 0));
		RectanglePoints.Add(Location + FVector(OuterExtentX, -OuterExtentY, 0));
	
		RectanglePoints.Add(Location + FVector(-OuterExtentX, -OuterExtentY, 0));
	}
	else
	{
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, -OuterExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, OuterExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(OuterExtentX, OuterExtentY, 0)));
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(OuterExtentX, -OuterExtentY, 0)));
	
		RectanglePoints.Add(Location + Rotation.RotateVector(FVector(-OuterExtentX, -OuterExtentY, 0)));
	}
	
	for (int32 i = 0; i < 4; ++i)
	{
		DrawDebugLine(World, RectanglePoints[i], RectanglePoints[i + 1], FNColor::GetColor(ENColor::NC_White),
			false, LifeTime, SDPG_World, NEXUS::Samples::TimerDrawThickness);
	}
}
