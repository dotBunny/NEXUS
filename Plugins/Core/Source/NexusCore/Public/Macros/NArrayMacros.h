// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_3D_ARRAY(IndexType, DataType, DataArray, SizeX, SizeY, SizeZ) \
public: \
	void Reset(const uint32 InSizeX, const uint32 InSizeY, const uint32 InSizeZ) \
	{ \
		SizeX = InSizeX; \
		SizeY = InSizeY; \
		SizeZ = InSizeZ; \
		Data.Empty(); \
		Data.SetNum(SizeX * SizeY * SizeZ); \
	} \
	\
	FORCEINLINE IndexType GetIndex(const int32 X, const int32 Y, const int32 Z) const \
	{ \
		return X + (Y * SizeX) + (Z * SizeX * SizeY); \
	} \
	FORCEINLINE IndexType GetIndex(const uint32 X, const uint32 Y, const uint32 Z) const \
	{ \
		return X + (Y * SizeX) + (Z * SizeX * SizeY); \
	} \
	FORCEINLINE IndexType GetIndex(const FVector& Index) const \
	{ \
		return FMath::FloorToInt(Index.X) + (FMath::FloorToInt(Index.Y) * SizeX) + (FMath::FloorToInt(Index.Z) * SizeX * SizeY); \
	} \
	FORCEINLINE IndexType GetIndex(const FIntVector3& Index) const \
	{ \
		return Index.X + (Index.Y * SizeX) + (Index.Z * SizeX * SizeY); \
	} \
	\
	FORCEINLINE DataType GetData(const int32 X, const int32 Y, const int32 Z) const \
	{ \
		return DataArray[GetIndex(X, Y, Z)]; \
	} \
	FORCEINLINE DataType GetData(const uint32 X, const uint32 Y, const uint32 Z) const \
	{ \
		return DataArray[GetIndex(X, Y, Z)]; \
	} \
	FORCEINLINE DataType GetData(const FVector& Index) const \
	{ \
		return DataArray[GetIndex(Index)]; \
	} \
	FORCEINLINE DataType GetData(const FIntVector3& Index) const \
	{ \
		return DataArray[GetIndex(Index)]; \
	} \
	FORCEINLINE DataType GetData(const IndexType& Index) const \
	{ \
		return DataArray[Index]; \
	} \
	\
	FORCEINLINE void SetData(const int32 X, const int32 Y, const int32 Z, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(X, Y, Z)] = NewValue; \
	} \
	FORCEINLINE void SetData(const uint32 X, const uint32 Y, const uint32 Z, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(X, Y, Z)] = NewValue; \
	} \
	FORCEINLINE void SetData(const FVector& Index, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(Index)] = NewValue; \
	} \
	FORCEINLINE void SetData(const FIntVector3& Index, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(Index)] = NewValue; \
	} \
	FORCEINLINE void SetData(const IndexType& Index, const DataType& NewValue) \
	{ \
		DataArray[Index] = NewValue; \
	}