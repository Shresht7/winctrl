# `winctrl`

## Development

### Build

```
g++ main.cpp winctrl.cpp -o winctrl.exe -luser32
```

### Release

```
g++ main.cpp winctrl.cpp -o winctrl.exe -luser32 -mwindows
```

#### Flags

##### `-luser32`: Link User32 Library

The `-l` flag tells the linker to include (or "link") a library. `user32` is the name of a core Windows library (`user32.dll`). It contains the actual implementation for most of the Windows User Interface functions. This includes everything related to window management, messages, menus, and user input. Functions like `SetWindowPos`, `SetWindowsHookEx`, `GetMessage` and `GetAsyncKeyState` are a part of `user32`. The `include <windows.h>` tells the compiler that these functions exists and their signatures, but the linker needs to know where to find the actual code that will do the work. `-luser32` tells the linker to look inside `user32.dll` to find them; without it, you would get "undefined reference" errors during compilation

##### `-mwindows`: Windows Subsystem

This flag tells the compiler to build the program as a "GUI" (Graphical User Interface) application instead of a "Console" application. This allows the program to run silently in the background.
