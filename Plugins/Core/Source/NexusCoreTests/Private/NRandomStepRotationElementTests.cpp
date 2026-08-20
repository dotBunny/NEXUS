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

#endif //WITH_TESTS
