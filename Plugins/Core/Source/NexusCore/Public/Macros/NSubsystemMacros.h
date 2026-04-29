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
 * Variant of N_WORLD_SUBSYSTEM that restricts subsystem creation to Game/PIE worlds only.
 *
 * Also guards creation with a ShouldCreate flag so a setting or preprocessor constant can suppress
 * the subsystem entirely. Two arms exist for editor/non-editor to match the differing world-type
 * checks required in each configuration.
 *
 * @param Type The concrete subsystem class.
 * @param ShouldCreate Boolean expression evaluated inside ShouldCreateSubsystem; false skips creation.
 */
#if WITH_EDITOR
#define N_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate) \
	public: \
		FORCEINLINE static Type* Get(const UWorld* World) { \
			return World->GetSubsystem<Type>(); \
		} \
		FORCEINLINE static Type* Get(UWorld& World) { \
			return World.GetSubsystem<Type>(); \
		} \
		virtual bool ShouldCreateSubsystem(UObject* Outer) const override { \
			if(!ShouldCreate) return false; \
			if (Outer && Super::ShouldCreateSubsystem(Outer)) { \
				if (UWorld* World = Outer->GetWorld()) { \
					if ((World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE ) \
						&& World->GetSubsystemBase(GetClass()) == nullptr) { \
						return true; \
					} \
				} \
			} \
			return false; \
		}
#else // !WITH_EDITOR
#define N_WORLD_SUBSYSTEM_GAME_ONLY(Type, ShouldCreate) \
	public: \
		FORCEINLINE static Type* Get(const UWorld* World) { \
			return World->GetSubsystem<Type>(); \
		} \
		FORCEINLINE static Type* Get(UWorld& World) { \
			return World.GetSubsystem<Type>(); \
		} \
		virtual bool ShouldCreateSubsystem(UObject* Outer) const override { \
			if(!ShouldCreate) return false; \
			if (Super::ShouldCreateSubsystem(Outer) && Outer) { \
				if (UWorld* World = Outer->GetWorld()) { \
					return (World->GetSubsystemBase(GetClass()) == nullptr); \
				} \
			} \
			return true; \
		}
#endif // WITH_EDITOR

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
 * creation to Game/PIE worlds. Pair with N_TICKABLE_WORLD_SUBSYSTEM_GET_TICKABLE_TICK_TYPE to
 * finish wiring the subsystem's tick behavior.
 *
 * @param Type The concrete tickable subsystem class.
 * @param ShouldCreate Boolean expression that gates subsystem creation.
 */
#if WITH_EDITOR
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
		virtual bool ShouldCreateSubsystem(UObject* Outer) const override { \
			if(!ShouldCreate) return false; \
			if (Outer && Super::ShouldCreateSubsystem(Outer)) { \
				if (UWorld* World = Outer->GetWorld()) { \
				if ((World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE ) \
					&& World->GetSubsystemBase(GetClass()) == nullptr) { \
						return true; \
					} \
				} \
			} \
			return false; \
		}
#else // !WITH_EDITOR
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
		virtual bool ShouldCreateSubsystem(UObject* Outer) const override { \
			if(!ShouldCreate) return false; \
			if (Super::ShouldCreateSubsystem(Outer) && Outer) { \
				if (UWorld* World = Outer->GetWorld()) { \
					return (World->GetSubsystemBase(GetClass()) == nullptr); \
				} \
			} \
			return true; \
		}		
#endif // WITH_EDITOR
