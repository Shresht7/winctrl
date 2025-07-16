#include <chrono>
#include <cmath>

#include "winctrl.h"
#include "helpers.h"

// A window cannot be resized below this many pixels
const int MIN_WINDOW_SIZE = 100;

// STATE
// -----

/// Whether we are currently dragging a window
static bool s_isDragging = false;
/// Whether we are currently resizing a window
static bool s_isResizing = false;

/// The window handle of the window being dragged or resized
static HWND s_draggedWindow = NULL;
/// The position of the mouse at the start of the delta
static POINT s_initialMousePos;
/// The window rect at the start of the delta
static RECT s_initialWindowRect;

enum ResizeRegion
{
    NONE,
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    RIGHT,
    BOTTOM_RIGHT,
    BOTTOM,
    BOTTOM_LEFT,
    LEFT,
    CENTER
};

/// Determines the corner or edge to resize from
static ResizeRegion s_activeResizeRegion = NONE;

// DRAG
// ----

bool isDragging() { return s_isDragging; }

void startDragging(MSLLHOOKSTRUCT *pMouse)
{
    HWND hWnd = WindowFromPoint(pMouse->pt);      // Get the window handle under the cursor
    s_draggedWindow = GetAncestor(hWnd, GA_ROOT); // Get its top-level window

    // If the window is excluded, abort the operation
    if (isExcludedWindow(s_draggedWindow))
    {
        s_draggedWindow = NULL; // Reset the dragged window handle
        return;
    }

    if (isFullscreen(s_draggedWindow))
    {
        ShowWindow(s_draggedWindow, SW_RESTORE);
    }

    s_isDragging = true;            // Start dragging
    s_isResizing = false;           // Ensure only one mode is active
    s_initialMousePos = pMouse->pt; // Store the initial mouse position
}

void stopDragging(MSLLHOOKSTRUCT *pMouse)
{
    // If the window was dragged to the top edge, maximize it
    if (s_isDragging && pMouse->pt.y == 0)
    {
        ShowWindow(s_draggedWindow, SW_MAXIMIZE);
    }

    s_isDragging = false;   // Stop dragging. This will prevent the WM_MOUSEMOVE logic from running until the next drag starts
    s_draggedWindow = NULL; // Reset the dragged window handle
}

void performDrag(MSLLHOOKSTRUCT *pMouse)
{
    if (!s_draggedWindow)
    {
        return;
    }

    // Get the current position of the window
    RECT windowRect;
    GetWindowRect(s_draggedWindow, &windowRect);

    // Calculate the window's new top-left coordinates
    int newX = windowRect.left + (pMouse->pt.x - s_initialMousePos.x);
    int newY = windowRect.top + (pMouse->pt.y - s_initialMousePos.y);

    // Move the window to the new coordinates
    SetWindowPos(s_draggedWindow, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    // Update the mouse position for the next movement
    s_initialMousePos = pMouse->pt;
}

// RESIZE
// ------

bool isResizing() { return s_isResizing; }

void startResizing(MSLLHOOKSTRUCT *pMouse)
{
    HWND hWnd = WindowFromPoint(pMouse->pt);      // Get the window handle under the cursor
    s_draggedWindow = GetAncestor(hWnd, GA_ROOT); // Get its top-level window

    // If the window is excluded, abort the operation
    if (isExcludedWindow(s_draggedWindow) || isFullscreen(s_draggedWindow))
    {
        s_draggedWindow = NULL; // Reset the dragged window handle
        return;
    }

    s_isResizing = true;                                  // Start resizing
    s_isDragging = false;                                 // Ensure only one mode is active
    s_initialMousePos = pMouse->pt;                       // Store the initial mouse position
    GetWindowRect(s_draggedWindow, &s_initialWindowRect); // Store the initial window rect

    // Determine the resize region based on a 3x3 grid
    RECT rect = s_initialWindowRect;
    POINT pt = pMouse->pt;

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int col = (pt.x - rect.left) * 3 / width;
    int row = (pt.y - rect.top) * 3 / height;

    if (row == 0)
    {
        if (col == 0)
            s_activeResizeRegion = TOP_LEFT;
        else if (col == 1)
            s_activeResizeRegion = TOP;
        else
            s_activeResizeRegion = TOP_RIGHT;
    }
    else if (row == 1)
    {
        if (col == 0)
            s_activeResizeRegion = LEFT;
        else if (col == 1)
            s_activeResizeRegion = CENTER;
        else
            s_activeResizeRegion = RIGHT;
    }
    else
    {
        if (col == 0)
            s_activeResizeRegion = BOTTOM_LEFT;
        else if (col == 1)
            s_activeResizeRegion = BOTTOM;
        else
            s_activeResizeRegion = BOTTOM_RIGHT;
    }
}

void stopResizing()
{
    s_isResizing = false;        // Stop resizing
    s_draggedWindow = NULL;      // Reset the dragged window handle
    s_activeResizeRegion = NONE; // Reset the active resize region
}

void performResize(MSLLHOOKSTRUCT *pMouse)
{

    // Calculate the change in mouse position from the start
    int dx = pMouse->pt.x - s_initialMousePos.x;
    int dy = pMouse->pt.y - s_initialMousePos.y;

    // Determine the dimensions of the new window
    int newX = s_initialWindowRect.left;
    int newY = s_initialWindowRect.top;
    int newWidth = s_initialWindowRect.right - s_initialWindowRect.left;
    int newHeight = s_initialWindowRect.bottom - s_initialWindowRect.top;

    // Adjust the dimensions based on which region is active
    switch (s_activeResizeRegion)
    {
    case TOP_LEFT:
        newX = s_initialWindowRect.left + dx;
        newY = s_initialWindowRect.top + dy;
        newWidth = s_initialWindowRect.right - newX;
        newHeight = s_initialWindowRect.bottom - newY;
        break;
    case TOP:
        newY = s_initialWindowRect.top + dy;
        newHeight = s_initialWindowRect.bottom - newY;
        break;
    case TOP_RIGHT:
        newY = s_initialWindowRect.top + dy;
        newWidth = (s_initialWindowRect.right + dx) - s_initialWindowRect.left;
        newHeight = s_initialWindowRect.bottom - newY;
        break;
    case RIGHT:
        newWidth = (s_initialWindowRect.right + dx) - s_initialWindowRect.left;
        break;
    case BOTTOM_RIGHT:
        newWidth = (s_initialWindowRect.right + dx) - s_initialWindowRect.left;
        newHeight = (s_initialWindowRect.bottom + dy) - s_initialWindowRect.top;
        break;
    case BOTTOM:
        newHeight = (s_initialWindowRect.bottom + dy) - s_initialWindowRect.top;
        break;
    case BOTTOM_LEFT:
        newX = s_initialWindowRect.left + dx;
        newWidth = s_initialWindowRect.right - newX;
        newHeight = (s_initialWindowRect.bottom + dy) - s_initialWindowRect.top;
        break;
    case LEFT:
        newX = s_initialWindowRect.left + dx;
        newWidth = s_initialWindowRect.right - newX;
        break;
    case CENTER:
    {
        // Resize from the center, maintaining aspect ratio based on vertical mouse movement
        // We use 'dy' (vertical mouse movement) as the primary driver for size change.
        float aspectRatio = (float)newWidth / (float)newHeight;
        int sizeChange = dy * 2; // Multiply by 2 to make the resizing more sensitive

        // Calculate new width and height based on the aspect ratio and size change.
        // The width change is proportional to the height change, scaled by the aspect ratio.
        newWidth += (int)(sizeChange * aspectRatio);
        newHeight += sizeChange;

        // Reposition the window to keep its center stationary
        // The adjustment is half of the total size change in each dimension.
        newX -= (int)((sizeChange * aspectRatio) / 2);
        newY -= sizeChange / 2;
        break;
    }
    case NONE:
    default:
        break;
    }

    // Enforce a minimum window size
    if (newWidth < MIN_WINDOW_SIZE)
    {
        newWidth = MIN_WINDOW_SIZE;
    }
    if (newHeight < MIN_WINDOW_SIZE)
    {
        newHeight = MIN_WINDOW_SIZE;
    }

    // Command the window to resize to the new dimensions
    SetWindowPos(s_draggedWindow, NULL, newX, newY, newWidth, newHeight, SWP_NOZORDER);
}

// VIRTUAL DESKTOP SCROLL
// ----------------------

static const std::chrono::milliseconds THROTTLE_TIME(500);
static std::chrono::steady_clock::time_point s_lastSwitchTime = std::chrono::steady_clock::now();

static void simulateVirtualDesktopSwitch(bool scrollUp)
{
    INPUT inputs[6] = {};
    ZeroMemory(inputs, sizeof(inputs));

    // We simulate the virtual desktop switch by sending a sequence of key events (Win + Ctrl + Left/Right Arrow)

    // Press Win
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;

    // Press Ctrl
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_CONTROL;

    // Press Left or Right Arrow
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = scrollUp ? VK_LEFT : VK_RIGHT;

    // Release Left or Right Arrow
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = scrollUp ? VK_LEFT : VK_RIGHT;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release Ctrl
    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_CONTROL;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release Win
    inputs[5].type = INPUT_KEYBOARD;
    inputs[5].ki.wVk = VK_LWIN;
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(6, inputs, sizeof(INPUT));
}

bool handleMouseWheel(MSLLHOOKSTRUCT *pMouse)
{
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastSwitchTime) > THROTTLE_TIME)
    {
        // Extract the scroll direction from the mouseData
        short wheelDelta = HIWORD(pMouse->mouseData);
        if (wheelDelta > 0)
        {
            // Scroll Up
            simulateVirtualDesktopSwitch(true);
        }
        else
        {
            // Scroll Down
            simulateVirtualDesktopSwitch(false);
        }

        // Update the lastSwitchTime to inform the throttle check
        s_lastSwitchTime = now;

        return true; // Event handled successfully
    }

    return false; // Throttled, so we skipped processing the event
}

// TRANSPARENCY
// ------------

bool handleTransparency(MSLLHOOKSTRUCT *pMouse)
{
    HWND hWnd = WindowFromPoint(pMouse->pt);
    HWND targetWnd = GetAncestor(hWnd, GA_ROOT);

    if (isExcludedWindow(targetWnd))
    {
        return false;
    }

    // Ensure the window has the layered extended style
    LONG_PTR exStyle = GetWindowLongPtr(targetWnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED))
    {
        SetWindowLongPtr(targetWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }

    // Get current alpha
    BYTE currentAlpha;
    DWORD flags;
    GetLayeredWindowAttributes(targetWnd, NULL, &currentAlpha, &flags);

    // If the window has no transparency set yet, default to fully opaque
    if (!(flags & LWA_ALPHA))
    {
        currentAlpha = 255;
    }

    // Determine scroll direction
    short wheelDelta = HIWORD(pMouse->mouseData);
    int alphaChange = 5; // Amount to change alpha by

    if (wheelDelta > 0)
    {
        // Scroll Up - Increase opacity
        currentAlpha = std::min(255, currentAlpha + alphaChange);
    }
    else
    {                                                            // Scroll Down - Decrease opacity
        currentAlpha = std::max(25, currentAlpha - alphaChange); // Min alpha of ~10%
    }

    // Apply the new alpha value
    SetLayeredWindowAttributes(targetWnd, 0, currentAlpha, LWA_ALPHA);

    return true; // Event handled
}
