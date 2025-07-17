#include <windows.h>

#include "winctrl.h"

// CONSTANTS
// ---------

// Flag for when a key is pressed
const int KEY_PRESSED_FLAG = 0x8000;

// GLOBAL VARIABLES
// ----------------

// The mouse-hook handle
static HHOOK s_mouseHook;

// The keyboard-hook handle
static HHOOK s_keyboardHook;

// Indicates if we should consume the Win key after a successful `winctrl` action
static bool s_shouldConsumeWin = false;

// MouseProc Callback
// ------------------

/// @brief Windows will call this callback function for every single mouse event (move, click etc).
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // If disabled, skip entirely
        if (!Feature::isWinCtrlEnabled)
        {
            return CallNextHookEx(s_mouseHook, nCode, wParam, lParam);
        }

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
                s_isLeftMouseButtonDown = true;
                s_isLeftMouseButtonDownTime = std::chrono::steady_clock::now();
                s_isLeftMouseButtonDownPos = pMouse->pt;
                s_shouldConsumeWin = true;
                break;

            // Middle button down
            case WM_MBUTTONDOWN:
                s_isMiddleMouseButtonDown = true;
                s_middleMouseButtonDownTime = std::chrono::steady_clock::now();
                s_middleMouseButtonDownPos = pMouse->pt;
                break;

            // Mouse move
            case WM_MOUSEMOVE:
                if (Feature::Move && isDragging())
                    performDrag(pMouse);
                else if (Feature::Resize && isResizing())
                    performResize(pMouse);
                else if (Feature::Move && s_isLeftMouseButtonDown)
                {
                    // If left button is down and we are not yet dragging, check for movement to start dragging
                    const int DRAG_THRESHOLD = 5; // Pixels
                    if (abs(pMouse->pt.x - s_isLeftMouseButtonDownPos.x) > DRAG_THRESHOLD ||
                        abs(pMouse->pt.y - s_isLeftMouseButtonDownPos.y) > DRAG_THRESHOLD)
                    {
                        startDragging(pMouse);
                        s_shouldConsumeWin = true;
                    }
                }
                else if (Feature::Resize && s_isMiddleMouseButtonDown)
                {
                    // If middle button is down and we are not yet resizing, check for movement to start resizing
                    const int DRAG_THRESHOLD = 5; // Pixels
                    if (abs(pMouse->pt.x - s_middleMouseButtonDownPos.x) > DRAG_THRESHOLD ||
                        abs(pMouse->pt.y - s_middleMouseButtonDownPos.y) > DRAG_THRESHOLD)
                    {
                        startResizing(pMouse);
                        s_shouldConsumeWin = true;
                    }
                }
                break;

            // Left button up
            case WM_LBUTTONUP:
                if (s_isLeftMouseButtonDown)
                {
                    s_isLeftMouseButtonDown = false;
                    auto now = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_isLeftMouseButtonDownTime).count();
                    // Define a click threshold (e.g., 200 milliseconds)
                    const int CLICK_THRESHOLD_MS = 200;

                    // Check if it was a click (short duration) and no dragging occurred
                    if (duration < CLICK_THRESHOLD_MS && !isDragging() &&
                        (abs(pMouse->pt.x - s_isLeftMouseButtonDownPos.x) < 5 &&
                         abs(pMouse->pt.y - s_isLeftMouseButtonDownPos.y) < 5))
                    {
                        toggleMaximizeRestore(pMouse);
                    }
                    else if (Feature::Move && isDragging())
                    {
                        stopDragging(pMouse);
                    }
                }
                break;

            // Middle button up
            case WM_MBUTTONUP:
                if (Feature::Resize)
                {
                    stopResizing();
                }
                s_isMiddleMouseButtonDown = false;
                break;

            // Mouse Wheel Scroll
            case WM_MOUSEWHEEL:
                // Check if Ctrl is also pressed for transparency adjustment
                if (GetAsyncKeyState(VK_CONTROL) & KEY_PRESSED_FLAG)
                {
                    if (Feature::Transparency && handleTransparency(pMouse))
                    {
                        s_shouldConsumeWin = true;
                        return 1; // Consume the mouse-scroll to prevent propagation
                    }
                }
                // Otherwise, scroll through the virtual desktops
                else
                {
                    // The event was handled (and not throttled), so consume it
                    if (Feature::VirtualDesktopScroll && handleMouseWheel(pMouse))
                        s_shouldConsumeWin = true;
                }
                break;
            }
        }
    }

    return CallNextHookEx(s_mouseHook, nCode, wParam, lParam);
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
            if (pKeyboard->vkCode == VK_LWIN && s_shouldConsumeWin)
            {
                // Note: Send an Esc key to consume the held-down Win key
                //  This is to prevent the Start Menu from appearing, which would otherwise happen
                //  because the system registers the Win key release.
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_ESCAPE;
                SendInput(1, &input, sizeof(INPUT));
                s_shouldConsumeWin = false; // Reset the flag for future operations
            }
        }
    }

    return CallNextHookEx(s_keyboardHook, nCode, wParam, lParam);
}

// SETUP AND TEARDOWN
// ------------------

// Setup low-level mouse and keyboard hooks. This tells Windows to call our
// MouseProc/KeyProc callback functions for every mouse/keyboard event
bool setupHooks()
{
    s_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    s_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    return s_mouseHook != NULL && s_keyboardHook != NULL;
}

// Cleanup all registered hooks before exiting the application
void teardownHooks()
{
    if (s_mouseHook)
    {
        UnhookWindowsHookEx(s_mouseHook);
        s_mouseHook = NULL;
    }
    if (s_keyboardHook)
    {
        UnhookWindowsHookEx(s_keyboardHook);
        s_keyboardHook = NULL;
    }
}
