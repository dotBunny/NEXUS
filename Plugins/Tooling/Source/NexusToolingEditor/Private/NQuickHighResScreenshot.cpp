// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NQuickHighResScreenshot.h"

#include "Editor.h"
#include "HighResScreenshot.h"
#include "LevelEditor.h"
#include "NToolingEditorMinimal.h"
#include "NToolingEditorUserSettings.h"
#include "SLevelViewport.h"
#include "Settings/LevelEditorMiscSettings.h"
#include "Slate/SceneViewport.h"

FString FNQuickHighResScreenshot::PendingFilePath;
FString FNQuickHighResScreenshot::PreviousFilenameOverride;
FDelegateHandle FNQuickHighResScreenshot::ProcessedHandle;

/** @return The editor's configured screenshot directory as an absolute path. */
static FString GetScreenshotDirectory()
{
	// Deliberately not UEngine::GameScreenshotSaveDirectory, which is what the high-res path itself reads:
	// that value is only ever assigned on the UGameEngine instance, so the CDO the path consults is empty
	// in the editor and the shot ends up relative to the process working directory. This is the directory
	// the editor's own F9 capture writes to, and it defaults to FPaths::ScreenShotDir().
	const FString Directory = GetDefault<ULevelEditorMiscSettings>()->EditorScreenshotSaveDirectory.Path;
	return FPaths::ConvertRelativePathToFull(Directory.IsEmpty() ? FPaths::ScreenShotDir() : Directory);
}

/**
 * @return The viewport to capture: the game viewport whenever a PIE session is up, otherwise the active
 *         level viewport. Null when neither can be resolved.
 */
static FViewport* ResolveTargetViewport()
{
	// A running PIE session is what the developer is looking at, and it may be hosted in the level
	// viewport, a floating window, or a standalone one. GetPIEViewport covers all three; reading the level
	// viewport's own active viewport only finds the first of them.
	if (GEditor->PlayWorld != nullptr)
	{
		if (FViewport* PIEViewport = GEditor->GetPIEViewport())
		{
			return PIEViewport;
		}
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	const TSharedPtr<SLevelViewport> LevelViewport = LevelEditorModule.GetFirstActiveLevelViewport();
	return LevelViewport.IsValid() ? LevelViewport->GetSharedActiveViewport().Get() : nullptr;
}

void FNQuickHighResScreenshot::Capture()
{
	// One capture already in flight; the engine's own dialog guards the same way.
	if (GIsHighResScreenshot)
	{
		return;
	}

	FViewport* TargetViewport = ResolveTargetViewport();
	if (TargetViewport == nullptr)
	{
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("No viewport available to capture."));
		return;
	}

	// Only the property's own ClampMin/ClampMax bound this. FHighResScreenshotConfig::MinResolutionMultipler
	// is 1.0, but the console command's parser accepts anything above zero and downscaling works, so the
	// dialog's floor is a UI convention rather than a capability limit.
	const float Multiplier = UNToolingEditorUserSettings::Get()->QuickHighResScreenshotMultiplier;
	if (Multiplier <= 0.0f)
	{
		return;
	}

	FHighResScreenshotConfig& Config = GetHighResScreenshotConfig();

	// Mirrors FHighResScreenshotConfig::ParseConsoleCommand, which is the setup HighResShot runs through.
	// Leaving the resolution globals at zero is the part that matters: FViewport::TakeHighResScreenShot
	// then derives the size from the viewport it is called on, times the multiplier. Setting them by hand
	// pins the shot to whatever viewport happened to be measured, which is wrong as soon as the target is
	// a PIE window rather than the level viewport, and stale values are what a rejected shot leaves behind.
	GScreenshotResolutionX = 0;
	GScreenshotResolutionY = 0;
	Config.ResolutionMultiplier = Multiplier;
	Config.ResolutionMultiplierScale = 0.0f;
	Config.UnscaledCaptureRegion = FIntRect(0, 0, 0, 0);
	Config.CaptureRegion = Config.UnscaledCaptureRegion;
	Config.bMaskEnabled = false;
	Config.bDateTimeBasedNaming = false;
	Config.bDumpBufferVisualizationTargets = false;

	// The one thing not left to the engine. Its own naming resolves against UEngine::GameScreenshotSaveDirectory,
	// which is unset in the editor, so an un-overridden HighResShot writes relative to the process working
	// directory rather than into Saved/Screenshots. An override also skips the index-suffix pass, hence the
	// timestamp. The extension matches what FViewport::Draw passes for bHDRScreenshot.
	const TCHAR* Extension = TargetViewport->GetSceneHDREnabled() ? TEXT(".exr") : TEXT(".png");
	PendingFilePath = GetScreenshotDirectory() /
		FString::Printf(TEXT("NEXUS_HighResScreenshot_%s%s"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")), Extension);

	PreviousFilenameOverride = Config.FilenameOverride;
	Config.SetFilename(PendingFilePath);

	// Reporting the written file is the engine's job: FViewport::HighResScreenshot raises its own toast
	// naming the path with an Open Folder hyperlink. This binding exists only to put FilenameOverride back.
	ProcessedHandle = FScreenshotRequest::OnScreenshotRequestProcessed().AddStatic(&FNQuickHighResScreenshot::OnScreenshotRequestProcessed);

	// The engine logs the written path itself once the shot lands; this only records which viewport it
	// picked, which is the part that is otherwise invisible when the answer looks wrong.
	UE_LOG(LogNexusToolingEditor, Verbose, TEXT("Capturing the %s viewport (%dx%d) at %.2fx to %s."),
		TargetViewport == GEditor->GetPIEViewport() ? TEXT("game") : TEXT("level"),
		TargetViewport->GetSizeXY().X, TargetViewport->GetSizeXY().Y, Multiplier, *PendingFilePath);

	if (!TargetViewport->TakeHighResScreenShot())
	{
		// Rejected outright for exceeding the maximum texture dimension. The engine raises its own
		// notification for that and nothing will ever be processed, so unwind the bookkeeping here.
		PendingFilePath.Empty();
		OnScreenshotRequestProcessed();
	}
}

void FNQuickHighResScreenshot::OnScreenshotRequestProcessed()
{
	FScreenshotRequest::OnScreenshotRequestProcessed().Remove(ProcessedHandle);
	ProcessedHandle.Reset();

	// FilenameOverride is global state shared with the built-in High Resolution Screenshot dialog and with
	// HighResShot itself, so leaving ours in place would have the next capture from either reuse our name.
	GetHighResScreenshotConfig().SetFilename(PreviousFilenameOverride);
	PreviousFilenameOverride.Empty();
	PendingFilePath.Empty();
}
