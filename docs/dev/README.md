# `winctrl`

## How it Works

This application uses a low-level global mouse hook to intercept all mouse events. It detects specific key combinations (Windows key + mouse button/scroll) and then performs the corresponding window action (move, resize, scroll).

- **Moving and Resizing**: When a drag or resize operation is initiated, the application identifies the window under the cursor and then continuously updates its position or size using the `SetWindowPos` Windows API function.
- **Virtual Desktop Switching**: For virtual desktop switching, the application simulates the `Win + Ctrl + Left/Right Arrow` key presses using `SendInput`.

---

## Development

### Adding an Icon

To embed an icon (e.g., `resources/icon.ico`) into your executable, first create a resource script file (e.g., `src/resources.rc`) with the line `IDI_ICON1 ICON "resources/icon.ico"`. Then, compile it using `windres`:

```bash
windres src/resources.rc -O coff -o resources/winctrl.res
```

Finally, include `resources/winctrl.res` in your `g++` compilation command (e.g., `g++ ... resources/winctrl.res -o winctrl.exe ...`).

### Build (Console Application)

```
g++ src/main.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp winctrl.res -o winctrl.exe -luser32 -mconsole
```

### Build (Tray Application)

```
g++ src/tray.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp winctrl.res -o winctrl_tray.exe -luser32 -mwindows
```

### Release (Console Application)

```
g++ src/main.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp winctrl.res -o winctrl.exe -luser32 -mwindows
```

### Release (Tray Application)

```
g++ src/tray.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp winctrl.res -o winctrl_tray.exe -luser32 -mwindows
```

#### Flags

##### `-luser32`: Link User32 Library

The `-l` flag tells the linker to include (or "link") a library. `user32` is the name of a core Windows library (`user32.dll`). It contains the actual implementation for most of the Windows User Interface functions. This includes everything related to window management, messages, menus, and user input. Functions like `SetWindowPos`, `SetWindowsHookEx`, `GetMessage` and `GetAsyncKeyState` are a part of `user32`. The `include <windows.h>` tells the compiler that these functions exists and their signatures, but the linker needs to know where to find the actual code that will do the work. `-luser32` tells the linker to look inside `user32.dll` to find them; without it, you would get "undefined reference" errors during compilation

##### `-mwindows`: Windows Subsystem

This flag tells the compiler to build the program as a "GUI" (Graphical User Interface) application instead of a "Console" application. This allows the program to run silently in the background.
