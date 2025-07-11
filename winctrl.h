#ifndef WINCTRL_H
#define WINCTRL_H

#include <windows.h>

// STATE

bool isDragging();
bool isResizing();

// MOVE ACTIONS

void startDragging(MSLLHOOKSTRUCT *pMouse);
void stopDragging();
void performDrag(MSLLHOOKSTRUCT *pMouse);

// RESIZE ACTIONS

void startResizing(MSLLHOOKSTRUCT *pMouse);
void stopResizing();
void performResize(MSLLHOOKSTRUCT *pMouse);

// VIRTUAL DESKTOP

bool handleMouseWheel(MSLLHOOKSTRUCT *pMouse);

// HELPER FUNCTIONS

bool isExcludedWindow(HWND hWnd);

#endif // WINCTRL_H
