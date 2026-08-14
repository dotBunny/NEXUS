// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FViewport;

/**
 * One-keypress high-resolution capture, doing to a viewport what HighResShot does.
 *
 * Same engine path the console command takes — FViewport::TakeHighResScreenShot, which renders the scene
 * offscreen at the multiplied size — so what comes out is the rendered view with no editor chrome in it.
 * What this adds over typing the command is a keybinding, a multiplier that persists per user, and an
 * output path that lands in Saved/Screenshots rather than beside the executable.
 *
 * While a PIE session is up it captures the game viewport; otherwise the active level viewport.
 * @see <a href="https://nexus-framework.com/docs/plugins/tooling/editor-types/quick-highres-screenshot/">FNQuickHighResScreenshot</a>
 */
class NEXUSTOOLINGEDITOR_API FNQuickHighResScreenshot
{
public:
	/**
	 * Capture the target viewport at the per-user multiplier, sized off that viewport rather than any
	 * fixed resolution, and let the engine report the written file.
	 * @note Does nothing when a capture is already in flight, or when no viewport can be resolved.
	 */
	static void Capture();

private:
	/** Full path, extension included, of the file the in-flight capture will write; empty when idle. */
	static FString PendingFilePath;

	/** FilenameOverride as it stood before the in-flight capture claimed it, put back once the shot is processed. */
	static FString PreviousFilenameOverride;

	/** Handle for the one-shot request-processed binding that restores the override. */
	static FDelegateHandle ProcessedHandle;

	/** Request-processed handler; hands the shared filename override back to whoever set it. */
	static void OnScreenshotRequestProcessed();
};
