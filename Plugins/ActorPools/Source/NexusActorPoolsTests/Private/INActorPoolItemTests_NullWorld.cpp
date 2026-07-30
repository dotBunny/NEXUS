// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NTestPooledActor.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_HIGH(INActorPoolItemTests_ReturnToActorPool_NullWorld,
	"NEXUS::UnitTests::NActorPools::INActorPoolItem::ReturnToActorPool::NullWorld",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that ReturnToActorPool on an INActorPoolItem with no owning pool and a null
	// world (i.e. constructed via NewObject under the transient package rather than spawned)
	// logs a warning and returns false without dereferencing the null world.
	ANTestPooledActor* Actor = NewObject<ANTestPooledActor>(GetTransientPackage());
	if (Actor == nullptr)
	{
		ADD_ERROR("NewObject<ANTestPooledActor> returned nullptr.");
		return;
	}

	if (Actor->IsAttachedToActorPool())
	{
		ADD_ERROR("Pre-condition failed: test actor unexpectedly reports as attached to a pool.");
		return;
	}
	if (Actor->GetWorld() != nullptr)
	{
		ADD_ERROR("Pre-condition failed: test actor unexpectedly has a non-null world.");
		return;
	}

	AddExpectedMessage(TEXT("Actor provided an invalid world"), ELogVerbosity::Warning);
	const bool bResult = Actor->ReturnToActorPool();
	CHECK_EQUALS("ReturnToActorPool() must return false when the actor has no world and no owning pool.", bResult, false)

	Actor->MarkAsGarbage();
}

#endif //WITH_TESTS
