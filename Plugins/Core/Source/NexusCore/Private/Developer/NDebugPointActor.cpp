// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NDebugPointActor.h"

ANDebugPointActor::ANDebugPointActor()
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

ANDebugPointActor* ANDebugPointActor::CreateInstance(UWorld* World, const FVector& Position, const FRotator& Rotation, const FString& Label)
{
	FActorSpawnParameters SpawnInfo;
	
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.InitialActorLabel = Label; 
	
	return World->SpawnActor<ANDebugPointActor>(Position, Rotation, SpawnInfo);
}
