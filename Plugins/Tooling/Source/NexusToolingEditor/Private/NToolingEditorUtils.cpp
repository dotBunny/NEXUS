#include "NToolingEditorUtils.h"

#include "NToolingEditorMinimal.h"


#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#ifdef UNICODE
#define SEND_MESSAGE  SendMessageW
#else // !UNICODE
#define SEND_MESSAGE  SendMessageA
#endif // UNICODE
#endif // PLATFORM_WINDOWS


void FNToolingEditorUtils::ReplaceAppIconSVG(FSlateVectorImageBrush* Icon)
{
	FSlateStyleSet* MutableStyleSet = const_cast<FSlateStyleSet*>(static_cast<const FSlateStyleSet*>(&FAppStyle::Get()));
	if (MutableStyleSet != nullptr)
	{
		MutableStyleSet->Set("AppIcon", Icon);
	}
	else
	{
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("Unable to replace AppIcon with FSlateVectorImageBrush override."));
	}
}

void FNToolingEditorUtils::ReplaceAppIcon(FSlateImageBrush* Icon)
{
	FSlateStyleSet* MutableStyleSet = const_cast<FSlateStyleSet*>(static_cast<const FSlateStyleSet*>(&FAppStyle::Get()));
	if (MutableStyleSet != nullptr)
	{
		MutableStyleSet->Set("AppIcon", Icon);
	}
	else
	{
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("Unable to replace AppIcon with FSlateImageBrush override."));
	}
}

bool FNToolingEditorUtils::ReplaceWindowIcon(const FString& IconPath)
{
#if PLATFORM_WINDOWS
	const FString FinalPath = FString::Printf(TEXT("%s.ico"), *IconPath);
	// ReSharper disable CppCStyleCast, CppUE4CodingStandardNamingViolationWarning, CppZeroConstantCanBeReplacedWithNullptr
	if (FPaths::FileExists(FinalPath))
	{
		const Windows::HWND WindowHandle = FWindowsPlatformMisc::GetTopLevelWindowHandle(FWindowsPlatformProcess::GetCurrentProcessId());
		Windows::HICON hIcon = (Windows::HICON)LoadImageA(NULL, TCHAR_TO_ANSI(*FinalPath),IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE);
		
		if (hIcon)
		{
			// Set the large icon (Alt+Tab, taskbar)
			SEND_MESSAGE(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        
			// Set the small icon (window title bar)
			SEND_MESSAGE(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        
			// Also set it for the window class
			SetClassLongPtr(WindowHandle, GCLP_HICON, (LONG_PTR)hIcon);
			SetClassLongPtr(WindowHandle, GCLP_HICONSM, (LONG_PTR)hIcon);
			return true;
		}
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("Unable to replace the Unreal Editor application icon with the provided icon(%s) as it failed to load."), *FinalPath);
		return false;
	}
	// ReSharper restore CppCStyleCast, CppUE4CodingStandardNamingViolationWarning, CppZeroConstantCanBeReplacedWithNullptr
	UE_LOG(LogNexusToolingEditor, Warning, TEXT("Unable to replace the Unreal Editor application icon with the provided icon(%s) as it could not be found."), *FinalPath);
#else // !PLATFORM_WINDOWS
	UE_LOG(LogNexusToolingEditor, Warning, TEXT("Replacing the operating system icon for the Unreal Editor application is not supported on this platform."));
#endif // PLATFORM_WINDOWS
	return false;
}