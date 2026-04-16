// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NDebugActor.h"

ANDebugActor::ANDebugActor()
{
	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = SphereMesh;
	
	// Load the engine's default sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		SphereMesh->SetStaticMesh(SphereMeshAsset.Object);
	}
}

ANDebugActor* ANDebugActor::CreateInstance(UWorld* World, const FVector& Position, const FRotator& Rotation, const FString& Label, const FVector& Scale)
{
	FActorSpawnParameters SpawnInfo;
	
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR	
	SpawnInfo.InitialActorLabel = Label; 
#endif // WITH_EDITOR

	const FTransform SpawnTransform(Rotation, Position, Scale);
	return World->SpawnActor<ANDebugActor>(StaticClass(), SpawnTransform, SpawnInfo);
}
