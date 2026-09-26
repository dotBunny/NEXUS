// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Injects the standard NEXUS UWorldSubsystem boilerplate (static Get helpers + tick stat id).
 *
 * Use in the body of a UWorldSubsystem-derived class to expose strongly-typed Get(World) accessors
 * and to supply the tick stat id. For tickable subsystems, see N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY.
 *
 * @param Type The concrete subsystem class itself (used as template argument to GetSubsystem).
 * @note Get(const UWorld*) dereferences World without a null check for speed; callers must pass a valid,
 *       non-null world. Resolve a possibly-expired weak context to a non-null world before calling.
 */
#define N_WORLD_SUBSYSTEM(Type) \
	public: \
		FORCEINLINE static Type* Get(const UWorld* World) { \
			return World->GetSubsystem<Type>(); \
		} \
		FORCEINLINE static Type* Get(UWorld& World) { \
			return World.GetSubsystem<Type>(); \
		} \
		virtual TStatId GetStatId() const final override { \
			RETURN_QUICK_DECLARE_CYCLE_STAT(Type, STATGROUP_Tickables) \
		}

/**
 * Supplies the ShouldCreateSubsystem override shared by N_WORLD_SUBSYSTEM_GAME_ONLY and
 * N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY.
 *
 * Creates the subsystem only when ShouldCreate holds, the base class agrees, the world is a Game or PIE world, and
 * the world does not already hold an instance of the class (or of a subclass). There is one definition for editor and
 * non-editor builds alike, so the behavior the editor's tests exercise is the behavior a cooked build ships with.
 *
 * @param ShouldCreate Boolean expression evaluated inside ShouldCreateSubsystem; false skips creation. It is
 *        evaluated as a whole, so a conditional or compound expression needs no parentheses of its own.
 */
#define N_WORLD_SUBSYSTEM_GAME_ONLY_SHOULD_CREATE(ShouldCreate) \
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { \
		if (!(ShouldCreate) || Outer == nullptr || !Super::ShouldCreateSubsystem(Outer)) { \
			return false; \
		} \
		const UWorld* World = Outer->GetWorld(); \
		return World != nullptr \
			&& (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) \
			&& World->GetSubsystemBase(GetClass()) == nullptr; \
	}

/**
 * Variant of N_WORLD_SUBSYSTEM that restricts subsystem creation to Game/PIE worlds only.
 *
 * Also guards creation with a ShouldCreate flag so a setting or preprocessor constant can suppress
 * the subsystem entirely; see N_WORLD_SUBSYSTEM_GAME_ONLY_SHOULD_CREATE.
 *
 * @param Type The concrete subsystem class.
 * @param ShouldCreate Boolean expression evaluated inside ShouldCreateSubsystem; false skips creation.
 * @note Get(const UWorld*) dereferences World without a null check for speed; callers must pass a valid,
 *       non-null world. Resolve a possibly-expired weak context to a non-null world before calling.
 */
#define N_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate) \
	public: \
		FORCEINLINE static Type* Get(const UWorld* World) { \
			return World->GetSubsystem<Type>(); \
		} \
		FORCEINLINE static Type* Get(UWorld& World) { \
			return World.GetSubsystem<Type>(); \
		} \
		N_WORLD_SUBSYSTEM_GAME_ONLY_SHOULD_CREATE(ShouldCreate)

/**
 * Supplies a GetTickableTickType() override that returns DefaultType for real instances and
 * Never for CDOs, the standard NEXUS pattern for tickable subsystems.
 *
 * @param DefaultType ETickableTickType value returned for live (non-CDO) instances.
 */
#define N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE(DefaultType) \
	virtual ETickableTickType GetTickableTickType() const override \
	{ \
		if (HasAnyFlags(RF_ClassDefaultObject)) return ETickableTickType::Never; \
		return DefaultType; \
	} \

/**
 * Tickable-world-subsystem variant of N_WORLD_SUBSYSTEM_GAME_ONLY.
 *
 * Injects Get accessors, a tick stat id, and a ShouldCreateSubsystem implementation that limits
 * creation to Game/PIE worlds (see N_WORLD_SUBSYSTEM_GAME_ONLY_SHOULD_CREATE). Pair with
 * N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE to finish wiring the subsystem's tick behavior.
 *
 * @param Type The concrete tickable subsystem class.
 * @param ShouldCreate Boolean expression that gates subsystem creation.
 * @note Get(const UWorld*) dereferences World without a null check for speed; callers must pass a valid,
 *       non-null world. Resolve a possibly-expired weak context to a non-null world before calling.
 */
#define N_TICKABLE_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate) \
	public: \
		FORCEINLINE static Type* Get(const UWorld* World) { \
			return World->GetSubsystem<Type>(); \
		} \
		FORCEINLINE static Type* Get(UWorld& World) { \
			return World.GetSubsystem<Type>(); \
		} \
		virtual TStatId GetStatId() const final override { \
			RETURN_QUICK_DECLARE_CYCLE_STAT(Type, STATGROUP_Tickables) \
		} \
		N_WORLD_SUBSYSTEM_GAME_ONLY_SHOULD_CREATE(ShouldCreate)
