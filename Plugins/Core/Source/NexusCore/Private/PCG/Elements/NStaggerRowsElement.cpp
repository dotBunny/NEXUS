// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "PCG/Elements/NStaggerRowsElement.h"

#include "NColor.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"
#include "Metadata/PCGMetadata.h"

TArray<FPCGPinProperties> UNStaggerRowsSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
	return PinProperties;
}

TArray<FPCGPinProperties> UNStaggerRowsSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return PinProperties;
}

#if WITH_EDITOR
FLinearColor UNStaggerRowsSettings::GetNodeTitleColor() const
{
	return FNColor::SortElement;
}
#endif

FPCGElementPtr UNStaggerRowsSettings::CreateElement() const
{
	return MakeShared<FNStaggerRowsElement>();
}

int32 FNStaggerRowsElement::GetAxisIndex(const ENAxis Axis)
{
	switch (Axis)
	{
		case ENAxis::X:
			return 0;
		case ENAxis::Y:
			return 1;
		case ENAxis::Z:
			return 2;
		default:
			return INDEX_NONE;
	}
}

void FNStaggerRowsElement::AssignRowIndices(const TConstArrayView<double> Positions, const double Tolerance, TArray<int32>& OutRowIndices)
{
	const int32 NumPoints = Positions.Num();
	OutRowIndices.Reset();
	OutRowIndices.SetNumZeroed(NumPoints);
	if (NumPoints == 0)
	{
		return;
	}

	// Visit the points in ascending order along the row axis so a row break is simply a gap wider than the
	// tolerance. Sorting indices rather than the positions keeps the caller's ordering intact on the way out.
	TArray<int32> SortedIndices;
	SortedIndices.SetNumUninitialized(NumPoints);
	for (int32 Index = 0; Index < NumPoints; ++Index)
	{
		SortedIndices[Index] = Index;
	}
	SortedIndices.Sort([&Positions](const int32 A, const int32 B) { return Positions[A] < Positions[B]; });

	// Measure the gap against the position that opened the row rather than the previous point's, so a long
	// run of closely-spaced points can never drift across what should have been a row boundary.
	int32 RowIndex = 0;
	double RowAnchor = Positions[SortedIndices[0]];
	for (const int32 PointIndex : SortedIndices)
	{
		const double Position = Positions[PointIndex];
		if (Position - RowAnchor > Tolerance)
		{
			++RowIndex;
			RowAnchor = Position;
		}
		OutRowIndices[PointIndex] = RowIndex;
	}
}

int32 FNStaggerRowsElement::GetFixedRowIndex(const double Position, const double RowSize)
{
	// A zero or negative band width has no meaningful division; collapse everything into the first row.
	if (RowSize <= UE_DOUBLE_SMALL_NUMBER)
	{
		return 0;
	}

	return FMath::FloorToInt32(Position / RowSize);
}

bool FNStaggerRowsElement::ShouldOffsetRow(const int32 RowIndex, const ENRowParity Parity)
{
	// Fixed-size detection produces negative row indices on the negative side of the origin, where C++ '%'
	// yields a negative remainder; normalize into {0, 1} before comparing against the requested parity.
	const int32 NormalizedParity = ((RowIndex % 2) + 2) % 2;
	return NormalizedParity == (Parity == ENRowParity::Even ? 0 : 1);
}

bool FNStaggerRowsElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FNStaggerRowsElement::Execute);
	check(Context);

	const UNStaggerRowsSettings* Settings = Context->GetInputSettings<UNStaggerRowsSettings>();
	check(Settings);

	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	const int32 RowAxisIndex = GetAxisIndex(Settings->RowAxis);
	const int32 OffsetAxisIndex = GetAxisIndex(Settings->OffsetAxis);

	// Without a row axis there is nothing to group by, and without an offset axis nothing to move along.
	if (RowAxisIndex == INDEX_NONE || OffsetAxisIndex == INDEX_NONE)
	{
		Outputs.Append(Inputs);
		return true;
	}

	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGBasePointData* InputPointData = Cast<const UPCGBasePointData>(Input.Data);
		if (!InputPointData)
		{
			continue;
		}

		const int32 NumPoints = InputPointData->GetNumPoints();
		if (NumPoints == 0)
		{
			Outputs.Add(Input);
			continue;
		}

		// Resolve the row each point belongs to. This grouping never reaches the output as an ordering —
		// it only decides which points get shifted — so the input's point order survives untouched.
		const TConstPCGValueRange<FTransform> InputTransforms = InputPointData->GetConstTransformValueRange();
		TArray<int32> RowIndices;
		if (Settings->RowDetection == ENRowDetection::Tolerance)
		{
			TArray<double> Positions;
			Positions.SetNumUninitialized(NumPoints);
			for (int32 Index = 0; Index < NumPoints; ++Index)
			{
				Positions[Index] = InputTransforms[Index].GetTranslation()[RowAxisIndex];
			}

			AssignRowIndices(Positions, Settings->RowTolerance, RowIndices);
		}
		else
		{
			RowIndices.SetNumUninitialized(NumPoints);
			for (int32 Index = 0; Index < NumPoints; ++Index)
			{
				RowIndices[Index] = GetFixedRowIndex(InputTransforms[Index].GetTranslation()[RowAxisIndex], Settings->RowSize);
			}
		}

		// Copy the input straight across (bCopyAll, so index N in maps to index N out) and mutate from there.
		UPCGBasePointData* OutputPointData = FPCGContext::NewPointData_AnyThread(Context);
		FPCGInitializeFromDataParams InitializeFromDataParams(InputPointData);
		InitializeFromDataParams.bInheritSpatialData = false;
		OutputPointData->InitializeFromDataWithParams(InitializeFromDataParams);
		UPCGBasePointData::SetPoints(InputPointData, OutputPointData, TArrayView<const int32>(), /*bCopyAll=*/true);

		UPCGMetadata* OutputMetadata = OutputPointData->MutableMetadata();
		FPCGMetadataAttribute<int32>* RowIndexAttribute = (Settings->bWriteRowIndex && OutputMetadata)
			? OutputMetadata->FindOrCreateAttribute<int32>(Settings->RowIndexAttributeName, 0, /*bAllowsInterpolation=*/false, /*bOverrideParent=*/true)
			: nullptr;

		TPCGValueRange<FTransform> OutputTransforms = OutputPointData->GetTransformValueRange();
		TPCGValueRange<int64> OutputMetadataEntries = RowIndexAttribute ? OutputPointData->GetMetadataEntryValueRange() : TPCGValueRange<int64>();

		for (int32 Index = 0; Index < NumPoints; ++Index)
		{
			const int32 RowIndex = RowIndices[Index];

			if (ShouldOffsetRow(RowIndex, Settings->RowParity))
			{
				FVector Translation = OutputTransforms[Index].GetTranslation();
				Translation[OffsetAxisIndex] += Settings->RowOffset;
				OutputTransforms[Index].SetTranslation(Translation);
			}

			if (RowIndexAttribute)
			{
				// Entries arrive copied from (and potentially parented to) the input's metadata, so give each
				// point a local entry first or the writes collapse onto whatever entry they happen to share.
				OutputMetadata->InitializeOnSet(OutputMetadataEntries[Index]);
				RowIndexAttribute->SetValue(OutputMetadataEntries[Index], RowIndex);
			}
		}

		Outputs.Add_GetRef(Input).Data = OutputPointData;
	}

	return true;
}
