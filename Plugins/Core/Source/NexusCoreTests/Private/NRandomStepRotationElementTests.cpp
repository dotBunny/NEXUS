// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NTestMacros.h"
#include "PCG/Elements/NRandomStepRotationElement.h"

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepCount_CountsWholeStepsInAFullTurn,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepCount::CountsWholeStepsInAFullTurn",
	N_TEST_CONTEXT_ANYWHERE)
{
	// 60 is the worked example: no rotation, plus 60, 120, 180, 240 and 300.
	CHECK_EQUALS("A 60 degree step should give six outcomes.", FNRandomStepRotationElement::GetStepCount(60.0), 6);
	CHECK_EQUALS("A 90 degree step should give four outcomes.", FNRandomStepRotationElement::GetStepCount(90.0), 4);
	CHECK_EQUALS("A 180 degree step should give two outcomes.", FNRandomStepRotationElement::GetStepCount(180.0), 2);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepCount_FullTurnLeavesASingleOutcome,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepCount::FullTurnLeavesASingleOutcome",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A 360 step only ever lands back on the unrotated orientation, so it must not read as two outcomes.
	CHECK_EQUALS("A 360 degree step should give one outcome.", FNRandomStepRotationElement::GetStepCount(360.0), 1);
	CHECK_EQUALS("A step past a full turn should give one outcome.", FNRandomStepRotationElement::GetStepCount(400.0), 1);
}

N_TEST_MEDIUM(FNRandomStepRotationElementTests_GetStepCount_NonPositiveStepLeavesASingleOutcome,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepCount::NonPositiveStepLeavesASingleOutcome",
	N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_EQUALS("A zero step should give one outcome.", FNRandomStepRotationElement::GetStepCount(0.0), 1);
	CHECK_EQUALS("A negative step should give one outcome.", FNRandomStepRotationElement::GetStepCount(-90.0), 1);
}

N_TEST_MEDIUM(FNRandomStepRotationElementTests_GetStepCount_UnevenStepDropsThePartialStep,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepCount::UnevenStepDropsThePartialStep",
	N_TEST_CONTEXT_ANYWHERE)
{
	// 50 fits seven whole times into 360 with 10 degrees left over; the leftover must not become an outcome.
	CHECK_EQUALS("A 50 degree step should give seven outcomes.", FNRandomStepRotationElement::GetStepCount(50.0), 7);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepRotation_StepZeroLeavesThePointUnrotated,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::StepZeroLeavesThePointUnrotated",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(0, 90.0, ENAxis::Z);
	CHECK_EQUALS("Step zero should not move the point.", StepRotation.RotateVector(FVector::ForwardVector), FVector::ForwardVector);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepRotation_TurnsAroundZ,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::TurnsAroundZ",
	N_TEST_CONTEXT_ANYWHERE)
{
	// A single 90 degree yaw takes forward round to right.
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(1, 90.0, ENAxis::Z);
	CHECK_EQUALS("A 90 degree Z step should take forward to right.", StepRotation.RotateVector(FVector::ForwardVector), FVector::RightVector);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepRotation_MultipliesTheStepAngle,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::MultipliesTheStepAngle",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Step 3 of a 60 degree turn is 180 degrees, which reverses forward.
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(3, 60.0, ENAxis::Z);
	CHECK_EQUALS("Three 60 degree steps should reverse forward.", StepRotation.RotateVector(FVector::ForwardVector), -FVector::ForwardVector);
}

N_TEST_MEDIUM(FNRandomStepRotationElementTests_GetStepRotation_FullTurnReturnsToStart,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::FullTurnReturnsToStart",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(4, 90.0, ENAxis::Z);
	CHECK_EQUALS("Four 90 degree steps should land back on the starting orientation.", StepRotation.RotateVector(FVector::ForwardVector), FVector::ForwardVector);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepRotation_HonorsTheSelectedAxis,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::HonorsTheSelectedAxis",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Each axis leaves its own direction alone and moves the other two, which is what pins the axis choice.
	const FQuat AroundX = FNRandomStepRotationElement::GetStepRotation(1, 90.0, ENAxis::X);
	CHECK_EQUALS("An X step should leave forward alone.", AroundX.RotateVector(FVector::ForwardVector), FVector::ForwardVector);
	CHECK_EQUALS("An X step should take right to up.", AroundX.RotateVector(FVector::RightVector), FVector::UpVector);

	const FQuat AroundY = FNRandomStepRotationElement::GetStepRotation(1, 90.0, ENAxis::Y);
	CHECK_EQUALS("A Y step should leave right alone.", AroundY.RotateVector(FVector::RightVector), FVector::RightVector);
	CHECK_EQUALS("A Y step should take up to forward.", AroundY.RotateVector(FVector::UpVector), FVector::ForwardVector);

	const FQuat AroundZ = FNRandomStepRotationElement::GetStepRotation(1, 90.0, ENAxis::Z);
	CHECK_EQUALS("A Z step should leave up alone.", AroundZ.RotateVector(FVector::UpVector), FVector::UpVector);
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetStepRotation_NoAxisLeavesThePointUnrotated,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetStepRotation::NoAxisLeavesThePointUnrotated",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Sharing ENAxis with the rest of NEXUS brings a None along; it must not fall through to a real axis
	// and rotate anyway, which is exactly what a switch default would have done.
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(2, 90.0, ENAxis::None);
	CHECK_EQUALS("No axis should leave forward alone.", StepRotation.RotateVector(FVector::ForwardVector), FVector::ForwardVector);
	CHECK_EQUALS("No axis should leave right alone.", StepRotation.RotateVector(FVector::RightVector), FVector::RightVector);
	CHECK_EQUALS("No axis should leave up alone.", StepRotation.RotateVector(FVector::UpVector), FVector::UpVector);
}

N_TEST_CRITICAL(FNRandomStepRotationElementTests_GetPointRotation_BaseSquaresTheMeshUpBeforeTheStep,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetPointRotation::BaseSquaresTheMeshUpBeforeTheStep",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The reason the base exists, and the reason it has to run first. A mesh modelled lying along X needs
	// standing up before a Z step spins it about its own length; run the base after the step instead and
	// it would be turning whatever the step had just done rather than the mesh.
	const FQuat StandUp = FRotator(90.0, 0.0, 0.0).Quaternion();
	const FQuat QuarterTurn = FNRandomStepRotationElement::GetStepRotation(1, 90.0, ENAxis::Z);

	const FQuat Rotation = FNRandomStepRotationElement::GetPointRotation(FQuat::Identity, QuarterTurn, StandUp, /*bLocalSpace=*/true);

	// Forward was the mesh's length; standing it up should leave it pointing along world up, and a spin
	// about Z after that turns it about itself rather than laying it back down.
	CHECK_MESSAGE(TEXT("The base should stand the mesh up before the step spins it."),
		Rotation.RotateVector(FVector::ForwardVector).Equals(FVector::UpVector, UE_KINDA_SMALL_NUMBER));
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetPointRotation_BaseAppliesWithNoStepAtAll,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetPointRotation::BaseAppliesWithNoStepAtAll",
	N_TEST_CONTEXT_ANYWHERE)
{
	// "Always applied" is the whole promise. A point that drew step zero, or a node whose step does
	// nothing at all, still has to come out squared up.
	const FQuat Base = FRotator(0.0, 30.0, 0.0).Quaternion();
	const FQuat Rotation = FNRandomStepRotationElement::GetPointRotation(FQuat::Identity, FQuat::Identity, Base, true);

	CHECK_MESSAGE(TEXT("The base alone should still turn the point."), Rotation.Equals(Base, UE_KINDA_SMALL_NUMBER));
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetPointRotation_NoBaseLeavesTheOldBehaviourExactly,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetPointRotation::NoBaseLeavesTheOldBehaviourExactly",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Adding the setting must not have moved anything for a graph that never touches it, in either space.
	const FQuat PointRotation = FRotator(20.0, 35.0, 10.0).Quaternion();
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(2, 60.0, ENAxis::Z);

	const FQuat Local = FNRandomStepRotationElement::GetPointRotation(PointRotation, StepRotation, FQuat::Identity, /*bLocalSpace=*/true);
	const FQuat World = FNRandomStepRotationElement::GetPointRotation(PointRotation, StepRotation, FQuat::Identity, /*bLocalSpace=*/false);

	CHECK_MESSAGE(TEXT("Local with no base should compose the step on the right."),
		Local.Equals((PointRotation * StepRotation).GetNormalized(), UE_KINDA_SMALL_NUMBER));
	CHECK_MESSAGE(TEXT("World with no base should compose the step on the left."),
		World.Equals((StepRotation * PointRotation).GetNormalized(), UE_KINDA_SMALL_NUMBER));
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetPointRotation_BaseStaysInnermostInBothSpaces,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetPointRotation::BaseStaysInnermostInBothSpaces",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The base is a property of the mesh, not of the space the step is taken in, so switching between
	// local and world must not change what squaring up means.
	const FQuat PointRotation = FRotator(15.0, 40.0, 0.0).Quaternion();
	const FQuat Base = FRotator(90.0, 0.0, 0.0).Quaternion();

	const FQuat Local = FNRandomStepRotationElement::GetPointRotation(PointRotation, FQuat::Identity, Base, true);
	const FQuat World = FNRandomStepRotationElement::GetPointRotation(PointRotation, FQuat::Identity, Base, false);

	// With no step to take, the two spaces have nothing to disagree about and both are point times base.
	CHECK_MESSAGE(TEXT("Local should apply the base inside the point's rotation."),
		Local.Equals((PointRotation * Base).GetNormalized(), UE_KINDA_SMALL_NUMBER));
	CHECK_MESSAGE(TEXT("World should apply the base inside the point's rotation too."),
		World.Equals((PointRotation * Base).GetNormalized(), UE_KINDA_SMALL_NUMBER));
}

N_TEST_HIGH(FNRandomStepRotationElementTests_GetPointRotation_ReturnsAUnitRotation,
	"NEXUS::UnitTests::NCore::FNRandomStepRotationElement::GetPointRotation::ReturnsAUnitRotation",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Three quaternions multiplied together drift, and a spawner handed a non-unit rotation will scale
	// the mesh it places by whatever the drift amounted to.
	const FQuat PointRotation = FRotator(23.0, 47.0, 11.0).Quaternion();
	const FQuat Base = FRotator(37.0, 12.0, 5.0).Quaternion();
	const FQuat StepRotation = FNRandomStepRotationElement::GetStepRotation(3, 45.0, ENAxis::Y);

	CHECK_MESSAGE(TEXT("Local should come back normalized."),
		FNRandomStepRotationElement::GetPointRotation(PointRotation, StepRotation, Base, true).IsNormalized());
	CHECK_MESSAGE(TEXT("World should come back normalized."),
		FNRandomStepRotationElement::GetPointRotation(PointRotation, StepRotation, Base, false).IsNormalized());
}

#endif //WITH_TESTS
