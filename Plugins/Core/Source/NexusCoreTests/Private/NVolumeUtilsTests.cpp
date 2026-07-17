// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NVolumeUtils.h"
#include "DynamicMeshBuilder.h"
#include "Macros/NTestMacros.h"
#include "Model.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(FNVolumeUtilsTests_FillGeometryData_NullModel_NoOp,
	"NEXUS::UnitTests::NCore::FNVolumeUtils::FillGeometryData::NullModel_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Pre-populate OutData so we can prove the early-out at NVolumeUtils.cpp:10 doesn't touch it. The
	// buffer Empty() calls only happen after the null check, so a sentinel index must survive.
	FNVolumeGeometryData OutData;
	OutData.Indices.Add(42);

	FNVolumeUtils::FillGeometryData(nullptr, OutData);

	CHECK_EQUALS("Null model must leave pre-existing Indices untouched (early-out before Empty())", OutData.Indices.Num(), 1);
	CHECK_EQUALS("Null model sentinel index value must survive unchanged", OutData.Indices[0], static_cast<uint32>(42));
}

N_TEST_HIGH(FNVolumeUtilsTests_FillGeometryData_EmptyModel_ClearsAndProducesNothing,
	"NEXUS::UnitTests::NCore::FNVolumeUtils::FillGeometryData::EmptyModel_ClearsAndProducesNothing",
	N_TEST_CONTEXT_EDITOR)
{
	// A freshly constructed UModel with no Nodes — the function should clear the output buffers and the
	// node loop iterates zero times. Proves the buffer-clear path runs and an empty model is handled
	// without crashing on the unguarded Model->Nodes access.
	UModel* Model = NewObject<UModel>();
	if (Model == nullptr)
	{
		ADD_ERROR("Failed to construct UModel via NewObject");
		return;
	}

	FNVolumeGeometryData OutData;
	OutData.Indices.Add(99);  // Pre-populate to verify the clear actually runs.

	FNVolumeUtils::FillGeometryData(Model, OutData);

	CHECK_EQUALS("Empty model must clear Indices on entry", OutData.Indices.Num(), 0);
	CHECK_EQUALS("Empty model must clear Vertices on entry", OutData.Vertices.Num(), 0);
}

#endif //WITH_TESTS
