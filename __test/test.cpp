#include <iostream>
#include <Windows.h>

// HHOOK mouseHook;
// bool isDragging = false;
// POINT dragStartPoint;

// LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode >= 0) {
//         if (wParam == WM_LBUTTONDOWN) {
//             // Left mouse button down event
//             std::cout << "Left mouse button down" << std::endl;
//         } else if (wParam == WM_LBUTTONUP) {
//             // Left mouse button up event
//             std::cout << "Left mouse button up" << std::endl;
//             if (isDragging) {
//                 // Handle the end of the drag-and-drop action here
//                 std::cout << "Drag-and-drop completed" << std::endl;
//                 isDragging = false;
//             }
//         } else if (wParam == WM_MOUSEMOVE) {
//             // Mouse move event
//             MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
//             int x = mouseInfo->pt.x;
//             int y = mouseInfo->pt.y;

//             if (isDragging) {
//                 // Handle mouse drag here
//                 std::cout << "Dragging: X = " << x << ", Y = " << y << std::endl;
//             }
//         } else if (wParam == WM_LBUTTONDBLCLK) {
//             // Left mouse button double-click event
//             std::cout << "Left mouse button double-click" << std::endl;
//         }

//         if (wParam == WM_LBUTTONDOWN) {
//             // Left mouse button down event
//             std::cout << "Left mouse button down" << std::endl;
//         }
//     }

//     return CallNextHookEx(mouseHook, nCode, wParam, lParam);
// }

int main() {
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode >= 0) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            if (wParam == WM_KEYDOWN) {
                // Handle key press events here
                int key = kbdStruct->vkCode;
                std::cout << "Key Pressed: " << key << std::endl;

                if (key == VK_ESCAPE) {
                    // Handle the Escape key press
                }
                // Add more key handling logic here
            }
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }, GetModuleHandle(NULL), 0);
    
    // mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, GetModuleHandle(NULL), 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    // UnhookWindowsHookEx(mouseHook);
}
