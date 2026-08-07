// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorCommands.h"

#include "LevelEditor.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorUtils.h"
#include "Commands/NWorldAssemblyEditorCellCommands.h"
#include "Commands/NWorldAssemblyEditorJunctionCommands.h"
#include "Commands/NWorldAssemblyEditorOrganCommands.h"
#include "Commands/NWorldAssemblyEditorQuickAssemblyCommands.h"
#include "Commands/NWorldAssemblyEditorWorldCommands.h"

void FNWorldAssemblyEditorCommands::RegisterCommands()
{
	const TSharedRef<FBindingContext> Context = this->AsShared();

	FNWorldAssemblyEditorWorldCommands::Register(Context);
	FNWorldAssemblyEditorCellCommands::Register(Context);
	FNWorldAssemblyEditorJunctionCommands::Register(Context);
	FNWorldAssemblyEditorOrganCommands::Register(Context);
	FNWorldAssemblyEditorQuickAssemblyCommands::Register(Context);
}

void FNWorldAssemblyEditorCommands::WorldAssemblyEdMode()
{
	GLevelEditorModeTools().ActivateMode(UNWorldAssemblyEdMode::Identifier);
}

bool FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow()
{
	if (UNWorldAssemblyEdMode::IsActive()) return false;
	return FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld();
}
