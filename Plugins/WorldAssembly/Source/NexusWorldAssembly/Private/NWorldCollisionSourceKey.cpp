// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldCollisionSourceKey.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Math/NHashUtils.h"
#include "Types/NRawMeshFactory.h"

uint64 FNWorldCollisionSourceKey::FromSource(const FNRawMeshSource& Source)
{
	return FromComponent(Source.Component, Source.InstanceIndex, Source.ElementOrdinal);
}

uint64 FNWorldCollisionSourceKey::FromComponent(const UPrimitiveComponent* Component, const int32 InstanceIndex,
	const int32 ElementOrdinal)
{
	if (Component == nullptr) return 0;

	const uint64 ActorKey = FromActor(Component->GetOwner());
	if (ActorKey == 0) return 0;

	uint64 Key = Combine(ActorKey, HashName(Component->GetFName()));
	Key = Combine(Key, Mix(static_cast<uint64>(static_cast<uint32>(InstanceIndex))));
	Key = Combine(Key, Mix(static_cast<uint64>(static_cast<uint32>(ElementOrdinal))));

	// A key of zero is the system's "no identity" sentinel, so a legitimate element must never produce one. The odds
	// are 1 in 2^64, but a silent collision with the sentinel would drop that element from every pool it belongs to.
	return Key == 0 ? 1 : Key;
}

uint64 FNWorldCollisionSourceKey::FromActor(const AActor* Actor)
{
	if (Actor == nullptr) return 0;

	const ULevel* Level = Actor->GetLevel();
	if (Level == nullptr) return 0;

	const UPackage* LevelPackage = Level->GetPackage();
	if (LevelPackage == nullptr) return 0;

	return Combine(HashName(LevelPackage->GetFName()), HashName(Actor->GetFName()));
}

uint64 FNWorldCollisionSourceKey::Mix(uint64 Value)
{
	// splitmix64's finalizer. Chosen over a truncated engine hash because the inputs here are small, dense integers
	// (instance indices, element ordinals) whose low bits would otherwise dominate a combined key.
	Value += 0x9E3779B97F4A7C15ULL;
	Value = (Value ^ (Value >> 30)) * 0xBF58476D1CE4E5B9ULL;
	Value = (Value ^ (Value >> 27)) * 0x94D049BB133111EBULL;
	return Value ^ (Value >> 31);
}

uint64 FNWorldCollisionSourceKey::Combine(const uint64 Seed, const uint64 Value)
{
	return Mix(Seed ^ Mix(Value));
}

uint64 FNWorldCollisionSourceKey::HashName(const FName& Name)
{
	return HashString(Name.ToString());
}

uint64 FNWorldCollisionSourceKey::HashString(const FString& Value)
{
	return FNHashUtils::djb2(Value);
}

uint64 FNWorldCollisionSourceKey::HashGuid(const FGuid& Guid)
{
	uint64 Hash = Mix(static_cast<uint64>(Guid.A));
	Hash = Combine(Hash, static_cast<uint64>(Guid.B));
	Hash = Combine(Hash, static_cast<uint64>(Guid.C));
	Hash = Combine(Hash, static_cast<uint64>(Guid.D));
	return Hash;
}

uint64 FNWorldCollisionSourceKey::HashVector(const FVector& Vector)
{
	using namespace NEXUS::WorldAssembly::CollisionKey;

	uint64 Hash = Mix(static_cast<uint64>(FMath::RoundToInt64(Vector.X * LocationQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Vector.Y * LocationQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Vector.Z * LocationQuantum)));
	return Hash;
}

uint64 FNWorldCollisionSourceKey::HashTransform(const FTransform& Transform)
{
	using namespace NEXUS::WorldAssembly::CollisionKey;

	uint64 Hash = HashVector(Transform.GetLocation());

	// Normalized so that a quaternion and its negation — the same orientation, reached by different arithmetic —
	// cannot hash apart and force a needless rebake.
	const FQuat Rotation = Transform.GetRotation().GetNormalized();
	const double Sign = (Rotation.W < 0.0) ? -1.0 : 1.0;
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Rotation.X * Sign * RotationQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Rotation.Y * Sign * RotationQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Rotation.Z * Sign * RotationQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Rotation.W * Sign * RotationQuantum)));

	const FVector Scale = Transform.GetScale3D();
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Scale.X * ScaleQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Scale.Y * ScaleQuantum)));
	Hash = Combine(Hash, static_cast<uint64>(FMath::RoundToInt64(Scale.Z * ScaleQuantum)));

	return Hash;
}
