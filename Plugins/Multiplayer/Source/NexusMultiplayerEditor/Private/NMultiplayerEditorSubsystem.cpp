// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerEditorSubsystem.h"

#include "NMultiplayerEditorModule.h"
#include "NMultiplayerEditorSettings.h"
#include "NMultiplayerEditorUserSettings.h"

void UNMultiplayerEditorSubsystem::Tick(float DeltaTime)
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

void UNMultiplayerEditorSubsystem::StartMultiplayerTest()
{
	ProcessHandles.Empty();
	LocalProcessDelegateHandle = FEditorDelegates::BeginStandaloneLocalPlay.AddUObject(this, &UNMultiplayerEditorSubsystem::AddLocalProcess);
	
	if (bIsMultiplayerTestRunning)
	{
		StopMultiplayerTest();
	}
	
	const FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();
	
	FRequestPlaySessionParams PlaySessionRequest;
	PlaySessionRequest.SessionDestination = EPlaySessionDestinationType::NewProcess;

	// Get default editor settings
	PlaySessionRequest.EditorPlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
	FObjectDuplicationParameters DuplicationParams(PlaySessionRequest.EditorPlaySettings, GetTransientPackage());
	PlaySessionRequest.EditorPlaySettings = CastChecked<ULevelEditorPlaySettings>(StaticDuplicateObjectEx(DuplicationParams));

#if WITH_EDITORONLY_DATA
	const UNMultiplayerEditorSettings* Settings = UNMultiplayerEditorSettings::Get();	
	const UNMultiplayerEditorUserSettings* UserSettings = UNMultiplayerEditorUserSettings::Get();
	
	Settings->ApplySettings(PlaySessionRequest);
	
	if (UserSettings->bClearLogsFolder)
	{
		FNEditorUtils::CleanLogsFolder();
	}
	UserSettings->ApplySettings(PlaySessionRequest);
		
#endif // WITH_EDITORONLY_DATA
		
	// Are there any additional parameters that a binding has provided?
	if (Module.OnCollectParameters.IsBound())
	{
		const FString AdditionalArgs = Module.OnCollectParameters.Execute().TrimStartAndEnd();
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

void UNMultiplayerEditorSubsystem::StopMultiplayerTest()
{
	if (!bIsMultiplayerTestRunning)
	{
		return;
	}
	
	const FNMultiplayerEditorModule& Module = FNMultiplayerEditorModule::Get();
	
	GUnrealEd->EndPlayOnLocalPc();
	Module.OnMultiplayerTestEnded.Broadcast();
	
	FEditorDelegates::BeginStandaloneLocalPlay.Remove(LocalProcessDelegateHandle);
	ProcessHandles.Empty();
	
	bIsMultiplayerTestRunning = false;
}

void UNMultiplayerEditorSubsystem::ToggleMultiplayerTest()
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

void UNMultiplayerEditorSubsystem::AddLocalProcess(const uint32 ProcessIdentifier)
{
	// We cache our ProcHandle as checking if the process is still running via FPlatformProcess::IsApplicationRunning
	// will create a new ProcHandle per call.
	ProcessHandles.Add(FPlatformProcess::OpenProcess(ProcessIdentifier));
}
