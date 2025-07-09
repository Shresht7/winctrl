#include <iostream>
#include <windows.h>

/// Global variable to store the mouse-hook handle
HHOOK g_mouseHook;

/// State to track if we are currently dragging a window
bool g_isDragging = false;
/// The position of the mouse at the start of the delta
POINT g_initialMousePos;
/// The window handle of the window being dragged
HWND g_draggedWindow;

/// @brief Windows will call this callback function for every single mouse event (move, click etc).
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // The lParam contains a pointer to a structure with detailed information about the mouse event (like it's coordinates `pt`)
        MSLLHOOKSTRUCT *pMouse = (MSLLHOOKSTRUCT *)lParam;

        // Check if the left Windows key is pressed
        bool isWinKeyDown = GetAsyncKeyState(VK_LWIN) & 0x8000;

        switch (wParam)
        {
        // Left button down
        case WM_LBUTTONDOWN:
            if (isWinKeyDown)
            {
                g_isDragging = true;                                  // Start dragging
                g_initialMousePos = pMouse->pt;                       // Store the initial mouse position
                g_draggedWindow = WindowFromPoint(g_initialMousePos); // Get the window handle under the cursor
            }
            break;

        // Mouse move
        case WM_MOUSEMOVE:
            //  If we are dragging a window when Win+LButton is pressed
            if (g_isDragging && g_draggedWindow)
            {
                // Get the current position of the window
                RECT windowRect;
                GetWindowRect(g_draggedWindow, &windowRect);

                // Calculate the window's new top-left coordinates
                int newX = windowRect.left + (pMouse->pt.x - g_initialMousePos.x);
                int newY = windowRect.top + (pMouse->pt.y - g_initialMousePos.y);

                // Move the window to the new coordinates
                SetWindowPos(g_draggedWindow, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                // Update the mouse position for the next movement
                g_initialMousePos = pMouse->pt;
            }
            break;

        // Left button up
        case WM_LBUTTONUP:
            g_isDragging = false;   // Stop dragging. This will prevent the WM_MOUSEMOVE logic from running until the next drag starts
            g_draggedWindow = NULL; // Reset the dragged window handle
            break;
        }
    }

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
