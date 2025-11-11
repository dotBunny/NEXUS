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
	FORCEINLINE IndexType GetIndex(const IndexType X, const IndexType Y) const \
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
	FORCEINLINE TTuple<IndexType, IndexType> GetInverseIndex(const IndexType Index) const \
	{ \
		const IndexType X = FMath::Modulo(Index, SizeX); \
		const IndexType Y = FMath::Modulo(((Index - X)/SizeX), SizeY); \
		return TTuple<IndexType, IndexType>(X,Y); \
	} \
	\
	FORCEINLINE DataType GetData(const IndexType X, const IndexType Y) const \
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
	FORCEINLINE void SetData(const IndexType X, const IndexType Y, const DataType& NewValue) \
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
	\
	FORCEINLINE size_t GetCount() const \
	{ \
		return DataArray.Num(); \
	} \
	\
	FORCEINLINE void GetSurroundingIndices(const IndexType Index, TArray<IndexType>& OutIndices) const \
	{ \
		auto [x,y] = GetInverseIndex(Index); \
		GetSurroundingIndices(x,y, OutIndices); \
	} \
	void GetSurroundingIndices(const IndexType X, const IndexType Y, TArray<IndexType>& OutIndices) const \
	{ \
		const bool bCanXSubtract = X > 0; \
		const uint32 XMinus = (X - 1); \
		const bool bCanXAdd = X < (SizeX - 1); \
		const uint32 XAdd = X + 1; \
		const bool bCanYSubtract = Y > 0; \
		const uint32 YMinus = (Y - 1); \
		const bool bCanYAdd = Y < (SizeY - 1); \
		const uint32 YAdd = (Y + 1); \
		if (bCanXSubtract) \
		{ \
			OutIndices.Add(GetIndex(XMinus, Y)); \
			if (bCanYAdd) OutIndices.Add(GetIndex(XMinus, YAdd)); \
			if (bCanYSubtract) OutIndices.Add(GetIndex(XMinus, YMinus)); \
		} \
		if (bCanXAdd) \
		{ \
			OutIndices.Add(GetIndex(XAdd, Y)); \
			if (bCanYAdd) OutIndices.Add(GetIndex(XAdd, YAdd)); \
			if (bCanYSubtract) OutIndices.Add(GetIndex(XAdd, YMinus)); \
		} \
		if (bCanYSubtract) OutIndices.Add(GetIndex(X, YMinus)); \
		if (bCanYAdd) OutIndices.Add(GetIndex(X, YAdd)); \
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
	FORCEINLINE IndexType GetIndex(const IndexType X, const IndexType Y, const IndexType Z) const \
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
	FORCEINLINE TTuple<IndexType, IndexType, IndexType> GetInverseIndex(const IndexType Index) const \
	{ \
		const IndexType X = FMath::Modulo(Index, SizeX); \
		const IndexType Y = FMath::Modulo(((Index - X)/SizeX), SizeY); \
		const IndexType Z = (Index-X -SizeX*Y)/(SizeX*SizeY); \
		return TTuple<IndexType,IndexType, IndexType>(X,Y, Z); \
	} \
	\
	FORCEINLINE DataType GetData(const IndexType X, const IndexType Y, const IndexType Z) const \
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
	FORCEINLINE void SetData(const IndexType X, const IndexType Y, const IndexType Z, const DataType& NewValue) \
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
	\
	FORCEINLINE size_t GetCount() const \
	{ \
		return DataArray.Num(); \
	} \
	\
	FORCEINLINE void GetSurroundingIndices(const IndexType Index, TArray<IndexType>& OutIndices) const \
	{ \
		auto [x,y,z] = GetInverseIndex(Index); \
		GetSurroundingIndices(x,y,z, OutIndices); \
	} \
	void GetSurroundingIndices(const IndexType X, const IndexType Y, const IndexType Z, TArray<IndexType>& OutIndices) const \
	{ \
		const bool bCanXSubtract = X > 0; \
		const uint32 XMinus = (X - 1); \
		const bool bCanXAdd = X < (SizeX - 1); \
		const uint32 XAdd = X + 1; \
		const bool bCanYSubtract = Y > 0; \
		const uint32 YMinus = (Y - 1); \
		const bool bCanYAdd = Y < (SizeY - 1); \
		const uint32 YAdd = (Y + 1); \
		const bool bCanZSubtract = Z > 0; \
		const uint32 ZMinus = (Z - 1); \
		const bool bCanZAdd = Z < (SizeZ - 1); \
		const uint32 ZAdd = (Z + 1); \
		if (bCanXSubtract) \
		{ \
			OutIndices.Add(GetIndex(XMinus, Y, Z)); \
			if (bCanYAdd) \
			{ \
				OutIndices.Add(GetIndex(XMinus, YAdd, Z)); \
				if (bCanZAdd) OutIndices.Add(GetIndex(XMinus, YAdd, ZAdd)); \
				if (bCanZSubtract) OutIndices.Add(GetIndex(XMinus, YAdd, ZMinus)); \
			} \
			if (bCanYSubtract) \
			{ \
				OutIndices.Add(GetIndex(XMinus, YMinus, Z)); \
				if (bCanZAdd) OutIndices.Add(GetIndex(XMinus, YMinus, ZAdd)); \
				if (bCanZSubtract) OutIndices.Add(GetIndex(XMinus, YMinus, ZMinus)); \
			} \
		} \
		if (bCanXAdd) \
		{ \
			OutIndices.Add(GetIndex(XAdd, Y, Z)); \
			if (bCanYAdd) \
			{ \
				OutIndices.Add(GetIndex(XAdd, YAdd, Z)); \
				if (bCanZAdd) OutIndices.Add(GetIndex(XAdd, YAdd, ZAdd)); \
				if (bCanZSubtract) OutIndices.Add(GetIndex(XAdd, YAdd, ZMinus)); \
			} \
			if (bCanYSubtract) \
			{ \
				OutIndices.Add(GetIndex(XAdd, YMinus, Z)); \
				if (bCanZAdd) OutIndices.Add(GetIndex(XAdd, YMinus, ZAdd)); \
				if (bCanZSubtract) OutIndices.Add(GetIndex(XAdd, YMinus, ZMinus)); \
			} \
		} \
		if (bCanYSubtract) \
		{ \
			OutIndices.Add(GetIndex(X, YMinus, Z)); \
			if (bCanZAdd) OutIndices.Add(GetIndex(X, YMinus, ZAdd)); \
			if (bCanZSubtract) OutIndices.Add(GetIndex(X, YMinus, ZMinus)); \
		} \
		if (bCanYAdd) \
		{ \
			OutIndices.Add(GetIndex(X, YAdd, Z)); \
			if (bCanZAdd) OutIndices.Add(GetIndex(X, YAdd, ZAdd)); \
			if (bCanZSubtract) OutIndices.Add(GetIndex(X, YAdd, ZMinus)); \
		} \
		if (bCanZSubtract) \
		{ \
			OutIndices.Add(GetIndex(X, Y, ZMinus)); \
			if (bCanXAdd) OutIndices.Add(GetIndex(XAdd, Y, ZMinus)); \
			if (bCanXSubtract) OutIndices.Add(GetIndex(XMinus, Y, ZMinus)); \
		} \
		if (bCanZAdd) \
		{ \
			OutIndices.Add(GetIndex(X, Y, ZAdd)); \
			if (bCanXAdd) OutIndices.Add(GetIndex(XAdd, Y, ZAdd)); \
			if (bCanXSubtract) OutIndices.Add(GetIndex(XMinus, Y, ZAdd)); \
		} \
	}