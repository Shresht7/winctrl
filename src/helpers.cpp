#include <windows.h>
#include <string>
#include <vector>
#include <cmath>

// HELPER FUNCTIONS
// ----------------

/// @brief Helper function to calculate the distance between two points
static double calculateDistance(POINT p1, POINT p2)
{
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

/// @brief Determines if a given window should be excluded from WinCtrl's operations.
/// @param hWnd The handle of the window to check.
/// @return True if the window should be excluded, false otherwise.
bool isExcludedWindow(HWND hWnd)
{
    if (hWnd == NULL)
    {
        return true;
    }

    wchar_t className[256];
    GetClassNameW(hWnd, className, sizeof(className) / sizeof(wchar_t));
    std::wstring clsName(className);

    // List of window class names to exclude
    static const std::vector<std::wstring> excludedClassNames = {
        L"Shell_TrayWnd",              // Taskbar
        L"Progman",                    // Desktop
        L"Windows.UI.Core.CoreWindow", // UWP apps like Start Menu, Widget
        L"ApplicationFrameWindow",     // Some UWP app frames,
        L"WorkerW",                    // Used by desktop wallpaper
        L"Button",                     // Common for system buttons
    };

    for (const auto &excludedName : excludedClassNames)
    {
        if (clsName == excludedName)
        {
            return true;
        }
    }

    // Final check for the desktop and taskbar windows
    if (hWnd == GetDesktopWindow() || hWnd == FindWindowW(L"Shell_TrayWnd", NULL))
    {
        return true;
    }

    return false;
}

/// @brief Determines if a given window is fullscreen.
/// @param hWnd The handle of the window to check.
/// @return True if the window is fullscreen, false otherwise.
bool isFullscreen(HWND hWnd)
{
    if (hWnd == NULL)
    {
        return false;
    }

    // Get the window style
    LONG style = GetWindowLong(hWnd, GWL_STYLE);

    // Check if it's a borderless window
    if ((style & WS_CAPTION) == 0 && (style & WS_THICKFRAME) == 0)
    {
        // For borderless windows, check if the window size matches the screen size
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        return windowRect.left == 0 && windowRect.top == 0 &&
               windowRect.right == screenWidth && windowRect.bottom == screenHeight;
    }

    // For bordered windows, check if the window is maximized
    return IsZoomed(hWnd);
}
