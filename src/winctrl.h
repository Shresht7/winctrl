#ifndef WINCTRL_H
#define WINCTRL_H

#include <windows.h>
#include <chrono>

// STATE

extern bool s_isMiddleMouseButtonDown;
extern std::chrono::steady_clock::time_point s_middleMouseButtonDownTime;
extern POINT s_middleMouseButtonDownPos;

bool isDragging();
bool isResizing();

// MOVE ACTIONS

void startDragging(MSLLHOOKSTRUCT *pMouse);
void stopDragging(MSLLHOOKSTRUCT *pMouse);
void performDrag(MSLLHOOKSTRUCT *pMouse);

// RESIZE ACTIONS

void startResizing(MSLLHOOKSTRUCT *pMouse);
void stopResizing();
void performResize(MSLLHOOKSTRUCT *pMouse);

// MAXIMIZE/RESTORE ACTIONS

void toggleMaximizeRestore(MSLLHOOKSTRUCT *pMouse);

// VIRTUAL DESKTOP

bool handleMouseWheel(MSLLHOOKSTRUCT *pMouse);

bool handleTransparency(MSLLHOOKSTRUCT *pMouse);

// HELPER FUNCTIONS

bool isExcludedWindow(HWND hWnd);
bool isFullscreen(HWND hWnd);

#endif // WINCTRL_H
