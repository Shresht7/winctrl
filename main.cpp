#include <iostream>
#include <windows.h>
#include <cmath>
#include "main.h"

// CONSTANTS
// ---------

const int KEY_PRESSED_FLAG = 0x8000;
const int MIN_WINDOW_SIZE = 100;

// GLOBAL VARIABLES
// ----------------

/// Global variable to store the mouse-hook handle
HHOOK g_mouseHook;

/// State to track if we are currently dragging a window
bool g_isDragging = false;
/// The position of the mouse at the start of the delta
POINT g_initialMousePos;
/// The window handle of the window being dragged
HWND g_draggedWindow;

/// State to track if we are currently resizing a window
bool g_isResizing = false;
/// The window rect at the start of the delta
RECT g_initialWindowRect;

// TODO: This currently does not support resizing from the edges

enum ResizeCorner
{
    NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

/// Determines the corner to resize from
ResizeCorner g_activeResizeCorner = NONE;

// HELPER FUNCTIONS
// ----------------

/// Helper function to calculate the distance between two points
double calculateDistance(POINT p1, POINT p2)
{
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

/// @brief Windows will call this callback function for every single mouse event (move, click etc).
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // The lParam contains a pointer to a structure with detailed information about the mouse event (like it's coordinates `pt`)
        MSLLHOOKSTRUCT *pMouse = (MSLLHOOKSTRUCT *)lParam;

        // Check if the left Windows key is pressed
        bool isWinKeyDown = GetAsyncKeyState(VK_LWIN) & KEY_PRESSED_FLAG;

        switch (wParam)
        {
        // Left button down
        case WM_LBUTTONDOWN:
            if (isWinKeyDown)
            {
                g_isDragging = true;                                  // Start dragging
                g_isResizing = false;                                 // Ensure only one mode is active
                g_initialMousePos = pMouse->pt;                       // Store the initial mouse position
                g_draggedWindow = WindowFromPoint(g_initialMousePos); // Get the window handle under the cursor
            }
            break;

        // Middle button down
        case WM_MBUTTONDOWN:
            if (isWinKeyDown)
            {
                g_isResizing = true;                                  // Start resizing
                g_isDragging = false;                                 // Ensure only one mode is active
                g_initialMousePos = pMouse->pt;                       // Store the initial mouse position
                g_draggedWindow = WindowFromPoint(pMouse->pt);        // Get the window handle under the cursor
                GetWindowRect(g_draggedWindow, &g_initialWindowRect); // Store the initial window rect

                // Determine the closest corner for resizing
                POINT topLeft = {g_initialWindowRect.left, g_initialWindowRect.top};
                POINT topRight = {g_initialWindowRect.right, g_initialWindowRect.top};
                POINT bottomLeft = {g_initialWindowRect.left, g_initialWindowRect.bottom};
                POINT bottomRight = {g_initialWindowRect.right, g_initialWindowRect.bottom};

                double distTL = calculateDistance(pMouse->pt, topLeft);
                double distTR = calculateDistance(pMouse->pt, topRight);
                double distBL = calculateDistance(pMouse->pt, bottomLeft);
                double distBR = calculateDistance(pMouse->pt, bottomRight);

                if (distTL < distTR && distTL < distBL && distTL < distBR)
                {
                    g_activeResizeCorner = TOP_LEFT;
                }
                else if (distTR < distTL && distTR < distBL && distTR < distBR)
                {
                    g_activeResizeCorner = TOP_RIGHT;
                }
                else if (distBL < distTL && distBL < distTR && distBL < distBR)
                {
                    g_activeResizeCorner = BOTTOM_LEFT;
                }
                else if (distBR < distTL && distBR < distTR && distBR < distBL)
                {
                    g_activeResizeCorner = BOTTOM_RIGHT;
                }
            }
            break;

        // Mouse move
        case WM_MOUSEMOVE:
            //  If we are dragging a window when Win+LButton is pressed
            if (g_isDragging && g_draggedWindow)
            {
                handleWindowDrag(pMouse);
            }
            else if (g_isResizing && g_draggedWindow)
            {
                handleWindowResize(pMouse);
            }
            break;

        // Left button up
        case WM_LBUTTONUP:
            g_isDragging = false;   // Stop dragging. This will prevent the WM_MOUSEMOVE logic from running until the next drag starts
            g_draggedWindow = NULL; // Reset the dragged window handle
            break;

        // Middle button up
        case WM_MBUTTONUP:
            g_isResizing = false;        // Stop resizing
            g_draggedWindow = NULL;      // Reset the dragged window handle
            g_activeResizeCorner = NONE; // Reset the active resize corner
            break;
        }
    }

    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

void handleWindowDrag(MSLLHOOKSTRUCT *pMouse)
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

void handleWindowResize(MSLLHOOKSTRUCT *pMouse)
{

    // Calculate the change in mouse position from the start
    int dx = pMouse->pt.x - g_initialMousePos.x;
    int dy = pMouse->pt.y - g_initialMousePos.y;

    // Determine the dimensions of the new window
    int newX = g_initialWindowRect.left;
    int newY = g_initialWindowRect.top;
    int newWidth = g_initialWindowRect.right - g_initialWindowRect.left;
    int newHeight = g_initialWindowRect.bottom - g_initialWindowRect.top;

    // Adjust the dimensions based on which corner is active
    switch (g_activeResizeCorner)
    {
    case TOP_LEFT:
        newX = g_initialWindowRect.left + dx;
        newY = g_initialWindowRect.top + dy;
        newWidth = g_initialWindowRect.right - newX;
        newHeight = g_initialWindowRect.bottom - newY;
        break;
    case TOP_RIGHT:
        newY = g_initialWindowRect.top + dy;
        newWidth = (g_initialWindowRect.right + dx) - g_initialWindowRect.left;
        newHeight = g_initialWindowRect.bottom - newY;
        break;
    case BOTTOM_LEFT:
        newX = g_initialWindowRect.left + dx;
        newWidth = g_initialWindowRect.right - newX;
        newHeight = (g_initialWindowRect.bottom + dy) - g_initialWindowRect.top;
        break;
    case BOTTOM_RIGHT:
        newWidth = (g_initialWindowRect.right + dx) - g_initialWindowRect.left;
        newHeight = (g_initialWindowRect.bottom + dy) - g_initialWindowRect.top;
        break;
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
    SetWindowPos(g_draggedWindow, NULL, newX, newY, newWidth, newHeight, SWP_NOZORDER);
}

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

/// Main entrypoint of the application
int main()
{
    // Set a low-level mouse hook. This tells Windows to call our MouseProc function for every mouse event
    if (!setupMouseHook())
    {
        return 1; // Mouse hook failed
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
    teardownMouseHook();

    return 0;
}
