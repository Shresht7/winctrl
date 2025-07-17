#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shellapi.h> // For Shell_NotifyIcon
#include <iostream>   // For std::cerr, though for a GUI app, error logging might go elsewhere

#include "hooks.h"
#include "winctrl.h"
#include "resources.h"

// Custom message for tray icon notifications
#define WM_TRAYICON (WM_USER + 1)

// Global variable for the hidden window handle
HWND g_hWnd;

// Forward declaration of the window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Function to add the tray icon
void AddTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1; // Icon ID
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)); // Load custom icon
    lstrcpy(nid.szTip, L"WinCtrl - Window Management");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

// Function to delete the tray icon
void DeleteTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = {0};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1; // Icon ID

    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// HELP MESSAGE
// ------------

const wchar_t *helpMessage =
    L"WinCtrl is running in the background.\n\n"
    L"Features:\n"
    L"- Win + Left Mouse Button Click: Maximize/Restore\n"
    L"- Win + Left Mouse Button Drag: Drag Window\n"
    L"- Win + Middle Mouse Button Drag: Resize Window\n"
    L"- Win + Ctrl + Scroll: Adjust Transparency\n"
    L"- Win + Scroll: Switch Virtual Desktop\n\n"
    L"Right-click the tray icon for more options and to toggle features.";

// WINDOW PROCEDURE
// ----------------

// Window Procedure for handling messages
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        AddTrayIcon(hWnd);
        break;

    case WM_TRAYICON:
        switch (LOWORD(lParam))
        {
        case WM_LBUTTONUP:
            MessageBox(hWnd, helpMessage, L"WinCtrl", MB_OK | MB_ICONINFORMATION);

            break;
        case WM_RBUTTONUP:
        {
            POINT curPoint;
            GetCursorPos(&curPoint);

            HMENU hMenu = CreatePopupMenu();

            // Add feature toggles
            AppendMenu(hMenu, MF_STRING | (Feature::isWinCtrlEnabled ? MF_CHECKED : MF_UNCHECKED), 1002, L"Enable WinCtrl");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); // Separator

            // Determine the flags for other menu items based on Feature::isWinCtrlEnabled
            UINT otherFeaturesFlags = MF_STRING;
            if (!Feature::isWinCtrlEnabled)
            {
                otherFeaturesFlags |= MF_GRAYED; // Grey out if WinCtrl is disabled
            }
            AppendMenu(hMenu, otherFeaturesFlags | (Feature::Move ? MF_CHECKED : MF_UNCHECKED), 1003, L"Enable Dragging");
            AppendMenu(hMenu, otherFeaturesFlags | (Feature::Resize ? MF_CHECKED : MF_UNCHECKED), 1004, L"Enable Resizing");
            AppendMenu(hMenu, otherFeaturesFlags | (Feature::Transparency ? MF_CHECKED : MF_UNCHECKED), 1005, L"Enable Transparency");
            AppendMenu(hMenu, otherFeaturesFlags | (Feature::VirtualDesktopScroll ? MF_CHECKED : MF_UNCHECKED), 1006, L"Enable Virtual Desktop Switching");

            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);    // Separator
            AppendMenu(hMenu, MF_STRING, 1001, L"Exit"); // Menu item with ID 1001

            // Set the foreground window to our window so the menu disappears when focus is lost
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu,
                           TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                           curPoint.x, curPoint.y, 0,
                           hWnd, NULL);
            DestroyMenu(hMenu);
            break;
        }
        default:
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1001: // "Exit" menu item clicked
            PostQuitMessage(0);
            break;
        case 1002: // "Pause WinCtrl" clicked
            Feature::toggleWinCtrlEnabled();
            break;
        case 1003: // "Enable Dragging" clicked
            Feature::toggleMove();
            break;
        case 1004: // "Enable Resizing" clicked
            Feature::toggleResize();
            break;
        case 1005: // "Enable Transparency" clicked
            Feature::toggleTransparency();
            break;
        case 1006: // "Enable Virtual Desktop Switching" clicked
            Feature::toggleVirtualDesktopScroll();
            break;
        }
        break;

    case WM_DESTROY:
        DeleteTrayIcon(hWnd);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// --------
// WIN MAIN
// --------

// WinMain entry point for GUI applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WinCtrlTrayClass";
    RegisterClassEx(&wc);

    // Create a hidden window to receive messages
    g_hWnd = CreateWindowEx(0, L"WinCtrlTrayClass", L"WinCtrl Hidden Window",
                            0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (!g_hWnd)
    {
        std::cerr << "Failed to create hidden window!" << std::endl;
        return 1;
    }

    // Setup hooks
    if (!setupHooks())
    {
        std::cerr << "Failed to setup hooks!" << std::endl;
        DeleteTrayIcon(g_hWnd); // Clean up tray icon if hooks fail
        DestroyWindow(g_hWnd);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Teardown hooks before exiting
    teardownHooks();

    return 0;
}
