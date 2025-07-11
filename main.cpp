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

HHOOK g_keyboardHook;

bool shouldConsumeWin = false;

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
                shouldConsumeWin = true;
                break;

            // Middle button down
            case WM_MBUTTONDOWN:
                startResizing(pMouse);
                shouldConsumeWin = true;
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
                    shouldConsumeWin = true;
                return 1;
                break;
            }
        }
    }

    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

// KeyboardProc Callback
// ---------------------

// This callback procedure, when registered, is called whenever windows sends a keyboard event
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;

        // Whenever we release the Windows key...
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        {
            // Check to see if we triggered a winctrl shortcut, indicating that we need to consume the Win key release
            if (pKeyboard->vkCode == VK_LWIN && shouldConsumeWin)
            {
                // Send an Esc key to consume the held-down Win key
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_ESCAPE;
                SendInput(1, &input, sizeof(INPUT));
                shouldConsumeWin = false; // Reset the flag for future operations
            }
        }
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

// SETUP AND TEARDOWN
// ------------------

// Setup low-level mouse and keyboard hooks. This tells Windows to call our
// MouseProc/KeyProc callback functions for every mouse/keyboard event
bool setupMouseHook()
{
    g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    return g_mouseHook != NULL && g_keyboardHook != NULL;
}

// Cleanup all registered hooks before exiting the application
void teardownHooks()
{
    if (g_mouseHook)
    {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = NULL;
    }
    if (g_keyboardHook)
    {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = NULL;
    }
}

// MAIN
// ----

/// Main entrypoint of the application
int main()
{
    // Register keyboard and mouse hooks
    if (!setupMouseHook())
    {
        std::cerr << "Failed to setup hooks!" << std::endl;
        return EXIT_FAILURE;
    }

    // A message loop to keep our program running in the background listening for events
    // This is essential for our hook to work
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook before exiting. This is crucial for cleanup
    teardownHooks();

    return EXIT_SUCCESS;
}
