#include "NWorldAssemblyGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Placement_Start, "NEXUS.WorldAssembly.Placement.Start", 
	"Indicate tagged item can be used at the start of an assembly. If no items are tagged, any can be used, but by having any item tagged Start tagged items will be drawn from.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NWorldAssembly_Placement_End, "NEXUS.WorldAssembly.Placement.End", 
	"Indicate tagged item can be used at the end of an assembly. If no items are tagged, any can be used, but by having any item tagged End tagged items will be drawn from.");