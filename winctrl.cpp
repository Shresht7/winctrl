#include <cmath>
#include "winctrl.h"

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

enum ResizeCorner
{
    NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

/// Determines the corner to resize from
static ResizeCorner s_activeResizeCorner = NONE;

// HELPER FUNCTIONS
// ----------------

/// Helper function to calculate the distance between two points
static double calculateDistance(POINT p1, POINT p2)
{
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// DRAG
// ----

bool isDragging() { return s_isDragging; }

void startDragging(MSLLHOOKSTRUCT *pMouse)
{
    s_isDragging = true;                                  // Start dragging
    s_isResizing = false;                                 // Ensure only one mode is active
    s_initialMousePos = pMouse->pt;                       // Store the initial mouse position
    s_draggedWindow = WindowFromPoint(s_initialMousePos); // Get the window handle under the cursor
}

void stopDragging()
{
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
    s_isResizing = true;                                  // Start resizing
    s_isDragging = false;                                 // Ensure only one mode is active
    s_initialMousePos = pMouse->pt;                       // Store the initial mouse position
    s_draggedWindow = WindowFromPoint(pMouse->pt);        // Get the window handle under the cursor
    GetWindowRect(s_draggedWindow, &s_initialWindowRect); // Store the initial window rect

    // Determine the closest corner for resizing
    POINT topLeft = {s_initialWindowRect.left, s_initialWindowRect.top};
    POINT topRight = {s_initialWindowRect.right, s_initialWindowRect.top};
    POINT bottomLeft = {s_initialWindowRect.left, s_initialWindowRect.bottom};
    POINT bottomRight = {s_initialWindowRect.right, s_initialWindowRect.bottom};

    double distTL = calculateDistance(pMouse->pt, topLeft);
    double distTR = calculateDistance(pMouse->pt, topRight);
    double distBL = calculateDistance(pMouse->pt, bottomLeft);
    double distBR = calculateDistance(pMouse->pt, bottomRight);

    if (distTL < distTR && distTL < distBL && distTL < distBR)
    {
        s_activeResizeCorner = TOP_LEFT;
    }
    else if (distTR < distTL && distTR < distBL && distTR < distBR)
    {
        s_activeResizeCorner = TOP_RIGHT;
    }
    else if (distBL < distTL && distBL < distTR && distBL < distBR)
    {
        s_activeResizeCorner = BOTTOM_LEFT;
    }
    else if (distBR < distTL && distBR < distTR && distBR < distBL)
    {
        s_activeResizeCorner = BOTTOM_RIGHT;
    }
}

void stopResizing()
{
    s_isResizing = false;        // Stop resizing
    s_draggedWindow = NULL;      // Reset the dragged window handle
    s_activeResizeCorner = NONE; // Reset the active resize corner
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

    // Adjust the dimensions based on which corner is active
    switch (s_activeResizeCorner)
    {
    case TOP_LEFT:
        newX = s_initialWindowRect.left + dx;
        newY = s_initialWindowRect.top + dy;
        newWidth = s_initialWindowRect.right - newX;
        newHeight = s_initialWindowRect.bottom - newY;
        break;
    case TOP_RIGHT:
        newY = s_initialWindowRect.top + dy;
        newWidth = (s_initialWindowRect.right + dx) - s_initialWindowRect.left;
        newHeight = s_initialWindowRect.bottom - newY;
        break;
    case BOTTOM_LEFT:
        newX = s_initialWindowRect.left + dx;
        newWidth = s_initialWindowRect.right - newX;
        newHeight = (s_initialWindowRect.bottom + dy) - s_initialWindowRect.top;
        break;
    case BOTTOM_RIGHT:
        newWidth = (s_initialWindowRect.right + dx) - s_initialWindowRect.left;
        newHeight = (s_initialWindowRect.bottom + dy) - s_initialWindowRect.top;
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
    SetWindowPos(s_draggedWindow, NULL, newX, newY, newWidth, newHeight, SWP_NOZORDER);
}
