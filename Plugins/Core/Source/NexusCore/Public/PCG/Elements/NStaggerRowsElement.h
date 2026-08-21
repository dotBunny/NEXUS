// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Math/NVectorUtils.h"
#include "PCGSettings.h"

#include "NStaggerRowsElement.generated.h"

/** How a point's position along the row axis resolves to the row it belongs to. */
UENUM(BlueprintType)
enum class ENRowDetection : uint8
{
	/** Derive rows from the data itself: points sitting within Row Tolerance of one another share a row, numbered along the row axis. */
	Tolerance	UMETA(DisplayName = "Tolerance (Derived)"),
	/** Bucket the row axis into fixed-width bands anchored at the world origin, keeping row numbering consistent across separately generated partitions. */
	FixedSize	UMETA(DisplayName = "Fixed Size")
};

/** Which half of the rows receives the offset. */
UENUM(BlueprintType)
enum class ENRowParity : uint8
{
	/** Offset rows 0, 2, 4 ... so the first row moves. */
	Even	UMETA(DisplayName = "Even (0, 2, 4 ...)"),
	/** Offset rows 1, 3, 5 ... so the first row stays put, giving the classic running-bond brick look. */
	Odd		UMETA(DisplayName = "Odd (1, 3, 5 ...)")
};

/**
 * PCG settings node that groups a point cloud into rows along one axis and shifts every other row
 * along another, producing a staggered running-bond layout.
 *
 * The row grouping is internal bookkeeping used only to decide which points move — the output keeps
 * the input's point order, so downstream nodes that index into the data are unaffected.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/pcg/elements/stagger-rows/">UNStaggerRowsSettings</a>
 */
UCLASS(BlueprintType, Blueprintable, Category="NEXUS")
class UNStaggerRowsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("NEXUS | Stagger Rows"); }
	virtual FText GetNodeTooltipText() const override { return INVTEXT("Groups points into rows along one axis, then offsets every other row along another."); }
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::PointOps; }
#endif

	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

	/** Axis whose position decides which row a point belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rows", meta = (PCG_Overridable, ToolTip = "Axis whose position decides which row a point belongs to. Rows run perpendicular to it."))
	ENAxis RowAxis = ENAxis::X;

	/** How a point's position along the row axis resolves to the row it belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rows", meta = (PCG_Overridable, ToolTip = "How a point's position along the row axis resolves to the row it belongs to."))
	ENRowDetection RowDetection = ENRowDetection::Tolerance;

	/** Widest spread along the row axis allowed within a single row; a larger gap opens the next row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rows", meta = (ClampMin = "0.0", EditCondition = "RowDetection == ENRowDetection::Tolerance", EditConditionHides, PCG_Overridable, ToolTip = "Widest spread along the row axis allowed within a single row; a larger gap opens the next row. Zero groups exactly-matching positions only."))
	double RowTolerance = 1.0;

	/** Width of each row band along the row axis, measured out from the world origin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Rows", meta = (ClampMin = "0.0", EditCondition = "RowDetection == ENRowDetection::FixedSize", EditConditionHides, PCG_Overridable, ToolTip = "Width of each row band along the row axis, measured out from the world origin."))
	double RowSize = 100.0;

	/** Axis the offset is applied along. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Offset", meta = (PCG_Overridable, ToolTip = "Axis the offset is applied along. Keep this different from the row axis — shifting along the row axis would slide points into neighbouring rows instead of staggering them."))
	ENAxis OffsetAxis = ENAxis::Y;

	/** Which half of the rows receives the offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Offset", meta = (PCG_Overridable, ToolTip = "Which half of the rows receives the offset."))
	ENRowParity RowParity = ENRowParity::Even;

	/** Amount added along the offset axis for every point in a matching row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Offset", meta = (PCG_Overridable, ToolTip = "Amount added along the offset axis for every point in a matching row. Half the point spacing gives an evenly staggered running bond."))
	double RowOffset = 0.0;

	/** When true, write each point's resolved row index out as an attribute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Write Row Index?", Category = "Settings|Metadata", meta = (PCG_Overridable, ToolTip = "Should each point's resolved row index be written out as an attribute?"))
	bool bWriteRowIndex = true;

	/** Attribute name storing each point's resolved row index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Row Index", Category = "Settings|Metadata", meta = (EditCondition = "bWriteRowIndex", EditConditionHides, PCG_Overridable, ToolTip = "Name of the attribute storing each point's resolved row index."))
	FName RowIndexAttributeName = TEXT("RowIndex");

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Executor paired with UNStaggerRowsSettings.
 */
class FNStaggerRowsElement : public IPCGElement
{
public:
	/**
	 * Maps an axis onto the FVector component index it addresses.
	 * @param Axis The axis to resolve.
	 * @return 0, 1 or 2 for X, Y and Z; INDEX_NONE for ENAxis::None.
	 */
	static NEXUSCORE_API int32 GetAxisIndex(ENAxis Axis);

	/**
	 * Groups positions along the row axis into rows, numbering them from the negative side up from zero.
	 * Pure and PCG-free so the grouping maths can be unit-tested directly.
	 * @param Positions Each point's position along the row axis, in input order.
	 * @param Tolerance Widest spread allowed within a row; a larger gap opens the next row. Zero groups exact matches only.
	 * @param OutRowIndices Receives one row index per input position, in input order.
	 */
	static NEXUSCORE_API void AssignRowIndices(TConstArrayView<double> Positions, double Tolerance, TArray<int32>& OutRowIndices);

	/**
	 * Resolves a position on the row axis to a fixed-width row band anchored at the world origin.
	 * @param Position The point's position along the row axis.
	 * @param RowSize Width of each band; a value at or below zero collapses everything into row zero.
	 * @return The band index, negative for positions on the negative side of the origin.
	 */
	static NEXUSCORE_API int32 GetFixedRowIndex(double Position, double RowSize);

	/**
	 * Tests whether a row index matches the parity selected for offsetting.
	 * @param RowIndex The row's index, which may be negative under fixed-size detection.
	 * @param Parity The parity that receives the offset.
	 * @return True when the row should be shifted along the offset axis.
	 */
	static NEXUSCORE_API bool ShouldOffsetRow(int32 RowIndex, ENRowParity Parity);

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(const UPCGSettings* Settings) const override { return EPCGElementExecutionLoopMode::SinglePrimaryPin; }
};
