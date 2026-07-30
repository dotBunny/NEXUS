// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NRawMeshLoop.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNRawMeshLoopTests_Ctor_Default_EmptyIndices,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Ctor::Default_EmptyIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop;
	CHECK_EQUALS("Default-constructed loop should have zero indices", Loop.Indices.Num(), 0);
}

N_TEST_HIGH(FNRawMeshLoopTests_Ctor_Triangle_StoresThreeIndicesInOrder,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Ctor::Triangle_StoresThreeIndicesInOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop(7, 8, 9);
	CHECK_EQUALS("Triangle constructor should produce 3 indices", Loop.Indices.Num(), 3);
	CHECK_EQUALS("Triangle index 0 should equal the first ctor arg", Loop.Indices[0], 7);
	CHECK_EQUALS("Triangle index 1 should equal the second ctor arg", Loop.Indices[1], 8);
	CHECK_EQUALS("Triangle index 2 should equal the third ctor arg", Loop.Indices[2], 9);
}

N_TEST_HIGH(FNRawMeshLoopTests_Ctor_Quad_StoresFourIndicesInOrder,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Ctor::Quad_StoresFourIndicesInOrder",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop(10, 11, 12, 13);
	CHECK_EQUALS("Quad constructor should produce 4 indices", Loop.Indices.Num(), 4);
	CHECK_EQUALS("Quad index 0 should equal the first ctor arg", Loop.Indices[0], 10);
	CHECK_EQUALS("Quad index 1 should equal the second ctor arg", Loop.Indices[1], 11);
	CHECK_EQUALS("Quad index 2 should equal the third ctor arg", Loop.Indices[2], 12);
	CHECK_EQUALS("Quad index 3 should equal the fourth ctor arg", Loop.Indices[3], 13);
}

N_TEST_HIGH(FNRawMeshLoopTests_Ctor_Ngon_MovesIndicesIntoLoop,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Ctor::Ngon_MovesIndicesIntoLoop",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The n-gon constructor takes the array by value and moves it in — verify by checking that the indices
	// arrive intact in the expected order. A 5-vertex pentagon is the smallest case beyond Quad's reach.
	const FNRawMeshLoop Loop(TArray<int32>{20, 21, 22, 23, 24});
	CHECK_EQUALS("N-gon constructor should preserve the input length", Loop.Indices.Num(), 5);
	for (int32 i = 0; i < 5; i++)
	{
		CHECK_EQUALS("N-gon constructor should preserve index order", Loop.Indices[i], 20 + i);
	}
}

N_TEST_HIGH(FNRawMeshLoopTests_IsTriangle_True_ForThreeIndices,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::IsTriangle::True_ForThreeIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop(0, 1, 2);
	CHECK_MESSAGE(TEXT("3-index loop must report IsTriangle()==true"), Loop.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("3-index loop must report IsQuad()==false"), Loop.IsQuad());
	CHECK_FALSE_MESSAGE(TEXT("3-index loop must report IsNgon()==false"), Loop.IsNgon());
}

N_TEST_HIGH(FNRawMeshLoopTests_IsQuad_True_ForFourIndices,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::IsQuad::True_ForFourIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop(0, 1, 2, 3);
	CHECK_MESSAGE(TEXT("4-index loop must report IsQuad()==true"), Loop.IsQuad());
	CHECK_FALSE_MESSAGE(TEXT("4-index loop must report IsTriangle()==false"), Loop.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("4-index loop must report IsNgon()==false (n-gon = strictly more than 4)"), Loop.IsNgon());
}

N_TEST_HIGH(FNRawMeshLoopTests_IsNgon_True_ForFiveOrMoreIndices,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::IsNgon::True_ForFiveOrMoreIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop Loop(TArray<int32>{0, 1, 2, 3, 4});
	CHECK_MESSAGE(TEXT("5-index loop must report IsNgon()==true"), Loop.IsNgon());
	CHECK_FALSE_MESSAGE(TEXT("5-index loop must report IsTriangle()==false"), Loop.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("5-index loop must report IsQuad()==false"), Loop.IsQuad());
}

N_TEST_HIGH(FNRawMeshLoopTests_Discriminators_AllFalse_ForEmptyOrUnderTri,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Discriminators::AllFalse_ForEmptyOrUnderTri",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Empty loop and 1/2-index degenerate loops must report false on all three discriminators — otherwise
	// downstream code (e.g. ConvertToTriangles' HasNonTris guard) could treat degenerate input as either a
	// triangle or an n-gon and either skip triangulation or take the ear-clip path on garbage.
	const FNRawMeshLoop Empty;
	CHECK_FALSE_MESSAGE(TEXT("Empty loop must report IsTriangle()==false"), Empty.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("Empty loop must report IsQuad()==false"), Empty.IsQuad());
	CHECK_FALSE_MESSAGE(TEXT("Empty loop must report IsNgon()==false"), Empty.IsNgon());

	const FNRawMeshLoop OneIndex(TArray<int32>{0});
	CHECK_FALSE_MESSAGE(TEXT("Single-index loop must report IsTriangle()==false"), OneIndex.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("Single-index loop must report IsQuad()==false"), OneIndex.IsQuad());
	CHECK_FALSE_MESSAGE(TEXT("Single-index loop must report IsNgon()==false"), OneIndex.IsNgon());

	const FNRawMeshLoop TwoIndices(TArray<int32>{0, 1});
	CHECK_FALSE_MESSAGE(TEXT("Two-index loop must report IsTriangle()==false"), TwoIndices.IsTriangle());
	CHECK_FALSE_MESSAGE(TEXT("Two-index loop must report IsQuad()==false"), TwoIndices.IsQuad());
	CHECK_FALSE_MESSAGE(TEXT("Two-index loop must report IsNgon()==false"), TwoIndices.IsNgon());
}

N_TEST_HIGH(FNRawMeshLoopTests_Equality_Identical_True,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Equality::Identical_True",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FNRawMeshLoop A(5, 6, 7);
	const FNRawMeshLoop B(5, 6, 7);
	CHECK_MESSAGE(TEXT("Loops with identical indices must compare equal"), A == B);
	CHECK_FALSE_MESSAGE(TEXT("Loops with identical indices must NOT compare unequal"), A != B);
}

N_TEST_HIGH(FNRawMeshLoopTests_Equality_DifferentLengths_False,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Equality::DifferentLengths_False",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Triangle vs. quad — the IsNgon/IsQuad/IsTriangle discriminators all hinge on length, so equality
	// must also reject a length mismatch.
	const FNRawMeshLoop Tri(0, 1, 2);
	const FNRawMeshLoop Quad(0, 1, 2, 3);
	CHECK_FALSE_MESSAGE(TEXT("Loops of different lengths must not compare equal"), Tri == Quad);
	CHECK_MESSAGE(TEXT("Loops of different lengths must compare unequal"), Tri != Quad);
}

N_TEST_HIGH(FNRawMeshLoopTests_Equality_OrderSensitive,
	"NEXUS::UnitTests::NCore::FNRawMeshLoop::Equality::OrderSensitive",
	N_TEST_CONTEXT_ANYWHERE)
{
	// (0,1,2) and (0,2,1) reference the same vertex set but with opposite winding — the docstring says
	// "Exact order-sensitive equality" so reversed winding must compare unequal even though the loops
	// describe the same triangle geometrically.
	const FNRawMeshLoop CCW(0, 1, 2);
	const FNRawMeshLoop CW(0, 2, 1);
	CHECK_FALSE_MESSAGE(TEXT("Reversed-winding loops must not compare equal (equality is order-sensitive)"), CCW == CW);
}

#endif //WITH_TESTS
