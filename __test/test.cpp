#include <windows.h>
#include <iostream>

int main() {
    // Set the starting and ending mouse positions
    int startX = 500;  // Starting X-coordinate
    int startY = 500;  // Starting Y-coordinate
    int endX = 600;    // Ending X-coordinate
    int endY = 600;    // Ending Y-coordinate

    // Convert starting and ending coordinates to screen coordinates
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int startScreenX = (startX * 65536) / screenWidth;
    int startScreenY = (startY * 65536) / screenHeight;
    int endScreenX = (endX * 65536) / screenWidth;
    int endScreenY = (endY * 65536) / screenHeight;

    // Set up the mouse input structure for button down
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = startScreenX;
    input.mi.dy = startScreenY;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;

    // Set up the mouse input structure for button up
    INPUT inputButtonUp;
    inputButtonUp.type = INPUT_MOUSE;
    inputButtonUp.mi.dx = endScreenX;
    inputButtonUp.mi.dy = endScreenY;
    inputButtonUp.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
    inputButtonUp.mi.mouseData = 0;
    inputButtonUp.mi.dwExtraInfo = 0;
    inputButtonUp.mi.time = 0;

    // Simulate moving the mouse from start to end position
    // for (int i = 0; i < 100; i++)
    {
        SendInput(1, &input, sizeof(INPUT));
        Sleep(500);  // Wait for half a second
        SendInput(1, &inputButtonUp, sizeof(INPUT));
    }

    std::cout << startScreenX << ' ' << startScreenY << '\n';
    std::cout << screenWidth << ' ' << screenHeight << '\n';
}
