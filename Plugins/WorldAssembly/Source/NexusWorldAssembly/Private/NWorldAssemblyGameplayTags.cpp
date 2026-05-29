// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_Starter, "NEXUS.WorldAssembly.BuiltIn.Starter", 
	"Tagged items can be used at the start of an Organ Assembly as the first placed node in the AssemblyGraph, attached to the Bone. If no Tissue entries are tagged with this (or StarterOnly), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_StarterOnly, "NEXUS.WorldAssembly.BuiltIn.StarterOnly", 
	"Tagged items can ONLY be used at the start of an Organ Assembly as the first placed node in the AssemblyGraph, attached to the Bone. If no Tissue entries are tagged with this (or Starter), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_NotStarter, "NEXUS.WorldAssembly.BuiltIn.NotStarter", 
	"Tagged items cannot be used at the start of an Organ Assembly node branch.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_Finisher, "NEXUS.WorldAssembly.BuiltIn.Finisher", 
	"Tagged items can be used at the end of an Organ Assembly node branch. If no Tissue entries are tagged with this (or FinisherOnly), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_FinisherOnly, "NEXUS.WorldAssembly.BuiltIn.FinisherOnly", 
	"Tagged items can ONLY be used at the end of an Organ Assembly node branch. If no Tissue entries are tagged with this (or Finisher), any can be used instead in their place.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_NotFinisher, "NEXUS.WorldAssembly.BuiltIn.NotFinisher", 
	"Tagged items cannot be used at the end of an Organ Assembly node branch.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_Unique, "NEXUS.WorldAssembly.BuiltIn.Unique", 
	"Only one of tagged items could be placed in an Organ Assembly. This is just a built-in default unique group, you can make your own to create discrete groups.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_BuiltIn_RequiredAny, "NEXUS.WorldAssembly.BuiltIn.RequiredAny", 
	"Flags this group that it must be included (at least once) in the graph when generating for it to be validated. When combined with the Unique groups, it will allow the ignoring of MinimumCount (which should still be left at 1) to ensure one from the group is part of the graph.");