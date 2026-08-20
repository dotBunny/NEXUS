// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "PCG/Elements/NRandomStepRotationElement.h"

#include "NColor.h"
#include "PCGContext.h"
#include "Data/PCGBasePointData.h"
#include "Helpers/PCGHelpers.h"

#if WITH_EDITOR
FLinearColor UNRandomStepRotationSettings::GetNodeTitleColor() const
{
	return FNColor::SortElement;
}
#endif

FPCGElementPtr UNRandomStepRotationSettings::CreateElement() const
{
	return MakeShared<FNRandomStepRotationElement>();
}

int32 FNRandomStepRotationElement::GetStepCount(const double StepAngle)
{
	// A non-positive step has no meaningful subdivision, and a step of a full turn or more only ever
	// lands back where it started — both leave "no rotation" as the single outcome.
	if (StepAngle <= UE_DOUBLE_SMALL_NUMBER || StepAngle >= 360.0)
	{
		return 1;
	}

	// 60 degrees yields six outcomes: no rotation, plus 60 through 300. The 360 step is the same
	// orientation as no rotation, so it is deliberately not counted a second time. A step that does not
	// divide 360 evenly drops its final partial step rather than emitting an uneven one.
	return FMath::FloorToInt32(360.0 / StepAngle);
}

FQuat FNRandomStepRotationElement::GetStepRotation(const int32 StepIndex, const double StepAngle, const ENAxis Axis)
{
	FVector AxisVector;
	switch (Axis)
	{
		case ENAxis::X:
			AxisVector = FVector::XAxisVector;
			break;
		case ENAxis::Y:
			AxisVector = FVector::YAxisVector;
			break;
		case ENAxis::Z:
			AxisVector = FVector::ZAxisVector;
			break;
		default:
			// ENAxis carries a None; it has to mean "leave the point alone" rather than quietly falling
			// through to one of the real axes and rotating anyway.
			return FQuat::Identity;
	}

	return FQuat(AxisVector, FMath::DegreesToRadians(StepIndex * StepAngle));
}

bool FNRandomStepRotationElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FNRandomStepRotationElement::Execute);
	check(Context);

	const UNRandomStepRotationSettings* Settings = Context->GetInputSettings<UNRandomStepRotationSettings>();
	check(Settings);

	ContextType* RotationContext = static_cast<ContextType*>(Context);

	const int Seed = Context->GetSeed();
	const double StepAngle = Settings->StepAngle;
	const int32 StepCount = GetStepCount(StepAngle);
	const ENAxis Axis = Settings->Axis;
	const bool bLocalSpace = Settings->RotationSpace == ENRotationSpace::Local;

	// A single outcome, or no axis to turn around, means every point stays put — skip the per-point work.
	if (StepCount <= 1 || Axis == ENAxis::None)
	{
		Context->OutputData = Context->InputData;
		return true;
	}

	return ExecutePointOperation(RotationContext, [Seed, StepAngle, StepCount, Axis, bLocalSpace](const UPCGBasePointData* InputData, UPCGBasePointData* OutputData, const int32 StartIndex, const int32 Count)
	{
		const TConstPCGValueRange<int32> SeedRange = InputData->GetConstSeedValueRange();
		TPCGValueRange<FTransform> TransformRange = OutputData->GetTransformValueRange();

		for (int32 Index = StartIndex; Index < (StartIndex + Count); ++Index)
		{
			// Fold in the point's own seed rather than drawing from a running stream, so a point keeps the
			// same step across regenerations and however the work happens to be chunked across threads.
			FRandomStream RandomSource(PCGHelpers::ComputeSeed(Seed, SeedRange[Index]));
			const int32 StepIndex = RandomSource.RandRange(0, StepCount - 1);
			if (StepIndex == 0)
			{
				continue;
			}

			FTransform& Transform = TransformRange[Index];
			const FQuat StepRotation = GetStepRotation(StepIndex, StepAngle, Axis);

			// Local composes on the right so the turn follows the point's own axis; world composes on the
			// left so it follows the world axis. The two only diverge once a point is already tilted.
			const FQuat Rotation = bLocalSpace
				? Transform.GetRotation() * StepRotation
				: StepRotation * Transform.GetRotation();
			Transform.SetRotation(Rotation.GetNormalized());
		}

		return true;
	});
}
