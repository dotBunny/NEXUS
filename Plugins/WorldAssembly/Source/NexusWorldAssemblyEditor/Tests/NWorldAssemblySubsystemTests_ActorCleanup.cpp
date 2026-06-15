// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWorldAssemblySubsystem.h"
#include "Developer/NDebugActor.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(UNWorldAssemblySubsystemTests_ActorCleanup_DestroyActorsForOperationDestroysOnlyMatchingTicket,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblySubsystem::ActorCleanup::DestroyActorsForOperationDestroysOnlyMatchingTicket",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that DestroyActorsForOperation tears down only the actors registered under the supplied ticket,
	// leaving actors registered under other tickets untouched.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNWorldAssemblySubsystem from PIE world.");
			return;
		}

		ANDebugActor* TicketAActorOne = World->SpawnActor<ANDebugActor>();
		ANDebugActor* TicketAActorTwo = World->SpawnActor<ANDebugActor>();
		ANDebugActor* TicketBActor = World->SpawnActor<ANDebugActor>();
		if (TicketAActorOne == nullptr || TicketAActorTwo == nullptr || TicketBActor == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the cleanup test.");
			return;
		}

		Subsystem->RegisterOperationActor(TicketAActorOne, 100);
		Subsystem->RegisterOperationActor(TicketAActorTwo, 100);
		Subsystem->RegisterOperationActor(TicketBActor, 200);

		Subsystem->DestroyOperationActors(100);

		CHECK_FALSE_MESSAGE(TEXT("First actor under ticket 100 must be destroyed."), IsValid(TicketAActorOne))
		CHECK_FALSE_MESSAGE(TEXT("Second actor under ticket 100 must be destroyed."), IsValid(TicketAActorTwo))
		CHECK_MESSAGE(TEXT("Actor under ticket 200 must be left untouched."), IsValid(TicketBActor))
	});
}

N_TEST_MEDIUM(UNWorldAssemblySubsystemTests_ActorCleanup_DestroyActorsForOperationUnknownTicketIsNoOp,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblySubsystem::ActorCleanup::DestroyActorsForOperationUnknownTicketIsNoOp",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that destroying a ticket with nothing registered is a no-op and does not touch other tracked actors.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNWorldAssemblySubsystem from PIE world.");
			return;
		}

		ANDebugActor* Actor = World->SpawnActor<ANDebugActor>();
		if (Actor == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actor for the cleanup test.");
			return;
		}

		Subsystem->RegisterOperationActor(Actor, 100);
		Subsystem->DestroyOperationActors(999);

		CHECK_MESSAGE(TEXT("An unknown ticket must be a no-op, leaving the registered actor alive."), IsValid(Actor))
	});
}

N_TEST_HIGH(UNWorldAssemblySubsystemTests_ActorCleanup_ClearDestroysAllTickets,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblySubsystem::ActorCleanup::ClearDestroysAllTickets",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that Clear destroys tracked actors across every ticket bucket, not just one.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNWorldAssemblySubsystem from PIE world.");
			return;
		}

		ANDebugActor* ActorA = World->SpawnActor<ANDebugActor>();
		ANDebugActor* ActorB = World->SpawnActor<ANDebugActor>();
		if (ActorA == nullptr || ActorB == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the cleanup test.");
			return;
		}

		Subsystem->RegisterOperationActor(ActorA, 100);
		Subsystem->RegisterOperationActor(ActorB, 200);

		Subsystem->Clear();

		CHECK_FALSE_MESSAGE(TEXT("Actor under ticket 100 must be destroyed by Clear."), IsValid(ActorA))
		CHECK_FALSE_MESSAGE(TEXT("Actor under ticket 200 must be destroyed by Clear."), IsValid(ActorB))
	});
}

N_TEST_MEDIUM(UNWorldAssemblySubsystemTests_ActorCleanup_UnregisterActorForCleanupStopsTracking,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblySubsystem::ActorCleanup::UnregisterActorForCleanupStopsTracking",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies that an unregistered actor is no longer destroyed by its former ticket's teardown.
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNWorldAssemblySubsystem from PIE world.");
			return;
		}

		ANDebugActor* Actor = World->SpawnActor<ANDebugActor>();
		if (Actor == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actor for the cleanup test.");
			return;
		}

		Subsystem->RegisterOperationActor(Actor, 100);
		Subsystem->UnregisterOperationActor(Actor);
		Subsystem->DestroyOperationActors(100);

		CHECK_MESSAGE(TEXT("An unregistered actor must survive its former ticket's teardown."), IsValid(Actor))
	});
}

N_TEST_MEDIUM(UNWorldAssemblySubsystemTests_ActorCleanup_UnregisterActorForCleanupFromOperationStopsTracking,
	"NEXUS::UnitTests::NWorldAssembly::UNWorldAssemblySubsystem::ActorCleanup::UnregisterActorForCleanupFromOperationStopsTracking",
	N_TEST_CONTEXT_EDITOR)
{
	// Verifies the ticket-aware unregister drops the actor from its bucket so the ticket's teardown skips it, while
	// passing the wrong ticket leaves the actor tracked (and still destroyed by its real ticket).
	FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
	{
		UNWorldAssemblySubsystem* Subsystem = UNWorldAssemblySubsystem::Get(World);
		if (!Subsystem)
		{
			ADD_ERROR("Could not retrieve UNWorldAssemblySubsystem from PIE world.");
			return;
		}

		ANDebugActor* MatchingActor = World->SpawnActor<ANDebugActor>();
		ANDebugActor* WrongTicketActor = World->SpawnActor<ANDebugActor>();
		if (MatchingActor == nullptr || WrongTicketActor == nullptr)
		{
			ADD_ERROR("Failed to spawn debug actors for the cleanup test.");
			return;
		}

		Subsystem->RegisterOperationActor(MatchingActor, 100);
		Subsystem->RegisterOperationActor(WrongTicketActor, 100);

		// Correct ticket removes the actor; wrong ticket is a no-op so that actor stays tracked under 100.
		Subsystem->UnregisterOperationActorByTicket(MatchingActor, 100);
		Subsystem->UnregisterOperationActorByTicket(WrongTicketActor, 200);

		Subsystem->DestroyOperationActors(100);

		CHECK_MESSAGE(TEXT("An actor unregistered from its ticket must survive that ticket's teardown."), IsValid(MatchingActor))
		CHECK_FALSE_MESSAGE(TEXT("An actor unregistered from the wrong ticket must still be destroyed by its real ticket."), IsValid(WrongTicketActor))
	});
}

#endif //WITH_TESTS
