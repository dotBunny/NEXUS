#include "NProcGenEditorSubsystem.h"

#include "Organ/NOrganGenerator.h"

void UNProcGenEditorSubsystem::BuildGenerator(UNOrganGenerator* OrganGenerator)
{
	ActiveGenerators.AddUnique(OrganGenerator);
	OrganGenerator->StartBuild(this);
}

void UNProcGenEditorSubsystem::OnFinishedBuild(UNOrganGenerator* OrganGenerator)
{
	ActiveGenerators.Remove(OrganGenerator);
}