#include <iostream>
#include <windows.h>

/// Global variable to store the mouse-hook handle
HHOOK g_mouseHook;

/// @brief Windows will call this callback function for every single mouse event (move, click etc).
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // TODO: Implement hook logic here
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

/// Main entrypoint of the application
int main()
{
    // Set a low-level mouse hook. This tells Windows to call our MouseProc function for every mouse event
    g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);

    // A message loop to keep our program running in the background listening for events
    // This is essential for our hook to work
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook before exiting. This is crucial for cleanup
    UnhookWindowsHookEx(g_mouseHook);

    return 0;
}
