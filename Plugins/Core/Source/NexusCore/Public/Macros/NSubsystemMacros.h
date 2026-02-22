// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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
