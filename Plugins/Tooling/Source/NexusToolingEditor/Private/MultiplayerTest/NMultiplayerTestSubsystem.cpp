// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "MultiplayerTest/NMultiplayerTestSubsystem.h"

#include "NToolingEditorModule.h"
#include "NToolingEditorSettings.h"
#include "NToolingEditorUserSettings.h"

void UNMultiplayerTestSubsystem::Tick(float DeltaTime)
{
	// We don't want to be hammering the processes, so instead we'll check every 2 seconds.
	DeltaAccumulator += DeltaTime;
	if (DeltaAccumulator < 2.f)
	{
		return;
	}
	DeltaAccumulator = 0.f;
	
	bool bAnyProcessAlive = false;
	for (FProcHandle& Handle : ProcessHandles)
	{
		if (Handle.IsValid() && FPlatformProcess::IsProcRunning(Handle))
		{
			bAnyProcessAlive = true;
		}
	}
	if (!bAnyProcessAlive)
	{
		StopMultiplayerTest();
	}
}

void UNMultiplayerTestSubsystem::StartMultiplayerTest()
{
	// Stop first so any prior delegate binding is removed and prior process handles are closed before we
	// overwrite LocalProcessDelegateHandle with the new binding below.
	if (bIsMultiplayerTestRunning)
	{
		StopMultiplayerTest();
	}

	LocalProcessDelegateHandle = FEditorDelegates::BeginStandaloneLocalPlay.AddUObject(this, &UNMultiplayerTestSubsystem::AddLocalProcess);

	const FNToolingEditorModule& Module = FNToolingEditorModule::Get();
	
	FRequestPlaySessionParams PlaySessionRequest;
	PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

	// Get default editor settings
	PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
	FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
	PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

#if WITH_EDITORONLY_DATA
	const UNToolingEditorSettings* Settings = UNToolingEditorSettings::Get();	
	const UNToolingEditorUserSettings* UserSettings = UNToolingEditorUserSettings::Get();
	
	Settings->ApplySettings(PlaySessionRequest);
	
	if (UserSettings->bClearLogsFolder)
	{
		FNEditorUtils::CleanLogsFolder();
	}
	UserSettings->ApplySettings(PlaySessionRequest);
		
#endif // WITH_EDITORONLY_DATA
		
	// Are there any additional parameters that a binding has provided?
	if (Module.OnMultiplayerTestParameters.IsBound())
	{
		const FString AdditionalArgs = Module.OnMultiplayerTestParameters.Execute().TrimStartAndEnd();
		if (!AdditionalArgs.IsEmpty())
		{
			PlaySessionRequest.EditorPlaySettings->AdditionalLaunchParameters.Append(
				FString::Printf(TEXT(" %s"), *AdditionalArgs));
		}
	}

	PlaySessionRequest.EditorPlaySettings->SetRunUnderOneProcess(false);

	// Start!
	GUnrealEd->RequestPlaySession(PlaySessionRequest);
	Module.OnMultiplayerTestStarted.Broadcast();
	
	bIsMultiplayerTestRunning = true;
}

void UNMultiplayerTestSubsystem::StopMultiplayerTest()
{
	if (!bIsMultiplayerTestRunning)
	{
		return;
	}
	
	// Some analysis says that we could receive this call via Deinitialize which *could* have the editor already torn down, so were going to
	// be explicit about the in editor callbacks.
	if (GUnrealEd && !GIsRequestingExit)
	{
		const FNToolingEditorModule& Module = FNToolingEditorModule::Get();
	
		GUnrealEd->EndPlayOnLocalPc();
		Module.OnMultiplayerTestEnded.Broadcast();
	
		FEditorDelegates::BeginStandaloneLocalPlay.Remove(LocalProcessDelegateHandle);
		LocalProcessDelegateHandle.Reset();
	}

	// FProcHandle's destructor does not release the underlying OS handle; CloseProcess is required to avoid leaking
	// a kernel handle per spawned PIE process every Start/Stop cycle.
	for (FProcHandle& Handle : ProcessHandles)
	{
		if (Handle.IsValid())
		{
			FPlatformProcess::CloseProc(Handle);
		}
	}
	ProcessHandles.Empty();

	bIsMultiplayerTestRunning = false;
}

void UNMultiplayerTestSubsystem::Deinitialize()
{
	if (bIsMultiplayerTestRunning)
	{
		StopMultiplayerTest();
	}
	Super::Deinitialize();
}

void UNMultiplayerTestSubsystem::ToggleMultiplayerTest()
{
	if (bIsMultiplayerTestRunning)
	{
		StopMultiplayerTest();
	}
	else
	{
		StartMultiplayerTest();
	}
}

void UNMultiplayerTestSubsystem::AddLocalProcess(const uint32 ProcessIdentifier)
{
	// We cache our ProcHandle as checking if the process is still running via FPlatformProcess::IsApplicationRunning
	// will create a new ProcHandle per call.
	ProcessHandles.Add(FPlatformProcess::OpenProcess(ProcessIdentifier));
}
