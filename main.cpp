#include <iostream>
#include <windows.h>
#include <cmath>
#include "winctrl.h"

// CONSTANTS
// ---------

const int KEY_PRESSED_FLAG = 0x8000;
const int MIN_WINDOW_SIZE = 100;

// GLOBAL VARIABLES
// ----------------

/// Global variable to store the mouse-hook handle
HHOOK g_mouseHook;

// MouseProc Callback
// ------------------

/// @brief Windows will call this callback function for every single mouse event (move, click etc).
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // The lParam contains a pointer to a structure with detailed information about the mouse event (like it's coordinates `pt`)
        MSLLHOOKSTRUCT *pMouse = (MSLLHOOKSTRUCT *)lParam;

        // Check if the left Windows key is pressed
        bool isWinKeyDown = GetAsyncKeyState(VK_LWIN) & KEY_PRESSED_FLAG;
        if (isWinKeyDown)
        {

            switch (wParam)
            {
            // Left button down
            case WM_LBUTTONDOWN:
                startDragging(pMouse);
                break;

            // Middle button down
            case WM_MBUTTONDOWN:
                startResizing(pMouse);
                break;

            // Mouse move
            case WM_MOUSEMOVE:
                if (isDragging())
                    performDrag(pMouse);
                else if (isResizing())
                    performResize(pMouse);
                break;

            // Left button up
            case WM_LBUTTONUP:
                stopDragging();
                break;

            // Middle button up
            case WM_MBUTTONUP:
                stopResizing();
                break;

            // Mouse Wheel Scroll
            case WM_MOUSEWHEEL:
                // The event was handled (and not throttled), so consume it
                if (handleMouseWheel(pMouse))
                    return 1;
                break;
            }
        }
    }

    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

// SETUP AND TEARDOWN
// ------------------

bool setupMouseHook()
{
    g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    return g_mouseHook != NULL;
}

void teardownMouseHook()
{
    if (g_mouseHook)
    {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = NULL;
    }
}

// MAIN
// ----

/// Main entrypoint of the application
int main()
{
    // Set a low-level mouse hook. This tells Windows to call our MouseProc function for every mouse event
    if (!setupMouseHook())
        return 1; // Mouse hook failed

    // A message loop to keep our program running in the background listening for events
    // This is essential for our hook to work
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook before exiting. This is crucial for cleanup
    teardownMouseHook();

    return 0;
}
