#include <iostream>
#include <Windows.h>

HHOOK mouseHook;
bool isDragging = false;
POINT dragStartPoint;

LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN) {
            // Left mouse button down event
            std::cout << "Left mouse button down" << std::endl;
        } else if (wParam == WM_LBUTTONUP) {
            // Left mouse button up event
            std::cout << "Left mouse button up" << std::endl;
            if (isDragging) {
                // Handle the end of the drag-and-drop action here
                std::cout << "Drag-and-drop completed" << std::endl;
                isDragging = false;
            }
        } else if (wParam == WM_MOUSEMOVE) {
            // Mouse move event
            MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            int x = mouseInfo->pt.x;
            int y = mouseInfo->pt.y;

            if (isDragging) {
                // Handle mouse drag here
                std::cout << "Dragging: X = " << x << ", Y = " << y << std::endl;
            }
        } else if (wParam == WM_LBUTTONDBLCLK) {
            // Left mouse button double-click event
            std::cout << "Left mouse button double-click" << std::endl;
        }

        if (wParam == WM_LBUTTONDOWN) {
            // Left mouse button down event
            std::cout << "Left mouse button down" << std::endl;
        }
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

int main() {
    // HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
    //     if (nCode >= 0) {
    //         KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    //         if (wParam == WM_KEYDOWN) {
    //             // Handle key press events here
    //             int key = kbdStruct->vkCode;
    //             std::cout << "Key Pressed: " << key << std::endl;

    //             if (key == VK_ESCAPE) {
    //                 // Handle the Escape key press
    //             }
    //             // Add more key handling logic here
    //         }
    //     }
    //     return CallNextHookEx(NULL, nCode, wParam, lParam);
    // }, GetModuleHandle(NULL), 0);
    
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, GetModuleHandle(NULL), 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    // UnhookWindowsHookEx(mouseHook);
}

// #include <windows.h>
// #include <iostream>

// int main() {
//     // Set the starting and ending mouse positions
//     int startX = 500;  // Starting X-coordinate
//     int startY = 500;  // Starting Y-coordinate
//     int endX = 600;    // Ending X-coordinate
//     int endY = 600;    // Ending Y-coordinate

//     // Convert starting and ending coordinates to screen coordinates
//     int screenWidth = GetSystemMetrics(SM_CXSCREEN);
//     int screenHeight = GetSystemMetrics(SM_CYSCREEN);
//     int startScreenX = (startX * 65536) / screenWidth;
//     int startScreenY = (startY * 65536) / screenHeight;
//     int endScreenX = (endX * 65536) / screenWidth;
//     int endScreenY = (endY * 65536) / screenHeight;

//     // Set up the mouse input structure for button down
//     INPUT inputButtonDown;
//     inputButtonDown.type = INPUT_MOUSE;
//     inputButtonDown.mi.dx = startScreenX;
//     inputButtonDown.mi.dy = startScreenY;
//     inputButtonDown.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
//     inputButtonDown.mi.mouseData = 0;
//     inputButtonDown.mi.dwExtraInfo = 0;
//     inputButtonDown.mi.time = 0;

//     // Set up the mouse input structure for movement
//     INPUT inputMove;
//     inputMove.type = INPUT_MOUSE;
//     inputMove.mi.dx = startScreenX;
//     inputMove.mi.dy = startScreenY;
//     inputMove.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
//     inputMove.mi.mouseData = 0;
//     inputMove.mi.dwExtraInfo = 0;
//     inputMove.mi.time = 0;

//     // Set up the mouse input structure for button up
//     INPUT inputButtonUp;
//     inputButtonUp.type = INPUT_MOUSE;
//     inputButtonUp.mi.dx = endScreenX;
//     inputButtonUp.mi.dy = endScreenY;
//     inputButtonUp.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
//     inputButtonUp.mi.mouseData = 0;
//     inputButtonUp.mi.dwExtraInfo = 0;
//     inputButtonUp.mi.time = 0;

//     // Send the mouse button down event
//     SendInput(1, &inputButtonDown, sizeof(INPUT));
//     Sleep(500);  // Wait for half a second

//     // Simulate moving the mouse from start to end position
//     for (int i = 0; i < 100; i++) {
//         inputMove.mi.dx = startScreenX + (endScreenX - startScreenX) * i / 100;
//         inputMove.mi.dy = startScreenY + (endScreenY - startScreenY) * i / 100;
//         SendInput(1, &inputMove, sizeof(INPUT));
//         Sleep(10);  // Wait for 10 milliseconds
//     }

//     // Send the mouse button up event
//     SendInput(1, &inputButtonUp, sizeof(INPUT));
//     Sleep(500);  // Wait for half a second

//     return 0;
// }
