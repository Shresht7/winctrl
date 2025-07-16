#include <iostream>
#include <windows.h>
#include <cmath>

#include "hooks.h"

// MAIN
// ----

/// Main entrypoint of the application
int main()
{
    // Register keyboard and mouse hooks
    if (!setupHooks())
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
