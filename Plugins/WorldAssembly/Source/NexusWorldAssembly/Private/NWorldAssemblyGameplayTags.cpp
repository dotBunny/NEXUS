// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_Starter, "NEXUS.WorldAssembly.Behavior.Starter", 
	"Tagged items can be used at the start of an Organ Assembly as the first placed node in the AssemblyGraph, attached to the Bone. If no Tissue entries are tagged with this (or StarterOnly), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_StarterOnly, "NEXUS.WorldAssembly.Behavior.StarterOnly", 
	"Tagged items can ONLY be used at the start of an Organ Assembly as the first placed node in the AssemblyGraph, attached to the Bone. If no Tissue entries are tagged with this (or Starter), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_NotStarter, "NEXUS.WorldAssembly.Behavior.NotStarter", 
	"Tagged items cannot be used at the start of an Organ Assembly node branch.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_Finisher, "NEXUS.WorldAssembly.Behavior.Finisher", 
	"Tagged items can be used at the end of an Organ Assembly node branch. If no Tissue entries are tagged with this (or FinisherOnly), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_FinisherOnly, "NEXUS.WorldAssembly.Behavior.FinisherOnly", 
	"Tagged items can ONLY be used at the end of an Organ Assembly node branch. If no Tissue entries are tagged with this (or Finisher), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_NotFinisher, "NEXUS.WorldAssembly.Behavior.NotFinisher", 
	"Tagged items cannot be used at the end of an Organ Assembly node branch.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_Unique, "NEXUS.WorldAssembly.Behavior.Unique", 
	"Only one of tagged items could be placed in an Organ Assembly. This is just a built-in default unique group, you can make your own to create discrete groups.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_RequiredAny, "NEXUS.WorldAssembly.Behavior.RequiredAny", 
	"Flags this group that it must be included (at least once) in the graph when generating for it to be validated. When combined with the Unique groups, it will allow the ignoring of MinimumCount (which should still be left at 1) to ensure one from the group is part of the graph.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Behavior_BadNeighbors, "NEXUS.WorldAssembly.Behavior.BadNeighbors", 
	"Cells in bad neighbor groups cannot be placed connected to each other in any way.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Flag_AlwaysRelevant, "NEXUS.WorldAssembly.Flag.AlwaysRelevant", 
	"Cells tagged with AlwaysRelevant will be always considered relevant for networking purposes.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Flag_Hotpath, "NEXUS.WorldAssembly.Flag.Hotpath", 
	"Cells tagged with Hotpath are treated as goals inside of the cell graph, starting with bone following the shortest path to connect all flagged Hotpath items, and connecting to any other bones. INCellInitialized-implementors have access to determine if they are on the Hotpath.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Counter_Alpha, "NEXUS.WorldAssembly.Counter.Alpha", "Built-in counter-based tag (Alpha).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Counter_Beta, "NEXUS.WorldAssembly.Counter.Beta", "Built-in counter-based tag (Beta).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Counter_Charlie, "NEXUS.WorldAssembly.Counter.Charlie", "Built-in counter-based tag (Charlie).");