// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdMode.h"

#include "Editor.h"
#include "NActorUtils.h"
#include "NCanvasUtils.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "Cell/NCellRootComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyDebugDraw.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyEdModeToolkit.h"
#include "NWorldAssemblyEdModeToolCommands.h"
#include "BaseGizmos/TransformGizmoUtil.h"
#include "InteractiveToolManager.h"
#include "Tools/NCellBoundsTool.h"
#include "Tools/NCellHullSplitTool.h"
#include "Tools/NCellHullVertexTool.h"
#include "Tools/NCellVoxelTool.h"
#include "Tools/NJunctionPlacementTool.h"
#include "Developer/NPrimitiveFont.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyUtils.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Macros/NFlagsMacros.h"
#include "Math/NBoxUtils.h"
#include "Math/NVectorUtils.h"
#include "Assembly/NAssemblyOperation.h"
#include "Developer/NMethodScopeTimer.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "UObject/UnrealType.h"

namespace
{
	/** Fallbacks handed back by the cached-geometry accessors when there is no active mode to read from. */
	const TArray<FVector> EmptyVertices;
	const TArray<FIntVector2> EmptyEdges;
	const FNCellVoxelData EmptyVoxelData;
}

UNWorldAssemblyEdMode* UNWorldAssemblyEdMode::Get()
{
	return Cast<UNWorldAssemblyEdMode>(GLevelEditorModeTools().GetActiveScriptableMode(Identifier));
}

FBox UNWorldAssemblyEdMode::GetCachedBounds()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CachedBounds : FBox(ForceInit);
}

const TArray<FVector>& UNWorldAssemblyEdMode::GetCachedBoundsVertices()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CachedBoundsVertices : EmptyVertices;
}

const FNCellVoxelData& UNWorldAssemblyEdMode::GetCachedVoxelData()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CachedVoxelData : EmptyVoxelData;
}

const TArray<FVector>& UNWorldAssemblyEdMode::GetCachedHullVertices()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CachedHullVertices : EmptyVertices;
}

const TArray<FIntVector2>& UNWorldAssemblyEdMode::GetCachedHullEdges()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CachedHullEdges : EmptyEdges;
}

ANCellActor* UNWorldAssemblyEdMode::GetCellActor()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CellActor.Get() : nullptr;
}

UNWorldAssemblyEdMode::ENCellEdMode UNWorldAssemblyEdMode::GetCellEdMode()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CellEdMode : ENCellEdMode::Bounds;
}

void UNWorldAssemblyEdMode::SetCellEdMode(const ENCellEdMode InCellEdMode)
{
	UNWorldAssemblyEdMode* Mode = Get();
	if (Mode == nullptr || Mode->CellEdMode == InCellEdMode) return;

	// No selection to clear here any more: each tool owns its own vertex/edge selection and drops it on shutdown, and
	// switching sub-mode is now a consequence of starting a different tool rather than something done behind its back.
	Mode->CellEdMode = InCellEdMode;
}

UNWorldAssemblyEdMode::ENCellVoxelMode UNWorldAssemblyEdMode::GetCellVoxelMode()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr ? Mode->CellVoxelMode : ENCellVoxelMode::None;
}

void UNWorldAssemblyEdMode::SetCellVoxelMode(const ENCellVoxelMode InCellVoxelMode)
{
	if (UNWorldAssemblyEdMode* Mode = Get())
	{
		Mode->CellVoxelMode = InCellVoxelMode;
	}
}

bool UNWorldAssemblyEdMode::HasCollisionVisualizer()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	return Mode != nullptr && Mode->CollisionVisualizer != nullptr;
}

void UNWorldAssemblyEdMode::SetRenderMode(const ENWorldAssemblyEdModeRenderMode InRenderMode)
{
	if (UNWorldAssemblyEdMode* Mode = Get())
	{
		Mode->RenderMode = InRenderMode;
	}
}

FText UNWorldAssemblyEdMode::GetWarningText()
{
	const UNWorldAssemblyEdMode* Mode = Get();
	if (Mode == nullptr) return FText::GetEmpty();

	TArray<FText> Lines;

	if (Mode->bHasDirtyActors)
	{
		Lines.Add(DirtyMessage);
	}

	// Matches the precedence the HUD text used: the combined bounds+hull line stands in for both, and the voxel line
	// only surfaces once neither of those applies.
	if (Mode->bAutoBoundsDisabled && Mode->bAutoHullDisabled)
	{
		Lines.Add(AutoBoundsHullMessage);
	}
	else if (Mode->bAutoBoundsDisabled)
	{
		Lines.Add(AutoBoundsMessage);
	}
	else if (Mode->bAutoHullDisabled)
	{
		Lines.Add(AutoHullMessage);
	}
	else if (Mode->bAutoVoxelDisabled)
	{
		Lines.Add(AutoVoxelMessage);
	}

	return Lines.IsEmpty() ? FText::GetEmpty() : FText::Join(FText::FromString(TEXT("\n")), Lines);
}

void UNWorldAssemblyEdMode::ProtectCellEdMode()
{
	UNWorldAssemblyEdMode* Mode = Get();
	if (Mode == nullptr) return;

	if (const ANCellActor* Actor = Mode->CellActor.Get();
		Actor != nullptr && Actor->GetCellRoot()->Details.Hull.HasNonTris() &&
		Mode->CellEdMode == ENCellEdMode::Hull)
	{
		SetCellEdMode(ENCellEdMode::Bounds);
	}
}

void UNWorldAssemblyEdMode::EndActiveTool()
{
	UNWorldAssemblyEdMode* Mode = Get();
	if (Mode == nullptr) return;

	UEditorInteractiveToolsContext* ToolsContext = Mode->GetInteractiveToolsContext();
	if (ToolsContext == nullptr || !ToolsContext->CanCompleteActiveTool()) return;

	ToolsContext->EndTool(EToolShutdownType::Completed);
}

void UNWorldAssemblyEdMode::OnActorDeleted(AActor* Actor)
{
	if (Actor == CellActor.Get())
	{
		CellActor.Reset();
	}
	if (Actor == CollisionVisualizer)
	{
		// The visualizer itself was removed (e.g. deleted by the user) — stop listening and clear our state.
		UnbindWorldChangeDelegates();
		CollisionSourceActors.Reset();
		bCollisionVisualizerDirty = false;
		CollisionVisualizer = nullptr;
	}
	else if (CollisionVisualizer != nullptr && CollisionSourceActors.Contains(FObjectKey(Actor)))
	{
		// A source actor was deleted; it can no longer pass the live filter (it's pending kill), so the source-set
		// membership is what tells us the visualizer needs rebuilding.
		MarkCollisionVisualizerDirty();
	}
}

TObjectPtr<ANDebugActor> UNWorldAssemblyEdMode::CreateCollisionVisualizer(UWorld* World)
{
	UNWorldAssemblyEdMode* Mode = Get();
	if (Mode == nullptr) return nullptr;

	return Mode->RefreshCollisionVisualizer(World);
}

void UNWorldAssemblyEdMode::DestroyCollisionVisualizer()
{
	if (UNWorldAssemblyEdMode* Mode = Get())
	{
		Mode->TearDownCollisionVisualizer();
	}
}

TObjectPtr<ANDebugActor> UNWorldAssemblyEdMode::RefreshCollisionVisualizer(UWorld* World)
{
	const bool bWasAlive = CollisionVisualizer != nullptr;

	// Only time the initial build; in-place refreshes are frequent and would otherwise spam the log.
	TOptional<FNMethodScopeTimer> Timer;
	if (!bWasAlive)
	{
		Timer.Emplace(TEXT("World Collision Build Time"));
	}

	TArray<AActor*> SourceActors;
	CollisionVisualizer = FNWorldAssemblyEditorUtils::RefreshWorldCollisionVisualizerActor(
		World, TArray<FBoxSphereBounds>(), CollisionVisualizer, SourceActors);

	bCollisionVisualizerDirty = false;

	if (CollisionVisualizer == nullptr)
	{
		// Nothing was spawned (no geometry / no material) — nothing to track or listen for.
		CollisionSourceActors.Reset();
		return nullptr;
	}

	// Refresh the source set used to test relevance of future world changes.
	CollisionSourceActors.Reset();
	CollisionSourceActors.Reserve(SourceActors.Num());
	for (const AActor* SourceActor : SourceActors)
	{
		CollisionSourceActors.Add(FObjectKey(SourceActor));
	}

	// Start listening only once a visualizer is actually alive.
	if (!bWasAlive)
	{
		BindWorldChangeDelegates();
	}

	return CollisionVisualizer;
}

void UNWorldAssemblyEdMode::TearDownCollisionVisualizer()
{
	UnbindWorldChangeDelegates();
	CollisionSourceActors.Reset();
	bCollisionVisualizerDirty = false;

	if (CollisionVisualizer != nullptr)
	{
		if (CollisionVisualizer->IsSelected())
		{
			GEditor->SelectActor(CollisionVisualizer, false, false);
		}
		CollisionVisualizer->GetWorld()->DestroyActor(CollisionVisualizer, false, false);
		CollisionVisualizer = nullptr;
	}
}

void UNWorldAssemblyEdMode::BindWorldChangeDelegates()
{
	// AddUObject rather than the AddStatic these used while the handlers were static: the binding is now tied to this
	// mode's lifetime, so a mode torn down without reaching UnbindWorldChangeDelegates unbinds itself rather than
	// leaving a delegate pointing at a dead object.
	if (!OnLevelActorAddedHandle.IsValid())
	{
		OnLevelActorAddedHandle = GEngine->OnLevelActorAdded().AddUObject(this, &UNWorldAssemblyEdMode::OnLevelActorAdded);
	}
	if (!OnObjectMovedHandle.IsValid())
	{
		OnObjectMovedHandle = GEditor->OnEndObjectMovement().AddUObject(this, &UNWorldAssemblyEdMode::OnObjectMoved);
	}
	if (!OnObjectPropertyChangedHandle.IsValid())
	{
		OnObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UNWorldAssemblyEdMode::OnObjectPropertyChanged);
	}
	if (!OnUndoRedoHandle.IsValid())
	{
		OnUndoRedoHandle = FEditorDelegates::PostUndoRedo.AddUObject(this, &UNWorldAssemblyEdMode::OnUndoRedo);
	}
}

void UNWorldAssemblyEdMode::UnbindWorldChangeDelegates()
{
	if (OnLevelActorAddedHandle.IsValid())
	{
		GEngine->OnLevelActorAdded().Remove(OnLevelActorAddedHandle);
		OnLevelActorAddedHandle.Reset();
	}
	if (OnObjectMovedHandle.IsValid())
	{
		GEditor->OnEndObjectMovement().Remove(OnObjectMovedHandle);
		OnObjectMovedHandle.Reset();
	}
	if (OnObjectPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnObjectPropertyChangedHandle);
		OnObjectPropertyChangedHandle.Reset();
	}
	if (OnUndoRedoHandle.IsValid())
	{
		FEditorDelegates::PostUndoRedo.Remove(OnUndoRedoHandle);
		OnUndoRedoHandle.Reset();
	}
}

bool UNWorldAssemblyEdMode::ShouldRebuildForActor(const AActor* Actor) const
{
	if (Actor == nullptr || CollisionVisualizer == nullptr) return false;

	// Was it part of the geometry we last built? (covers delete / collision-off / ignore-tag-added transitions)
	if (CollisionSourceActors.Contains(FObjectKey(Actor))) return true;

	// Is it relevant now? (covers add / collision-on transitions) — same predicate the visualizer build uses.
	return FNActorUtils::PassesFilter(Actor, FNCreateVirtualWorldTask::CreateWorldActorFilterSettings(UNWorldAssemblySettings::Get()->WorldCollisionSettings));
}

AActor* UNWorldAssemblyEdMode::ResolveAffectedActor(UObject* Object)
{
	if (Object == nullptr) return nullptr;
	if (AActor* Actor = Cast<AActor>(Object)) return Actor;
	if (const UActorComponent* Component = Cast<UActorComponent>(Object)) return Component->GetOwner();
	return nullptr;
}

void UNWorldAssemblyEdMode::OnLevelActorAdded(AActor* Actor)
{
	if (ShouldRebuildForActor(Actor))
	{
		MarkCollisionVisualizerDirty();
	}
}

void UNWorldAssemblyEdMode::OnObjectMoved(UObject& Object)
{
	if (ShouldRebuildForActor(ResolveAffectedActor(&Object)))
	{
		MarkCollisionVisualizerDirty();
	}
}

void UNWorldAssemblyEdMode::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Ignore the continuous mid-edit stream (slider scrubs, gizmo drags); we rebuild on the finalizing change instead.
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive) return;

	if (ShouldRebuildForActor(ResolveAffectedActor(Object)))
	{
		MarkCollisionVisualizerDirty();
	}
}

void UNWorldAssemblyEdMode::OnUndoRedo()
{
	if (CollisionVisualizer != nullptr)
	{
		MarkCollisionVisualizerDirty();
	}
}

const FEditorModeID UNWorldAssemblyEdMode::Identifier = TEXT("NWorldAssemblyEdMode");
const FText UNWorldAssemblyEdMode::DirtyMessage = FText::FromString("Dirty Cell Actor");
const FText UNWorldAssemblyEdMode::AutoBoundsMessage = FText::FromString("Cell Bounds not calculated on save.");
const FText UNWorldAssemblyEdMode::AutoBoundsHullMessage = FText::FromString("Cell Bounds and Hull not calculated on save.");
const FText UNWorldAssemblyEdMode::AutoHullMessage = FText::FromString("Cell Hull not calculated on save.");
const FText UNWorldAssemblyEdMode::AutoVoxelMessage = FText::FromString("Cell Voxel not calculated on save.");

UNWorldAssemblyEdMode::UNWorldAssemblyEdMode()
{
	// A UEdMode carries its own registration data rather than being handed it by the caller, so what used to be
	// arguments to FEditorModeRegistry::RegisterMode live here. The registry discovers the mode from its UCLASS.
	Info = FEditorModeInfo(
		Identifier,
		NSLOCTEXT("NexusWorldAssemblyEditor", "UNWorldAssemblyEdMode", "World Assembly"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"),
		true);

	// Names the multibox the toolkit's palettes are built into, which is what lets the editor persist per-user
	// customization of them. Not a constructor argument — FEditorModeInfo leaves it default and expects the mode to
	// fill it in — but FToolkitBuilderArgs requires one, so it is set here alongside the rest of the registration data.
	Info.ToolbarCustomizationName = TEXT("NWorldAssemblyEdModeToolbar");
}

void UNWorldAssemblyEdMode::CreateToolkit()
{
	Toolkit = MakeShared<FNWorldAssemblyEdModeToolkit>();
}

void UNWorldAssemblyEdMode::BindCommands()
{
	Super::BindCommands();

	// RegisterTool maps each command into the toolkit's command list — the same list the rail palettes resolve their
	// buttons against — so the palette entries added in FNWorldAssemblyEdModeToolkit::RegisterPalettes light up from
	// here even though the toolkit was built first.
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	RegisterToggleableTool(ToolCommands.BeginCellBoundsTool, NEXUS::WorldAssembly::Tools::CellBounds,
		NewObject<UNCellBoundsToolBuilder>(this));
	RegisterToggleableTool(ToolCommands.BeginCellHullVertexTool, NEXUS::WorldAssembly::Tools::CellHullVertex,
		NewObject<UNCellHullVertexToolBuilder>(this));
	RegisterToggleableTool(ToolCommands.BeginCellHullSplitTool, NEXUS::WorldAssembly::Tools::CellHullSplit,
		NewObject<UNCellHullSplitToolBuilder>(this));
	RegisterToggleableTool(ToolCommands.BeginCellVoxelTool, NEXUS::WorldAssembly::Tools::CellVoxel,
		NewObject<UNCellVoxelToolBuilder>(this));
	RegisterToggleableTool(ToolCommands.BeginJunctionPlacementTool, NEXUS::WorldAssembly::Tools::JunctionPlacement,
		NewObject<UNJunctionPlacementToolBuilder>(this));
}

void UNWorldAssemblyEdMode::RegisterToggleableTool(const TSharedPtr<FUICommandInfo>& UICommand,
	const FString& ToolIdentifier, UInteractiveToolBuilder* Builder)
{
	// Register normally first: this is what makes the tool type known to the manager, which the re-mapped action
	// below still relies on to start it.
	RegisterTool(UICommand, ToolIdentifier, Builder);

	if (!Toolkit.IsValid()) return;

	UEditorInteractiveToolsContext* ToolsContext = GetInteractiveToolsContext();
	if (ToolsContext == nullptr) return;

	const TSharedRef<FUICommandList>& CommandList = Toolkit->GetToolkitCommands();
	CommandList->UnmapAction(UICommand);
	CommandList->MapAction(UICommand,
		FExecuteAction::CreateWeakLambda(ToolsContext, [ToolsContext, ToolIdentifier]()
		{
			if (ToolsContext->IsToolActive(EToolSide::Mouse, ToolIdentifier))
			{
				// Completed, not Cancelled: these tools have already committed every edit, so there is nothing to roll
				// back and asking for a cancel would misreport what happened to anything listening.
				ToolsContext->EndTool(EToolShutdownType::Completed);
				return;
			}
			ToolsContext->StartTool(ToolIdentifier);
		}),
		FCanExecuteAction::CreateWeakLambda(ToolsContext, [this, ToolsContext, ToolIdentifier]()
		{
			// An active tool can always be switched off; an inactive one has to pass the usual start checks.
			return ToolsContext->IsToolActive(EToolSide::Mouse, ToolIdentifier)
				|| (ShouldToolStartBeAllowed(ToolIdentifier)
					&& ToolsContext->ToolManager->CanActivateTool(EToolSide::Mouse, ToolIdentifier));
		}),
		FIsActionChecked::CreateUObject(ToolsContext, &UEdModeInteractiveToolsContext::IsToolActive, EToolSide::Mouse, ToolIdentifier),
		EUIActionRepeatMode::RepeatDisabled);
}

bool UNWorldAssemblyEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::Escape && Event == IE_Pressed)
	{
		if (const UEditorInteractiveToolsContext* ToolsContext = GetInteractiveToolsContext();
			ToolsContext != nullptr && ToolsContext->CanCompleteActiveTool())
		{
			EndActiveTool();

			// Handled only when there was a tool to end, so escape keeps falling through to the level editor's own
			// handling — clearing the selection — the rest of the time.
			return true;
		}
	}

	return Super::InputKey(ViewportClient, Viewport, Key, Event);
}

void UNWorldAssemblyEdMode::BeginDestroy()
{
	// Exit() already does this on every ordinary deactivation; this is the backstop for a mode torn down without one,
	// which under FEdMode was the destructor's job.
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->TearDownOperation();
		OrganGenerator = nullptr;
	}

	Super::BeginDestroy();
}

void UNWorldAssemblyEdMode::Enter()
{
	// Reset the cached state. Still required even though this is instance state now: FEditorModeTools recycles mode
	// objects (RecycledScriptableModes), so re-entering the mode hands back the same instance carrying whatever the
	// previous session left on it.
	CellActor = nullptr;
	CachedHullVertices.Empty();
	CachedHullEdges.Empty();
	CachedBounds = FBox(ForceInit);
	CachedVoxelData = FNCellVoxelData();
	CachedBoundsVertices.Empty();
	RenderMode = ENWorldAssemblyEdModeRenderMode::All;

	FNWorldAssemblyEditorColors::Refresh();

	bCanTick = true;

	// Create our temp organ generator to use with any selections
	OrganGenerator = NewObject<UNAssemblyOperation>(GetTransientPackage(), NEXUS::WorldAssembly::Operations::EditorMode);
	OrganGenerator->DisplayName = FText::FromName(NEXUS::WorldAssembly::Operations::EditorMode);

	OnLevelActorDeletedHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &UNWorldAssemblyEdMode::OnActorDeleted);

	Super::Enter();

	// Put a UCombinedTransformGizmoContextObject in this mode's tools context. Nothing supplies one by default —
	// FEditorModeTools registers the *editor* gizmo context (UEditorTransformGizmoContextObject) instead, which is a
	// different type — so without this every UE::TransformGizmoUtil::Create*TransformGizmo call quietly returns
	// nullptr, and the tools that build gizmos in Setup crash on the result. Modeling Mode registers it the same way.
	//
	// Deliberately after Super::Enter, which is what constructs the tools context (CreateInteractiveToolsContexts):
	// registered any earlier there is nothing to register into, and the call ensure-fails on a null context. The tools
	// themselves only ask for a gizmo when one is started, so registering this late is still well ahead of any use.
	if (UEditorInteractiveToolsContext* ToolsContext = GetInteractiveToolsContext())
	{
		UE::TransformGizmoUtil::RegisterTransformGizmoContextObject(ToolsContext);
	}
}

void UNWorldAssemblyEdMode::Exit()
{
	CellActor = nullptr;
	bCanTick = false;

	GEngine->OnLevelActorDeleted().Remove(OnLevelActorDeletedHandle);

	// Balances the registration in Enter, and before Super::Exit for the same reason that one sits after Super::Enter:
	// the base is what destroys the tools context. The mode object is recycled between activations, so keeping this
	// paired stops a re-entry inheriting anything stale.
	if (UEditorInteractiveToolsContext* ToolsContext = GetInteractiveToolsContext())
	{
		UE::TransformGizmoUtil::DeregisterTransformGizmoContextObject(ToolsContext);
	}

	// Destroy any visualizer kicking around. Deliberately the instance method rather than the static facade: the mode
	// manager drops us from its active list before calling Exit, so Get() would already return nullptr here and the
	// facade would quietly no-op, leaking the visualizer actor into the level.
	TearDownCollisionVisualizer();

	// Remove our temp organ generator
	if (OrganGenerator != nullptr)
	{
		OrganGenerator->TearDownOperation();
		OrganGenerator = nullptr;
	}

	Super::Exit();
}

void UNWorldAssemblyEdMode::ModeTick(float DeltaTime)
{
	if (bCanTick == false) return;


	// Coalesce any world changes flagged since the last tick into a single in-place rebuild of the visualizer.
	if (bCollisionVisualizerDirty && CollisionVisualizer != nullptr)
	{
		if (UWorld* VisualizerWorld = CollisionVisualizer->GetWorld())
		{
			RefreshCollisionVisualizer(VisualizerWorld);
		}
		bCollisionVisualizerDirty = false;
	}

	// Resolve the cell actor for the active world. Reuse the cached pointer while it's still alive and belongs to
	// that world; only fall back to the full GetCellActorFromWorld level/actor scan when it's gone. Deletion of the
	// cached actor is handled by OnActorDeleted (it clears the pointer), a world switch is caught by the world
	// compare, and while no cell actor exists the scan repeats each tick — which is also what lets us pick one up
	// once it's added.
	const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	if (CurrentWorld == nullptr)
	{
		CellActor.Reset();
	}
	else
	{
		if (const ANCellActor* Cached = CellActor.Get(); Cached == nullptr || Cached->GetWorld() != CurrentWorld)
		{
			CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
		}

		if (ANCellActor* Actor = CellActor.Get(); Actor != nullptr && !Actor->WasSpawnedFromProxy())
		{
			UNCellRootComponent* RootComponent = Actor->GetCellRoot();
			const FRotator Rotation = RootComponent->GetOffsetRotator();
			const FVector Offset = RootComponent->GetOffsetLocation();

			// The hull, bounds and voxel caches are each consumed only by NCellRootComponentVisualizer, and only in
			// their matching ed-mode (hull points/edges in Hull, min/max in Bounds, the grid overlay in Voxel). Only
			// the active mode's cache is ever read, so refresh just that one rather than rebuilding all of them every
			// tick. Hull and bounds derive from the live offset transform, so they recompute each tick while active to
			// track viewport drags.
			switch (CellEdMode)
			{
			case ENCellEdMode::Hull:
				CachedHullVertices = FNVectorUtils::RotateAndOffsetPoints(RootComponent->Details.Hull.Vertices, Rotation, Offset);
				CachedHullEdges = RootComponent->Details.Hull.GetEdgeIndices();
				break;
			case ENCellEdMode::Bounds:
				CachedBounds = FNWorldAssemblyUtils::CreateRotatedBox(RootComponent->Details.Bounds, Rotation, Offset);
				CachedBoundsVertices = FNBoxUtils::GetVertices(CachedBounds);
				break;
			case ENCellEdMode::Voxel:
				// The IsEqual guard skips the array copy when the source grid is unchanged (covering in-place voxel
				// edits that keep the count) and the Origin guard catches a re-anchored grid whose contents match.
				if (const FNCellVoxelData& SourceVoxelData = RootComponent->Details.VoxelData;
					!CachedVoxelData.IsEqual(SourceVoxelData) || CachedVoxelData.GetOrigin() != SourceVoxelData.GetOrigin())
				{
					CachedVoxelData = SourceVoxelData;
				}
				break;
			}

			bAutoBoundsDisabled = !RootComponent->Details.BoundsSettings.bCalculateOnSave;
			bAutoHullDisabled = !RootComponent->Details.HullSettings.bCalculateOnSave;
			bAllowNonConvexHull = RootComponent->Details.HullSettings.bAllowNonConvex;
			bAutoVoxelDisabled = (!RootComponent->Details.VoxelSettings.bCalculateOnSave && RootComponent->Details.VoxelSettings.bUseVoxelData);
		}
	}

	Super::ModeTick(DeltaTime);
}

void UNWorldAssemblyEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	bHasDirtyActors = false;

	// We don't have anything to do in play mode - maybe in the future.
	if (FNEditorUtils::IsPlayInEditor() ||
		RenderMode == ENWorldAssemblyEdModeRenderMode::None ||
		RenderMode == ENWorldAssemblyEdModeRenderMode::LevelScreenshot)
	{
		Super::Render(View, Viewport, PDI);
		return;
	}

	// Iterate all roots and draw their bounds
	if (FNWorldAssemblyRegistry::HasRootComponents())
	{
		for (const auto RootComponent : FNWorldAssemblyRegistry::GetCellRootComponents())
		{
			if (RootComponent == nullptr) continue;

			const ANCellActor* Actor = RootComponent->GetNCellActor();
			if (Actor != nullptr)
			{
				// Do not draw cell actors when in editor mode when spawned from proxy
				if (Actor->WasSpawnedFromProxy())
				{
					continue;
				}

				// Notice ON Dirty
				if (Actor->IsActorDirty())
				{
					bHasDirtyActors = true;
				}
			}

			// Draw debug information
			RootComponent->DrawDebugPDI(PDI, static_cast<uint8>(CellVoxelMode),
				FNWorldAssemblyEditorColors::GetCellBounds(), FNWorldAssemblyEditorColors::GetCellHull());
			// We can't use caching because we are drawing ALL the possible roots
		}
	}
	if (FNWorldAssemblyRegistry::HasJunctionComponents() || FNWorldAssemblyRegistry::HasBoneComponents())
	{
		const UNWorldAssemblySettings* WorldAssemblySettings = UNWorldAssemblySettings::Get();
		const UNWorldAssemblyEditorUserSettings* WorldAssemblyEditorUserSettings = UNWorldAssemblyEditorUserSettings::Get();

		// Draw Junctions
		for (const auto JunctionComponent : FNWorldAssemblyRegistry::GetCellJunctionComponents())
		{
			// Bad ref?
			if (JunctionComponent == nullptr)
			{
				continue;
			}

			FNCellLinkDetails& LinkDetails = JunctionComponent->LinkDetails;
			// Author-time
			if (LinkDetails.JunctionInstanceIdentifier == -1)
			{
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsInvalid,
					true,
					false,
					true,
					true,
					WorldAssemblySettings);
				continue;
			}

			// Runtime Connected, by a routed connector rather than by mating directly with another cell. Tested
			// ahead of bConnected because a connector pairing sets both, and it is the more specific state — the
			// socket is spanned by geometry that arrives separately, not closed off against a neighboring cell.
			if (LinkDetails.bConnector)
			{
				// Both ends draw, unlike the mated case below. A mated pair has its two sockets co-located, so that
				// branch elects a single drawer to avoid rendering the same rectangle twice; a connector pair is
				// specifically two sockets with open space between them, so electing one would leave the other end
				// invisible. (The election would also be meaningless here: node identifiers restart per graph, and a
				// connector pair can span graphs, so both ends can hold the same one.)
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					false, true, true, true,
					WorldAssemblySettings);
				continue;
			}

			// Runtime Connected
			if (LinkDetails.bConnected)
			{
				// A mated pair has its two sockets co-located, so one end is elected to draw the rectangle rather than
				// both rendering it twice. Node identifiers are unique within a graph but restart between them, so an
				// inverse mating that spans two graphs can hold the same one on both ends — and a strict comparison
				// would then elect neither, leaving a visible hole. Ties draw twice instead, which on two identical
				// co-located rectangles looks the same as drawing once.
				const bool bConnectedDrawer = LinkDetails.ConnectedNodeIdentifier >= LinkDetails.NodeIdentifier;
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsValid,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsInvalid,
					false,true,bConnectedDrawer, bConnectedDrawer,
					WorldAssemblySettings);
				continue;
			}

			// Runtime Disconnected
			if (WorldAssemblyEditorUserSettings->bDebugWorldDrawUnfilledJunctions)
			{
				JunctionComponent->DrawDebugPDI(PDI,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsUnfilled,
					WorldAssemblyEditorUserSettings->ColorPaletteJunctionsUnfilled,
					false,false,true, false,
					WorldAssemblySettings);
			}
		}

		// We draw the bones in world mode without socket, so that they show up in edit mode of generated stuff
		for (const auto BoneComponent : FNWorldAssemblyRegistry::GetBoneComponents())
		{
			BoneComponent->DrawDebugPDI(PDI, WorldAssemblyEditorUserSettings->ColorPaletteBonesValid, WorldAssemblyEditorUserSettings->ColorPaletteBonesInvalid, false, false);
		}
	}

	// Draw the routes the connector pass proved clear. Deliberately outside the junction-component block above: the
	// routes come from the editor subsystem's own cache rather than the registry, so they draw in the default
	// proxy-only preview too — which is exactly the case with no junction components to hang them off.
	if (UNWorldAssemblyEditorUserSettings::Get()->bDebugWorldDrawJunctionConnectors)
	{
		const UWorld* CurrentWorld = GetWorld();
		for (const TPair<int32, UNWorldAssemblyEditorSubsystem::FNGeneratedConnections>& Entry :
			UNWorldAssemblyEditorSubsystem::Get()->GetGeneratedConnections())
		{
			// Routes are world-space points with no object references, so unlike the proxies they would happily
			// outlive their world; skipping on a mismatch is what keeps one world's routes out of another's viewport.
			if (Entry.Value.World.Get() != CurrentWorld) continue;

			for (const FNCellJunctionConnection& Connection : Entry.Value.Connections)
			{
				// The center curve shares the valid-junction color with the sockets it runs between, so the route
				// reads as one continuous connection; the corner curves keep their own color to stay legible as the
				// bounding volume rather than part of that line.
				FNWorldAssemblyDebugDraw::DrawConnectorPath(PDI, Connection.Path,
					FNWorldAssemblyEditorColors::GetJunctionValid(), FNWorldAssemblyEditorColors::GetJunctionConnectorCorners());
			}
		}
	}

	// Selection-specific drawing options
	const ENWorldAssemblySelectionFlags Flags = FNWorldAssemblyEditorUtils::GetSelectionFlags();
	if (N_FLAGS_UINT8_HAS_UINT8(Flags, ENWorldAssemblySelectionFlags::OrganVolume))
	{
		TArray<ANOrganVolume*> SelectedOrganVolumes = FNWorldAssemblyEditorUtils::GetSelectedOrganVolumes();

		// Ensure we only process organ selection when it has changed.
		if (const uint32 NewSelectedOrganHash = FNArrayUtils::GetPointersHash(SelectedOrganVolumes);
			NewSelectedOrganHash != PreviousSelectedOrganHash)
		{
			OrganGenerator->Reset();
			for (const ANOrganVolume* OrganVolume : SelectedOrganVolumes)
			{
				OrganGenerator->AddToContext(OrganVolume->GetOrganComponent());
			}
			OrganGenerator->LockContext(FNEditorUtils::GetCurrentWorld()); // We need the context locked to figure out the actual ordering

			PreviousSelectedOrganHash = NewSelectedOrganHash;
		}

		if (OrganGenerator->IsLocked())
		{
			TArray<TArray<TObjectPtr<UNOrganComponent>>>& Order = OrganGenerator->GetGenerationOrder();
			// #SONARQUBE-DISABLE-CPP_S134 Need the extra depth to iterate
			for (int32 i = 0; i < Order.Num(); i++)
			{
				for (int32 p = 0; p < Order[i].Num(); p++)
				{
					Order[i][p]->DrawDebugPDI(PDI);

					FString Label = FString::Printf(TEXT(" %i:%i %s"), i, p, *Order[i][p]->GetDebugLabel());

					FNPositionRotation LabelOrientation = Order[i][p]->GetDebugLabelPositionRotation();
					FNPrimitiveFont::DrawPDI(PDI, Label,
					 	LabelOrientation.Position, LabelOrientation.Rotation, FLinearColor::White);
				}
			}
			// #SONARQUBE-ENABLE
		}
	}
	else if ( OrganGenerator->IsLocked())
	{
		OrganGenerator->Reset();
		PreviousSelectedOrganHash = 0;
	}

	Super::Render(View, Viewport, PDI);
}

