#include "hypno_event.h"

// void hypno_event::setKeyDownCallback(std::function<void(int)> onKeyDown) {
//     hypno_event::getInstance().onKeyDown = onKeyDown;
// }

// void hypno_event::setKeyUpCallback(std::function<void(int)> onKeyUp) {
//     hypno_event::getInstance().onKeyUp = onKeyUp;
// }

// void hypno_event::setLDownCallback(std::function<void(int, int)> onLDown) {
//     hypno_event::getInstance().onLDown = onLDown;
// }

// void hypno_event::setLUpCallback(std::function<void(int, int)> onLUp) {
//     hypno_event::getInstance().onLUp = onLUp;
// }
// void hypno_event::setRDownCallback(std::function<void(int, int)> onRDown) {
//     hypno_event::getInstance().onRDown = onRDown;
// }

// void hypno_event::setRUpCallback(std::function<void(int, int)> onRUp) {
//     hypno_event::getInstance().onRUp = onRUp;
// }

// void hypno_event::setMoveCallback(std::function<void(int, int)> onMove) {
//     hypno_event::getInstance().onMove = onMove;
// }

#ifdef WINDOWS

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}

hypno_event::hypno_event() {
    screen_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screen_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

cv::Mat captureScreenMat(HWND hwnd)
{
    cv::Mat img, bgrim;

    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    int screen_x = hypno_event::getInstance().screen_x;
    int screen_y = hypno_event::getInstance().screen_y;
    int width  = hypno_event::getInstance().width;
    int height = hypno_event::getInstance().height;

    img.create(height, width, CV_8UC4);
    bgrim.create(height, width, CV_8UC3);

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    SelectObject(hwindowCompatibleDC, hbwindow);

    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screen_x, screen_y, width, height, SRCCOPY);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, img.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    cvtColor(img, bgrim, cv::COLOR_BGRA2BGR);

    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return bgrim;
}

cv::Mat hypno_event::captureScreen() {
    return captureScreenMat(GetDesktopWindow());
}

void hypno_event::sendKeyDown(int keyCode) {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::sendKeyUp(int keyCode) {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::toScreenCoord(int &x, int &y) {
    x = (long long)(x * 65536) / hypno_event::getInstance().width;
    y = (long long)(y * 65536) / hypno_event::getInstance().height;
}

void hypno_event::sendLDown(int x, int y) {
    toScreenCoord(x, y);
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::sendRDown(int x, int y) {
    toScreenCoord(x, y);
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::sendLUp(int x, int y) {
    toScreenCoord(x, y);
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::sendRUp(int x, int y) {
    toScreenCoord(x, y);
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTUP;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void hypno_event::sendMove(int x, int y) {
    toScreenCoord(x, y);
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

// LRESULT CALLBACK hypno_event::GlobalKeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     return hypno_event::getInstance().KeyboardHookCallback(nCode, wParam, lParam);
// }

// LRESULT CALLBACK hypno_event::KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode >= 0) {
//         KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
//         if (wParam == WM_KEYDOWN) {
//             hypno_event::getInstance().onKeyDown(kbdStruct->vkCode);
//         }
//         else if (wParam == WM_KEYUP) {
//             hypno_event::getInstance().onKeyUp(kbdStruct->vkCode);
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

// LRESULT CALLBACK hypno_event::GlobalMouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     return hypno_event::getInstance().MouseHookCallback(nCode, wParam, lParam);
// }

// LRESULT CALLBACK hypno_event::MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode >= 0) {
//         MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
//         int x = mouseInfo->pt.x;
//         int y = mouseInfo->pt.y;
//         if (wParam == WM_LBUTTONDOWN) {
//             hypno_event::getInstance().onLDown(x, y);
//         }
//         else if (wParam == WM_LBUTTONUP) {
//             hypno_event::getInstance().onLUp(x, y);
//         }
//         else if (wParam == WM_RBUTTONDOWN) {
//             hypno_event::getInstance().onRDown(x, y);
//         }
//         else if (wParam == WM_RBUTTONUP) {
//             hypno_event::getInstance().onRUp(x, y);
//         }
//         else if (wParam == WM_MOUSEMOVE) {
//             hypno_event::getInstance().onMove(x, y);
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

// void hypno_event::startHook() {
//     hypno_event::getInstance().keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardHookCallback, GetModuleHandle(NULL), 0);
//     hypno_event::getInstance().mouseHook = SetWindowsHookEx(WH_MOUSE_LL, GlobalMouseHookCallback, GetModuleHandle(NULL), 0);
// }

// void hypno_event::msgLoop() {
//     if (GetMessage(&hypno_event::getInstance().msg, NULL, 0, 0) > 0) {
//         TranslateMessage(&hypno_event::getInstance().msg);
//         DispatchMessage(&hypno_event::getInstance().msg);
//     }
// }

// void hypno_event::stopHook() {
//     UnhookWindowsHookEx(hypno_event::getInstance().keyboardHook);
// }

#else

hypno_event::hypno_event() {
    width = CGDisplayPixelsWide(CGMainDisplayID());
    height = CGDisplayPixelsHigh(CGMainDisplayID());
}

cv::Mat hypno_event::captureScreen()
{
    size_t width = hypno_event::getInstance().width;
    size_t height = hypno_event::getInstance().height;

    cv::Mat im(cv::Size(width,height), CV_8UC4);
    cv::Mat bgrim(cv::Size(width,height), CV_8UC3);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef contextRef = CGBitmapContextCreate(im.data, im.cols, im.rows, 8, im.step[0], colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault);

    CGImageRef imageRef = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), imageRef);
    cvtColor(im, bgrim, cv::COLOR_RGBA2BGR);

    CGImageRelease(imageRef);
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpace);

    return bgrim;
}

void hypno_event::sendKeyDown(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void hypno_event::sendKeyUp(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void hypno_event::sendLDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void hypno_event::sendLUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void hypno_event::sendRDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void hypno_event::sendRUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void hypno_event::sendMove(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventMouseMoved);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

// void hypno_event::startHook()
// {
//     CFMachPortRef &eventTap = hypno_event::getInstance().eventTap;
//     eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, kCGEventMaskForAllEvents, MyCGEventCallback, NULL);

//     if (!eventTap) {
//         std::cerr << "Failed to create event tap!" << std::endl;
//         return;
//     }

//     CFRunLoopSourceRef &runLoopSource = hypno_event::getInstance().runLoopSource;
//     runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
//     CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
//     CGEventTapEnable(eventTap, true);
// }

// CGEventRef hypno_event::MyCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
//     if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
//         CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
//         if (type == kCGEventKeyDown) {
//             hypno_event::getInstance().onKeyDown(keyCode);
//         }
//         else if (type == kCGEventKeyUp) {
//             hypno_event::getInstance().onKeyUp(keyCode);
//         }
//     }
//     else if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventLeftMouseUp || type == kCGEventRightMouseUp || type == kCGEventMouseMoved) {
//         CGPoint cursor = CGEventGetLocation(event);
//         int x = cursor.x;
//         int y = cursor.y;
//         if (type == kCGEventLeftMouseDown) {
//             hypno_event::getInstance().onLDown(x, y);
//         }
//         else if (type == kCGEventLeftMouseUp) {
//             hypno_event::getInstance().onLUp(x, y);
//         }
//         else if (type == kCGEventRightMouseDown) {
//             hypno_event::getInstance().onRDown(x, y);
//         }
//         else if (type == kCGEventRightMouseUp) {
//             hypno_event::getInstance().onRUp(x, y);
//         }
//         else if (type == kCGEventMouseMoved) {
//             hypno_event::getInstance().onMove(x, y);
//         }
//     }
//     return event;
// }

// void hypno_event::msgLoop() {
//     CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false); // Poll for events with a 0.1 second timeout
// }

// void hypno_event::stopHook() {
//     CFMachPortRef &eventTap = hypno_event::getInstance().eventTap;
//     if (eventTap) {
//         CGEventTapEnable(eventTap, false);
//         CFRelease(eventTap);
//     }
//     CFRunLoopSourceRef &runLoopSource = hypno_event::getInstance().runLoopSource;
//     if (runLoopSource) {
//         CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
//         CFRelease(runLoopSource);
//     }
// }

#endif
