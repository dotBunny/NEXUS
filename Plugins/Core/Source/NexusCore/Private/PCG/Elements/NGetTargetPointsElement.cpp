// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "PCG/Elements/NGetTargetPointsElement.h"

#include "NColor.h"
#include "PCGContext.h"
#include "PCGGraphExecutionStateInterface.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "NGetTargetPointsElement"

TArray<FPCGPinProperties> UNGetTargetPointsSettings::InputPinProperties() const
{
	// A source node: everything it emits comes from the level, not from an upstream pin.
	return TArray<FPCGPinProperties>();
}

TArray<FPCGPinProperties> UNGetTargetPointsSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return PinProperties;
}

#if WITH_EDITOR
FLinearColor UNGetTargetPointsSettings::GetNodeTitleColor() const
{
	return FNColor::GetElement;
}
#endif

FPCGElementPtr UNGetTargetPointsSettings::CreateElement() const
{
	return MakeShared<FNGetTargetPointsElement>();
}

bool FNGetTargetPointsElement::MatchesTagFilter(const TConstArrayView<FName> ComponentTags, const bool bFilterByTag, const FName Tag)
{
	// With filtering off, or on but with nothing to match against, every component qualifies. Treating an
	// empty tag as "match nothing" would leave the node silently emitting no points the moment the box is
	// ticked, which reads as a broken node rather than as a filter doing its job.
	if (!bFilterByTag || Tag.IsNone())
	{
		return true;
	}

	return ComponentTags.Contains(Tag);
}

bool FNGetTargetPointsElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FNGetTargetPointsElement::Execute);
	check(Context);

	const UNGetTargetPointsSettings* Settings = Context->GetInputSettings<UNGetTargetPointsSettings>();
	check(Settings);

	IPCGGraphExecutionSource* ExecutionSource = Context->ExecutionSource.Get();

	// Prefer the original source. Once a graph is partitioned the executing component lives on a partition
	// actor, whose owner holds none of the markers — the components sit on the actor the graph was authored
	// on. GetOriginalSource returns self when the graph is not partitioned, so this costs nothing there.
	IPCGGraphExecutionSource* OriginalSource = ExecutionSource ? ExecutionSource->GetExecutionState().GetOriginalSource() : nullptr;
	AActor* SourceActor = OriginalSource ? OriginalSource->GetExecutionState().GetTypedTarget<AActor>() : nullptr;
	if (!SourceActor && ExecutionSource)
	{
		SourceActor = ExecutionSource->GetExecutionState().GetTypedTarget<AActor>();
	}

	if (!SourceActor)
	{
		PCGE_LOG(Warning, GraphAndLog, LOCTEXT("NoSourceActor", "No actor to read target point components from."));
		return true;
	}

	// Fall back to the base class rather than gathering nothing, so clearing the class in the details
	// panel does not quietly empty the node's output.
	const TSubclassOf<UNTargetPointComponent> ComponentClass = Settings->ComponentClass
		? Settings->ComponentClass
		: TSubclassOf<UNTargetPointComponent>(UNTargetPointComponent::StaticClass());

	TArray<UActorComponent*> ActorComponents;
	SourceActor->GetComponents(ComponentClass, ActorComponents);

	// Separate "looked at the wrong actor" from "the tag filter excluded everything" — both surface as an
	// empty output, but they are completely different mistakes to go and fix.
	if (ActorComponents.IsEmpty())
	{
		PCGE_LOG(Warning, GraphAndLog, FText::Format(
			LOCTEXT("NoComponentsOnActor", "'{0}' has no '{1}' components."),
			FText::FromString(SourceActor->GetActorNameOrLabel()),
			FText::FromString(GetNameSafe(ComponentClass))));
	}

	TArray<FTransform> Transforms;
	Transforms.Reserve(ActorComponents.Num());
	for (const UActorComponent* ActorComponent : ActorComponents)
	{
		const USceneComponent* SceneComponent = Cast<const USceneComponent>(ActorComponent);
		if (!SceneComponent || !MatchesTagFilter(SceneComponent->ComponentTags, Settings->bFilterByTag, Settings->ComponentTag))
		{
			continue;
		}

		Transforms.Add(SceneComponent->GetComponentTransform());
	}

	if (!ActorComponents.IsEmpty() && Transforms.IsEmpty())
	{
		PCGE_LOG(Warning, GraphAndLog, FText::Format(
			LOCTEXT("AllComponentsFiltered", "All {0} '{1}' components on '{2}' were excluded by the tag filter."),
			FText::AsNumber(ActorComponents.Num()),
			FText::FromString(GetNameSafe(ComponentClass)),
			FText::FromString(SourceActor->GetActorNameOrLabel())));
	}
	else
	{
		PCGE_LOG(Verbose, LogOnly, FText::Format(
			LOCTEXT("GatheredPoints", "Gathered {0} of {1} components from '{2}'."),
			FText::AsNumber(Transforms.Num()),
			FText::AsNumber(ActorComponents.Num()),
			FText::FromString(SourceActor->GetActorNameOrLabel())));
	}

	// Emit the data even when nothing matched, so downstream sees an empty point set rather than an
	// unconnected pin — the two behave very differently once they reach a falloff or a spawner.
	UPCGBasePointData* PointData = FPCGContext::NewPointData_AnyThread(Context);
	PointData->SetNumPoints(Transforms.Num());
	PointData->AllocateProperties(EPCGPointNativeProperties::Transform);

	TPCGValueRange<FTransform> TransformRange = PointData->GetTransformValueRange(/*bAllocate=*/false);
	for (int32 Index = 0; Index < Transforms.Num(); ++Index)
	{
		TransformRange[Index] = Transforms[Index];
	}

	Context->OutputData.TaggedData.Emplace_GetRef().Data = PointData;

	return true;
}

#undef LOCTEXT_NAMESPACE
