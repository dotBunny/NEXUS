// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Math/NVectorUtils.h"
#include "PCGSettings.h"
#include "Elements/PCGPointOperationElementBase.h"

#include "NRandomStepRotationElement.generated.h"

/** The frame a step rotation is applied in. */
UENUM(BlueprintType)
enum class ENRotationSpace : uint8
{
	/** Turn around the point's own axis, so a tilted point spins about its tilted axis. */
	Local	UMETA(DisplayName = "Local"),
	/** Turn around the world axis, so every point spins about the same world direction regardless of its orientation. */
	World	UMETA(DisplayName = "World")
};

/**
 * PCG settings node that adds a random whole number of fixed-size turns to each point's rotation.
 *
 * A step angle of 60 gives six equally likely outcomes — no rotation, or 60, 120, 180, 240 or 300
 * degrees — because a full 360 turn lands back on the unrotated orientation. The roll is driven by
 * each point's own seed, so points keep their orientation across regenerations.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/elements/random-step-rotation/">UNRandomStepRotationSettings</a>
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNRandomStepRotationSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Random Step Rotation"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Adds a random whole number of fixed-size turns to each point's rotation."); }
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif

	/** Exposes the Seed property, which combines with each point's own seed to pick that point's step. */
	virtual bool UseSeed() const override { return true; }

	/** Size of a single turn, in degrees. The number of outcomes is how many whole steps fit in 360 degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0", PCG_Overridable, ToolTip = "Size of a single turn, in degrees. 60 gives six outcomes: no rotation, or 60 through 300."))
	double StepAngle = 90.0;

	/** The axis the turn is applied around. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rotation", meta = (PCG_Overridable, ToolTip = "The axis the turn is applied around: X rolls, Y pitches, Z yaws. None leaves every point unrotated."))
	ENAxis Axis = ENAxis::Z;

	/** Whether the turn follows the point's own axis or the world axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rotation", meta = (PCG_Overridable, ToolTip = "Whether the turn follows the point's own axis or the world axis. These only differ once a point is already tilted off that axis."))
	ENRotationSpace RotationSpace = ENRotationSpace::Local;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNRandomStepRotationSettings.
 */
class FNRandomStepRotationElement : public FPCGPointOperationElementBase
{
public:
	/**
	 * Counts how many distinct orientations a step angle produces, including leaving the point unrotated.
	 * Pure and PCG-free so the step maths can be unit-tested directly.
	 * @param StepAngle Size of a single turn, in degrees.
	 * @return The number of outcomes; always at least one. A step angle that does not divide 360 evenly drops the final partial step.
	 */
	static NEXUSCORE_API int32 GetStepCount(double StepAngle);

	/**
	 * Builds the rotation applied for a given step.
	 * @param StepIndex How many whole steps to turn; zero leaves the point unrotated.
	 * @param StepAngle Size of a single turn, in degrees.
	 * @param Axis The axis to turn around; ENAxis::None leaves the point unrotated.
	 * @return The delta rotation to compose onto the point's existing rotation.
	 */
	static NEXUSCORE_API FQuat GetStepRotation(int32 StepIndex, double StepAngle, ENAxis Axis);

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGPointNativeProperties GetPropertiesToAllocate(FPCGContext* InContext) const override { return EPCGPointNativeProperties::Transform; }
	virtual bool ShouldCopyPoints() const override { return true; }
	virtual bool SupportsBasePointDataInputs(FPCGContext* InContext) const override { return true; }
};
