// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NDrawDebugHelpers.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "Assembly/NJunctionConnectorSolver.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellJunctionConnection.h"
#include "Cell/NCellLevelInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "Math/NSeedGenerator.h"
#include "NWorldAssemblyLibrary.generated.h"

/**
 * A utility class providing functionality to support World Assembly operations.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/">UNWorldAssemblyLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | World Assembly")
class NEXUSWORLDASSEMBLY_API UNWorldAssemblyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Convert a junction's grid socket size into world units using the project's SocketSize/SocketDepth settings.
	 * @param JunctionComponent The junction whose socket size to convert.
	 * @param bWithDepth When true, fills Z with the configured SocketDepth; otherwise Z stays 1.
	 * @return The junction's world-space size (X,Y scaled from the socket grid; Z = depth when requested).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Size",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-junction-world-size"))
	static FVector GetJunctionWorldSize(UNCellJunctionComponent* JunctionComponent, bool bWithDepth = false)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Size as no JunctionComponent was made available."));
			return  FVector::ZeroVector;
		}
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		FVector ReturnVector = FVector::OneVector;
		ReturnVector.X = JunctionComponent->Details.SocketSize.X * Settings->SocketSize.X;
		ReturnVector.Y = JunctionComponent->Details.SocketSize.Y * Settings->SocketSize.Y;
		if (bWithDepth)
		{
			ReturnVector.Z = JunctionComponent->GetFillDepth();
		}
		return ReturnVector;
	}

	/**
	 * Variant of GetJunctionWorldSize that returns the size as (Depth, X, Y) — useful when the depth axis must lead.
	 * @param JunctionComponent The junction whose socket size to convert.
	 * @param Scale Uniform multiplier applied to all three components.
	 * @return A vector packed as (SocketDepth, world X, world Y), each scaled by Scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Size (Shifted)", meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-junction-world-size-shifted", ToolTip="Depth, X, Y"))
	static FVector GetJunctionWorldSizeShifted(UNCellJunctionComponent* JunctionComponent, float Scale = 1.f)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Size as no JunctionComponent was made available."));
			return  FVector::ZeroVector;
		}
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		return FVector(
			JunctionComponent->GetFillDepth() * Scale,
			(JunctionComponent->Details.SocketSize.X * Settings->SocketSize.X) * Scale,
			(JunctionComponent->Details.SocketSize.Y * Settings->SocketSize.Y) * Scale);
	}

	/**
	 * Signed distance, along the junction's forward axis, at which a filler should anchor its fill volume before it
	 * extrudes forward by the fill depth. Encodes the direction of the junction's FillDepthMode: 0 for the forward
	 * modes, -depth for the backward modes, and -depth/2 for the centered modes.
	 * @param JunctionComponent The junction whose fill-depth anchor to read.
	 * @return The anchor distance in world units; negative values shift the volume toward the junction's backward direction.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction Fill Depth Anchor",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-junction-fill-depth-anchor"))
	static float GetJunctionFillDepthAnchor(UNCellJunctionComponent* JunctionComponent)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction Fill Depth Anchor as no JunctionComponent was made available."));
			return 0.f;
		}
		return JunctionComponent->GetFillDepthAnchor();
	}

	/**
	 * World-space form of Get Junction Fill Depth Anchor: the anchor distance projected along the junction's forward
	 * direction. Add this to a filler's placement location so it extrudes forward from the anchored near edge, which
	 * realizes the Forward / Backward / Centered fill-depth modes without moving the junction's spawn transform.
	 * @param JunctionComponent The junction whose fill-depth anchor to read.
	 * @return The world-space anchor offset (junction forward direction scaled by the signed anchor distance).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction Fill Depth Offset",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-junction-fill-depth-offset"))
	static FVector GetJunctionFillDepthOffset(UNCellJunctionComponent* JunctionComponent)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction Fill Depth Offset as no JunctionComponent was made available."));
			return FVector::ZeroVector;
		}
		return JunctionComponent->GetComponentRotation().Vector() * JunctionComponent->GetFillDepthAnchor();
	}

	/**
	 * @param JunctionComponent The junction to query.
	 * @param SocketSize Socket size (in grid units) to project the corners for.
	 * @return The junction's four corner points in world space for the given socket size.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Corner Points",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-junction-world-corner-points"))
	static TArray<FVector> GetJunctionWorldCornerPoints(UNCellJunctionComponent* JunctionComponent, const FVector2D& SocketSize)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Corner Points as no JunctionComponent was made available."));
			return  TArray<FVector>();
		}
		return JunctionComponent->GetWorldCornerPoints(SocketSize);
	};


	/**
	 * Draw the route a junction connector was given, as debug lines.
	 *
	 * By default this draws the path's stored samples — the exact points the connector pass swept for collisions —
	 * so what appears is the geometry that was proven clear rather than a re-approximation of it. Those samples are
	 * spaced by the pass's Sample Step and taken uniformly in curve parameter rather than by arc length, so a
	 * sharply-bending route reads as a run of flat facets. Subdivisions trades that literalness for a smoother
	 * read; see its note.
	 * @param WorldContextObject Object that provides the context of which world to operate in.
	 * @param Path The cached route, as handed to a connector through OnConnectJunctions.
	 * @param CenterColor The color to draw the center curve with.
	 * @param CornerColor The color to draw the four socket-corner curves with.
	 * @param bDrawCorners Should the corner curves be drawn alongside the center? They bound the volume a connector's
	 *        geometry may occupy, so they are usually the more useful view of the two.
	 * @param Subdivisions Extra points to evaluate between each stored sample of the center curve, smoothing out the
	 *        faceting. Zero draws the stored samples untouched.
	 * @param bPersistentLines Should the drawn lines be permanent?
	 * @param LifeTime How long should the lines last if not permanent, -1 for a single frame.
	 * @param DepthPriority What priority should they be drawn at?
	 * @param Thickness The thickness of the drawn lines.
	 * @note Subdivisions only smooths the center curve, which is the one stored as a curve definition (control points
	 *       plus tangents) that can be re-evaluated at any resolution. The corner curves are generated by sweeping a
	 *       frame along the center and have no such definition, so they keep their sampled resolution — smoothing
	 *       them would mean inventing geometry the pass never tested. Subdividing also never moves the line off the
	 *       stored samples: it evaluates the same curve at a finer step, so the drawn path still passes through
	 *       every point that was collision-tested.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Draw Junction Connector Path",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#draw-junction-connector-path",
			WorldContext = "WorldContextObject", ClampMin="0", UIMax="16"))
	static void DrawJunctionConnectorPath(const UObject* WorldContextObject, const FNCellJunctionConnectorPath& Path,
		const FLinearColor CenterColor = FLinearColor::Green, const FLinearColor CornerColor = FLinearColor::Yellow,
		const bool bDrawCorners = true, const int32 Subdivisions = 0, const bool bPersistentLines = false,
		const float LifeTime = -1.f, const uint8 DepthPriority = 0, const float Thickness = 2.f)
	{
		const UWorld* InWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);
		if (InWorld == nullptr) return;

		const FColor ResolvedCenterColor = CenterColor.ToFColor(true);

		// Subdividing needs the curve definition the samples were produced from. A path that predates it, or was
		// default-constructed, still draws — just at its stored resolution.
		const int32 SampleSegmentCount = Path.Center.Points.Num() - 1;
		const bool bSubdivide = Subdivisions > 0 && SampleSegmentCount > 0
			&& Path.ControlPoints.Num() >= 2 && Path.ControlTangents.Num() == Path.ControlPoints.Num();

		if (bSubdivide)
		{
			// Evaluating at a whole multiple of the stored step keeps every original sample on the drawn line, so a
			// smoothed view and the validated one never disagree about where the path goes.
			const int32 DrawSegmentCount = SampleSegmentCount * (Subdivisions + 1);

			TArray<FVector> SmoothPoints;
			SmoothPoints.Reserve(DrawSegmentCount + 1);
			for (int32 i = 0; i <= DrawSegmentCount; i++)
			{
				SmoothPoints.Add(FNJunctionConnectorSolver::EvaluateCurve(Path.ControlPoints, Path.ControlTangents,
					static_cast<float>(i) / DrawSegmentCount));
			}

			FNDrawDebugHelpers::DrawPointLine(InWorld, SmoothPoints, ResolvedCenterColor, false,
				bPersistentLines, LifeTime, DepthPriority, Thickness);
		}
		else
		{
			FNDrawDebugHelpers::DrawPointLine(InWorld, Path.Center.Points, ResolvedCenterColor, false,
				bPersistentLines, LifeTime, DepthPriority, Thickness);
		}

		if (!bDrawCorners) return;

		const FColor ResolvedCornerColor = CornerColor.ToFColor(true);
		for (const FNCellJunctionConnectorCurve& Corner : Path.Corners)
		{
			FNDrawDebugHelpers::DrawPointLine(InWorld, Corner.Points, ResolvedCornerColor, false,
				bPersistentLines, LifeTime, DepthPriority, Thickness);
		}
	}

	/** @return A freshly generated human-friendly seed string suitable for use as FNAssemblyOperationSettings::Seed. */
	UFUNCTION(BlueprintPure, Category = "NEXUS|WorldAssembly", DisplayName="Get New Friendly Seed",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-new-friendly-seed"))
	static FString GetNewFriendlySeed() { return FNSeedGenerator::RandomFriendlySeed(); }

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final context tags associated with the world assembly.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Append/Remove Context Tags to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-context-tags"))
	static FGameplayTagContainer GetContextTags(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetContextTags() : FGameplayTagContainer();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The context tags this cell added to the overall state.
	 * @note Returns a copy; edits made to it are not written back to the cell.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags Added",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#reading-cell-state"))
	static FGameplayTagContainer GetContextTagsAdded(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetContextTagsAdded() : FGameplayTagContainer();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The assembly tags used by the cell itself.
	 * @note Returns a copy; edits made to it are not written back to the cell.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Assembly Tags",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-assembly-tags"))
	static FGameplayTagContainer GetAssemblyTags(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetAssemblyTags() : FGameplayTagContainer();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The cell's seed formatted as a human-readable hexadecimal string.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Hex Seed",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-hex-seed"))
	static FString GetHexSeed(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? FNSeedGenerator::HexFromSeed(LevelInstance->GetSeed()) : FString();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The identifier of the graph node this cell was assembled from.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Node Identifier",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#get-node-identifier"))
	static int32 GetNodeIdentifier(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetNodeIdentifier() : INDEX_NONE;
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Add/Subtract Tag Counter to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Map)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#reading-cell-state"))
	static TMap<FGameplayTag, int32> GetTagCounter(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetTagCounter() : TMap<FGameplayTag, int32>();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation as an array of tag/count pairs.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Add/Subtract Tag Counter to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Array)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#reading-cell-state"))
	static TArray<FNGameplayTagCount> GetTagCounterArray(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetTagCounterArray() : TArray<FNGameplayTagCount>();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The ticket identifying the assembly operation this cell belongs to, used to key the context cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Operation Ticket",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#reading-cell-state"))
	static int32 GetOperationTicket(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetOperationTicket() : INDEX_NONE;
	}

	/**
	 * Only the owner / creator of the ANCellProxy will be able to reach the DynamicMesh through their ANCellLevelInstance.
	 * @param LevelInstance The cell level instance to query.
	 * @return The cell's proxy dynamic mesh, or nullptr if the instance is invalid or has no proxy mesh set.
	 */
	static UDynamicMesh* GetProxyMesh(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetProxyMesh() : nullptr;
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the assembly's hot path.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#is-hotpath"))
	static bool IsHotPath(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPath();
	};

	/**
	 * Exec-pin variant of Is HotPath; the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the assembly's hot path.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPath();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the shortest-path hot path (spokes from the start cell).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Shortest)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#is-hotpath-shortest"))
	static bool IsHotPathShortest(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathShortest();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the shortest-path hot path (spokes from the start cell).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Shortest) ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathShortestExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathShortest();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the sequential hot path (nearest-first visiting chain).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Sequential)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#is-hotpath-sequential"))
	static bool IsHotPathSequential(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathSequential();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the sequential hot path (nearest-first visiting chain).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Sequential) ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathSequentialExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathSequential();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @param Tag Tag to look for.
	 * @return true if the cell's final TagCounter contains an entry for Tag.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#testing-cell-state"))
	static bool HasTagCounter(ANCellLevelInstance* LevelInstance, const FGameplayTag Tag)
	{
		return IsValid(LevelInstance) && LevelInstance->GetTagCounter().Contains(Tag);
	};

	/**
	 * Exec-pin variant of Has Tag Counter; the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @param Tag Tag to look for.
	 * @return true if the cell's final TagCounter contains an entry for Tag.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Tag Counter ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool HasTagCounterExec(ANCellLevelInstance* LevelInstance, const FGameplayTag Tag)
	{
		return IsValid(LevelInstance) && LevelInstance->GetTagCounter().Contains(Tag);
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cell's final context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Context Tag(s)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#testing-cell-state"))
	static bool HasContextTags(ANCellLevelInstance* LevelInstance, const FGameplayTagContainer TagContainer)
	{
		return IsValid(LevelInstance) && LevelInstance->GetContextTags().HasAllExact(TagContainer);
	};

	/**
	 * Exec-pin variant of Has Context Tag(s); the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cell's final context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Context Tag(s) ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool HasContextTagsExec(ANCellLevelInstance* LevelInstance, const FGameplayTagContainer TagContainer)
	{
		return IsValid(LevelInstance) && LevelInstance->GetContextTags().HasAllExact(TagContainer);
	};

	/**
	 * Adds tags to the cell's final context tag set (union; duplicates are ignored). Use this to persist edits:
	 * Get Context Tags returns a Blueprint copy, so changes made to that copy are not written back to the cell.
	 * @param LevelInstance The cell level instance to modify.
	 * @param TagContainer Tags to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Append Context Tags",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#mutating-cell-state"))
	static void AppendContextTags(ANCellLevelInstance* LevelInstance, FGameplayTagContainer TagContainer)
	{
		if (IsValid(LevelInstance))
		{
			LevelInstance->GetContextTags().AppendTags(TagContainer);
		}
	}

	/**
	 * Removes tags from the cell's final context tag set.
	 * @param LevelInstance The cell level instance to modify.
	 * @param TagContainer Tags to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Remove Context Tags",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#mutating-cell-state"))
	static void RemoveContextTags(ANCellLevelInstance* LevelInstance, FGameplayTagContainer TagContainer)
	{
		if (IsValid(LevelInstance))
		{
			LevelInstance->GetContextTags().RemoveTags(TagContainer);
		}
	}

	/**
	 * Adds to the cell's final TagCounter for a tag, creating the entry if needed. Use this to persist edits:
	 * the Get Tag Counter accessors return Blueprint copies, so changes made to those copies are not written back.
	 * @param LevelInstance The cell level instance to modify.
	 * @param Tag Tag whose counter to increase.
	 * @param Value Amount to add (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Add Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#mutating-cell-state"))
	static void AddTagCounter(ANCellLevelInstance* LevelInstance, FGameplayTag Tag, int32 Value = 1)
	{
		if (IsValid(LevelInstance))
		{
			FindOrAddTagCount(LevelInstance->GetTagCounterArray(), Tag).Count += Value;
		}
	}

	/**
	 * Subtracts from the cell's final TagCounter for a tag, creating the entry if needed.
	 * @param LevelInstance The cell level instance to modify.
	 * @param Tag Tag whose counter to decrease.
	 * @param Value Amount to subtract (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Subtract Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#mutating-cell-state"))
	static void SubtractTagCounter(ANCellLevelInstance* LevelInstance, FGameplayTag Tag, int32 Value = 1)
	{
		if (IsValid(LevelInstance))
		{
			FindOrAddTagCount(LevelInstance->GetTagCounterArray(), Tag).Count -= Value;
		}
	}

	/**
	 * Tests whether the context cache still holds state for an assembly operation. The cache is keyed by the
	 * operation's ticket (see Get Operation Ticket) and is what the Operation Tag Counter / Context Tag accessors below
	 * read and mutate.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @return true if cached context state exists for OperationTicket.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Cache",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static bool HasContextCache(int32 OperationTicket)
	{
		return FNWorldAssemblyContextCache::HasOperation(OperationTicket);
	}

	/**
	 * Exec-pin variant of Has Operation Context Cache; the result drives the True/False execution outputs in Blueprint.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @return true if cached context state exists for OperationTicket.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Cache ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool HasContextCacheExec(int32 OperationTicket)
	{
		return FNWorldAssemblyContextCache::HasOperation(OperationTicket);
	}

	/**
	 * Reads the cached operation's running tag counter for a single tag.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter value to read.
	 * @return The counter value, or -1 if the operation or tag is not present in the cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Operation Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static int32 GetOperationTagCounter(int32 OperationTicket, FGameplayTag Tag)
	{
		int32 Count = -1;
		FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, Count);
		return Count;
	}

	/**
	 * Reads the cached operation's running tag counter for a single tag, distinguishing "absent" from a real value (unlike
	 * Get Operation Tag Counter, which collapses both to -1).
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter value to read.
	 * @param OutValue Receives the counter value when found; left unchanged otherwise.
	 * @return true if a counter entry exists for Tag on this operation.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="TryGet Operation Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static bool TryGetOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32& OutValue)
	{
		return FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, OutValue);
	}

	/**
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag to look for.
	 * @return true if the cached operation has a counter entry for Tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static bool HasOperationTagCounter(int32 OperationTicket, FGameplayTag Tag)
	{
		return FNWorldAssemblyContextCache::HasTagCount(OperationTicket, Tag);
	}

	/**
	 * Adds to the cached operation's running counter for a tag, creating the entry if needed.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter to increase.
	 * @param Value Amount to add (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Add Operation Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static void AddOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32 Value = 1)
	{
		FNWorldAssemblyContextCache::AddTagCount(OperationTicket, Tag, Value);
	}

	/**
	 * Subtracts from the cached operation's running counter for a tag.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter to decrease.
	 * @param Value Amount to subtract (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Subtract Operation Tag Counter",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static void SubtractOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32 Value = 1)
	{
		FNWorldAssemblyContextCache::SubtractTagCount(OperationTicket, Tag, Value);
	}

	/**
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cached operation's context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Tag(s)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static bool HasOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		return FNWorldAssemblyContextCache::HasContextTags(OperationTicket, TagContainer);
	}

	/**
	 * Exec-pin variant of Has Operation Context Tag(s); the result drives the True/False execution outputs in Blueprint.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cached operation's context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Tag(s) ?",
		meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#exec-pin-variants", ExpandBoolAsExecs="ReturnValue"))
	static bool HasOperationContextTagsExec(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		return FNWorldAssemblyContextCache::HasContextTags(OperationTicket, TagContainer);
	}

	/**
	 * Adds tags to the cached operation's context tag set (union; duplicates are ignored).
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Append Operation Context Tag(s)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static void AppendOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		FNWorldAssemblyContextCache::AppendContextTags(OperationTicket, TagContainer);
	}

	/**
	 * Removes tags from the cached operation's context tag set.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Remove Operation Context Tag(s)",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/world-assembly-library/#operation-context-cache"))
	static void RemoveOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		FNWorldAssemblyContextCache::RemoveContextTags(OperationTicket, TagContainer);
	}

private:
	/**
	 * Finds the counter entry for Tag in Counter, appending a zero-initialized entry (Count = 0) when absent so
	 * callers can adjust Count in place.
	 * @param Counter The flat tag-counter array to search.
	 * @param Tag The tag whose entry to locate or create.
	 * @return A reference to the existing or newly appended entry for Tag.
	 */
	static FNGameplayTagCount& FindOrAddTagCount(TArray<FNGameplayTagCount>& Counter, const FGameplayTag& Tag)
	{
		for (FNGameplayTagCount& Entry : Counter)
		{
			if (Entry.Tag == Tag)
			{
				return Entry;
			}
		}
		FNGameplayTagCount& Added = Counter.AddDefaulted_GetRef();
		Added.Tag = Tag;
		return Added;
	}
};