// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblySettings.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellJunctionDetails.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Engine/World.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentFillDepthHarness
{
	/** Distinct authored override depth used to tell the Override* magnitudes apart from the project SocketDepth. */
	constexpr float OverrideDepth = 37.f;

	/** @return A throwaway, unregistered junction on a freshly spawned actor carrying the given fill-depth mode and override depth. */
	static UNCellJunctionComponent* MakeJunction(UWorld* World, const ENCellJunctionFillDepthMode Mode, const float OverrideFillDepth)
	{
		ANDebugActor* Owner = World->SpawnActor<ANDebugActor>();
		UNCellJunctionComponent* Junction = NewObject<UNCellJunctionComponent>(Owner);
		Junction->Details.FillDepthMode = Mode;
		Junction->Details.OverrideFillDepth = OverrideFillDepth;
		return Junction;
	}
}

N_TEST_HIGH(FNCellJunctionDetailsTests_GetFillDepthAnchorScale_Direction,
	"NEXUS::UnitTests::NWorldAssembly::FNCellJunctionDetails::GetFillDepthAnchorScale::Direction",
	N_TEST_CONTEXT_ANYWHERE)
{
	// The scale encodes only the direction of the mode and is independent of the depth source (Default vs Override).
	using enum ENCellJunctionFillDepthMode;

	CHECK_EQUALS("DefaultForward keeps the near edge on the socket plane.", FNCellJunctionDetails::GetFillDepthAnchorScale(DefaultForward), 0.f)
	CHECK_EQUALS("OverrideForward keeps the near edge on the socket plane.", FNCellJunctionDetails::GetFillDepthAnchorScale(OverrideForward), 0.f)

	CHECK_EQUALS("DefaultBackward pulls the near edge a full depth behind.", FNCellJunctionDetails::GetFillDepthAnchorScale(DefaultBackward), -1.f)
	CHECK_EQUALS("OverrideBackward pulls the near edge a full depth behind.", FNCellJunctionDetails::GetFillDepthAnchorScale(OverrideBackward), -1.f)

	CHECK_EQUALS("DefaultCentered straddles the socket plane.", FNCellJunctionDetails::GetFillDepthAnchorScale(DefaultCentered), -0.5f)
	CHECK_EQUALS("OverrideCentered straddles the socket plane.", FNCellJunctionDetails::GetFillDepthAnchorScale(OverrideCentered), -0.5f)
}

N_TEST_HIGH(UNCellJunctionComponentTests_GetFillDepth_ModeSource,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::GetFillDepth::ModeSource",
	N_TEST_CONTEXT_EDITOR)
{
	// The Default* modes take their magnitude from the project SocketDepth; the Override* modes take the authored OverrideFillDepth.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentFillDepthHarness;
		using enum ENCellJunctionFillDepthMode;

		const float SocketDepth = UNWorldAssemblySettings::Get()->SocketDepth;

		CHECK_EQUALS("DefaultForward uses the project SocketDepth.", MakeJunction(World, DefaultForward, OverrideDepth)->GetFillDepth(), SocketDepth)
		CHECK_EQUALS("DefaultBackward uses the project SocketDepth.", MakeJunction(World, DefaultBackward, OverrideDepth)->GetFillDepth(), SocketDepth)
		CHECK_EQUALS("DefaultCentered uses the project SocketDepth.", MakeJunction(World, DefaultCentered, OverrideDepth)->GetFillDepth(), SocketDepth)

		CHECK_EQUALS("OverrideForward uses the authored OverrideFillDepth.", MakeJunction(World, OverrideForward, OverrideDepth)->GetFillDepth(), OverrideDepth)
		CHECK_EQUALS("OverrideBackward uses the authored OverrideFillDepth.", MakeJunction(World, OverrideBackward, OverrideDepth)->GetFillDepth(), OverrideDepth)
		CHECK_EQUALS("OverrideCentered uses the authored OverrideFillDepth.", MakeJunction(World, OverrideCentered, OverrideDepth)->GetFillDepth(), OverrideDepth)
	});
}

N_TEST_HIGH(UNCellJunctionComponentTests_GetFillDepthAnchor_SignedByDirection,
	"NEXUS::UnitTests::NWorldAssembly::UNCellJunctionComponent::GetFillDepthAnchor::SignedByDirection",
	N_TEST_CONTEXT_EDITOR)
{
	// The anchor is the fill depth scaled by direction (0 forward, -depth backward, -depth/2 centered) for both depth sources.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		using namespace NEXUS::UnitTests::NWorldAssembly::FNCellJunctionComponentFillDepthHarness;
		using enum ENCellJunctionFillDepthMode;

		const float SocketDepth = UNWorldAssemblySettings::Get()->SocketDepth;

		CHECK_EQUALS("DefaultForward anchors at the junction origin.", MakeJunction(World, DefaultForward, OverrideDepth)->GetFillDepthAnchor(), 0.f)
		CHECK_EQUALS("DefaultBackward anchors a full SocketDepth behind.", MakeJunction(World, DefaultBackward, OverrideDepth)->GetFillDepthAnchor(), -SocketDepth)
		CHECK_EQUALS("DefaultCentered anchors half a SocketDepth behind.", MakeJunction(World, DefaultCentered, OverrideDepth)->GetFillDepthAnchor(), -SocketDepth * 0.5f)

		CHECK_EQUALS("OverrideForward anchors at the junction origin.", MakeJunction(World, OverrideForward, OverrideDepth)->GetFillDepthAnchor(), 0.f)
		CHECK_EQUALS("OverrideBackward anchors a full OverrideFillDepth behind.", MakeJunction(World, OverrideBackward, OverrideDepth)->GetFillDepthAnchor(), -OverrideDepth)
		CHECK_EQUALS("OverrideCentered anchors half an OverrideFillDepth behind.", MakeJunction(World, OverrideCentered, OverrideDepth)->GetFillDepthAnchor(), -OverrideDepth * 0.5f)
	});
}

#endif //WITH_TESTS
