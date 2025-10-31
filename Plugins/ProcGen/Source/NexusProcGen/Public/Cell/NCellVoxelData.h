// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Macros/NArrayMacros.h"

#include "NCellVoxelData.generated.h"

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENCellVoxel : uint8
{
	CVD_Empty		= 0 UMETA(DisplayName = "Empty"),
	CVD_Occupied	= 1 << 0 UMETA(DisplayName = "Occupied")
};
ENUM_CLASS_FLAGS(ENCellVoxel)

USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNCellVoxelData
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	uint32 SizeX = 0;
	
	UPROPERTY(VisibleAnywhere)
	uint32 SizeY = 0; 
	
	UPROPERTY(VisibleAnywhere)
	uint32 SizeZ = 0;
	
	UPROPERTY(VisibleAnywhere)
	TArray<uint8> Data;
	
	N_IMPLEMENT_3D_ARRAY(uint32, uint8, Data, SizeX, SizeY, SizeZ)
};