# `winctrl`

GNOME like mouse-centric window management on Windows.

## Features

- **Move Windows**: Hold down the <kbd>Win</kbd> key and drag a window with the `Left Mouse Button` to move it. You don't have to target the titlebar!
  - **Maximize on Top**: Dragging a window to the very top edge of the screen will maximize it.
- **Resize Windows**: Hold down the <kbd>Win</kbd> key and drag with the `Middle Mouse Button`. Resizing is directional based on where you click:
  - **Edges**: Dragging from a window's side or top/bottom edge resizes along that axis.
  - **Corners**: Dragging from a corner resizes both height and width.
  - **Center**: Dragging from the center "zooms" the window in and out, preserving its aspect ratio.
- **Adjust Transparency**: Hold <kbd>Win</kbd> + <kbd>Ctrl</kbd> and use the `Mouse Scroll Wheel` to adjust the transparency of the window under the cursor.
- **Virtual Desktop Switch**: Hold down the <kbd>Win</kbd> key and use the `Mouse Scroll Wheel` to switch between virtual desktops.

## Usage

After building, you can run the application from the terminal

```
. .\winctrl.exe
```

> [!TIP]
>
> To have `winctrl.exe` run automatically every time you log in, you can add a shortcut to it in your Windows Startup folder:
> 1. Press <kbd>Win</kbd> + <kbd>R</kbd> to open the Run dialog.
> 2. Type `shell:startup` and press <kbd>Enter</kbd>. This will open the Startup folder.
> 3. Create a shortcut to your `winctrl.exe` file and place it in this folder.
> Now Windows will launch the program on startup!

---

## How it Works

This application uses a low-level global mouse hook to intercept all mouse events. It detects specific key combinations (Windows key + mouse button/scroll) and then performs the corresponding window action (move, resize, scroll).

- **Moving and Resizing**: When a drag or resize operation is initiated, the application identifies the window under the cursor and then continuously updates its position or size using the `SetWindowPos` Windows API function.
- **Virtual Desktop Switching**: For virtual desktop switching, the application simulates the `Win + Ctrl + Left/Right Arrow` key presses using `SendInput`.

## Development

### Build

```
g++ src/main.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp -o winctrl.exe -luser32
```

### Release

```
g++ src/main.cpp src/hooks.cpp src/winctrl.cpp src/helpers.cpp -o winctrl.exe -luser32 -mwindows
```

#### Flags

##### `-luser32`: Link User32 Library

The `-l` flag tells the linker to include (or "link") a library. `user32` is the name of a core Windows library (`user32.dll`). It contains the actual implementation for most of the Windows User Interface functions. This includes everything related to window management, messages, menus, and user input. Functions like `SetWindowPos`, `SetWindowsHookEx`, `GetMessage` and `GetAsyncKeyState` are a part of `user32`. The `include <windows.h>` tells the compiler that these functions exists and their signatures, but the linker needs to know where to find the actual code that will do the work. `-luser32` tells the linker to look inside `user32.dll` to find them; without it, you would get "undefined reference" errors during compilation

##### `-mwindows`: Windows Subsystem

This flag tells the compiler to build the program as a "GUI" (Graphical User Interface) application instead of a "Console" application. This allows the program to run silently in the background.

---

## License

The project is licensed under the [MIT License](./LICENSE)
