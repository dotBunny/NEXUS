// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_FLAT_2D_ARRAY(IndexType, DataType, DataArray, SizeX, SizeY) \
public: \
	void Reset(const IndexType InSizeX, const IndexType InSizeY) \
	{ \
		SizeX = InSizeX; \
		SizeY = InSizeY; \
		Data.Empty(); \
		Data.SetNum(SizeX * SizeY); \
	} \
	void Resize(const IndexType NewSizeX, const IndexType NewSizeY) \
	{ \
		TArray<DataType> NewData; \
		NewData.Empty(); \
		NewData.SetNum(NewSizeX * NewSizeY * NewSizeZ); \
		for (IndexType x = 0; x < SizeX; x++) \
		{ \
			if (x >= NewSizeX) continue; \
			for (IndexType y = 0; y < SizeY; y++) \
			{ \
				if (y >= NewSizeY) continue; \
				NewData[x + (y * NewSizeX)] = MoveTemp(Data[GetIndex(x, y)]); \
			} \
		} \
		SizeX = NewSizeX; \
		SizeY = NewSizeY; \
		Data = MoveTemp(NewData); \
	} \
	\
	FORCEINLINE IndexType GetIndex(const int32 X, const int32 Y) const \
	{ \
		return X + (Y * SizeX); \
	} \
	FORCEINLINE IndexType GetIndex(const uint32 X, const uint32 Y) const \
	{ \
		return X + (Y * SizeX); \
	} \
	FORCEINLINE IndexType GetIndex(const FVector2D& Index) const \
	{ \
		return FMath::FloorToInt(Index.X) + (FMath::FloorToInt(Index.Y) * SizeX); \
	} \
	FORCEINLINE IndexType GetIndex(const FIntVector2& Index) const \
	{ \
		return Index.X + (Index.Y * SizeX); \
	} \
	\
	FORCEINLINE DataType GetData(const int32 X, const int32 Y) const \
	{ \
		return DataArray[GetIndex(X, Y)]; \
	} \
	FORCEINLINE DataType GetData(const uint32 X, const uint32 Y) const \
	{ \
		return DataArray[GetIndex(X, Y)]; \
	} \
	FORCEINLINE DataType GetData(const FVector2D& Index) const \
	{ \
		return DataArray[GetIndex(Index)]; \
	} \
	FORCEINLINE DataType GetData(const FIntVector2& Index) const \
	{ \
		return DataArray[GetIndex(Index)]; \
	} \
	FORCEINLINE DataType GetData(const IndexType& Index) const \
	{ \
		return DataArray[Index]; \
	} \
	\
	FORCEINLINE void SetData(const int32 X, const int32 Y, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(X, Y)] = NewValue; \
	} \
	FORCEINLINE void SetData(const uint32 X, const uint32 Y, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(X, Y)] = NewValue; \
	} \
	FORCEINLINE void SetData(const FVector2D& Index, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(Index)] = NewValue; \
	} \
	FORCEINLINE void SetData(const FIntVector2& Index, const DataType& NewValue) \
	{ \
		DataArray[GetIndex(Index)] = NewValue; \
	} \
	FORCEINLINE void SetData(const IndexType& Index, const DataType& NewValue) \
	{ \
		DataArray[Index] = NewValue; \
	} \
	FORCEINLINE size_t GetCount() const \
	{ \
		return DataArray.Num(); \
	}

#define N_IMPLEMENT_FLAT_3D_ARRAY(IndexType, DataType, DataArray, SizeX, SizeY, SizeZ) \
public: \
	void Reset(const IndexType InSizeX, const IndexType InSizeY, const IndexType InSizeZ) \
	{ \
		SizeX = InSizeX; \
		SizeY = InSizeY; \
		SizeZ = InSizeZ; \
		Data.Empty(); \
		Data.SetNum(SizeX * SizeY * SizeZ); \
	} \
	void Resize(const IndexType NewSizeX, const IndexType NewSizeY, const IndexType NewSizeZ) \
	{ \
		TArray<DataType> NewData; \
		NewData.Empty(); \
		NewData.SetNum(NewSizeX * NewSizeY * NewSizeZ); \
		for (IndexType x = 0; x < SizeX; x++) \
		{ \
			if (x >= NewSizeX) continue; \
			for (IndexType y = 0; y < SizeY; y++) \
			{ \
				if (y >= NewSizeY) continue; \
				for (IndexType z = 0; z < SizeZ; z++) \
				{ \
					if (z >= NewSizeZ) continue; \
					NewData[x + (y * NewSizeX) + (z * NewSizeX * NewSizeY)] = MoveTemp(Data[GetIndex(x, y, z)]); \
				} \
			} \
		} \
		SizeX = NewSizeX; \
		SizeY = NewSizeY; \
		SizeZ = NewSizeZ; \
		Data = MoveTemp(NewData); \
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
	} \
	FORCEINLINE size_t GetCount() const \
	{ \
		return DataArray.Num(); \
	}