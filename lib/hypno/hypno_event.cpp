#include "hypno_event.h"

// void HypnoEvent::setKeyDownCallback(std::function<void(int)> onKeyDown) {
//     HypnoEvent::getInstance().onKeyDown = onKeyDown;
// }

// void HypnoEvent::setKeyUpCallback(std::function<void(int)> onKeyUp) {
//     HypnoEvent::getInstance().onKeyUp = onKeyUp;
// }

// void HypnoEvent::setLDownCallback(std::function<void(int, int)> onLDown) {
//     HypnoEvent::getInstance().onLDown = onLDown;
// }

// void HypnoEvent::setLUpCallback(std::function<void(int, int)> onLUp) {
//     HypnoEvent::getInstance().onLUp = onLUp;
// }
// void HypnoEvent::setRDownCallback(std::function<void(int, int)> onRDown) {
//     HypnoEvent::getInstance().onRDown = onRDown;
// }

// void HypnoEvent::setRUpCallback(std::function<void(int, int)> onRUp) {
//     HypnoEvent::getInstance().onRUp = onRUp;
// }

// void HypnoEvent::setMoveCallback(std::function<void(int, int)> onMove) {
//     HypnoEvent::getInstance().onMove = onMove;
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

HypnoEvent::HypnoEvent() {
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

    int screen_x = HypnoEvent::getInstance().screen_x;
    int screen_y = HypnoEvent::getInstance().screen_y;
    int width  = HypnoEvent::getInstance().width;
    int height = HypnoEvent::getInstance().height;

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

cv::Mat HypnoEvent::captureScreen() {
    return captureScreenMat(GetDesktopWindow());
}

void HypnoEvent::sendKeyDown(int keyCode) {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void HypnoEvent::sendKeyUp(int keyCode) {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void HypnoEvent::toScreenCoord(int &x, int &y) {
    x = (long long)(x * 65536) / HypnoEvent::getInstance().width;
    y = (long long)(y * 65536) / HypnoEvent::getInstance().height;
}

void HypnoEvent::sendLDown(int x, int y) {
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

void HypnoEvent::sendRDown(int x, int y) {
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

void HypnoEvent::sendLUp(int x, int y) {
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

void HypnoEvent::sendRUp(int x, int y) {
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

void HypnoEvent::sendMove(int x, int y) {
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

// LRESULT CALLBACK HypnoEvent::GlobalKeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     return HypnoEvent::getInstance().KeyboardHookCallback(nCode, wParam, lParam);
// }

// LRESULT CALLBACK HypnoEvent::KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode >= 0) {
//         KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
//         if (wParam == WM_KEYDOWN) {
//             HypnoEvent::getInstance().onKeyDown(kbdStruct->vkCode);
//         }
//         else if (wParam == WM_KEYUP) {
//             HypnoEvent::getInstance().onKeyUp(kbdStruct->vkCode);
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

// LRESULT CALLBACK HypnoEvent::GlobalMouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     return HypnoEvent::getInstance().MouseHookCallback(nCode, wParam, lParam);
// }

// LRESULT CALLBACK HypnoEvent::MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
//     if (nCode >= 0) {
//         MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
//         int x = mouseInfo->pt.x;
//         int y = mouseInfo->pt.y;
//         if (wParam == WM_LBUTTONDOWN) {
//             HypnoEvent::getInstance().onLDown(x, y);
//         }
//         else if (wParam == WM_LBUTTONUP) {
//             HypnoEvent::getInstance().onLUp(x, y);
//         }
//         else if (wParam == WM_RBUTTONDOWN) {
//             HypnoEvent::getInstance().onRDown(x, y);
//         }
//         else if (wParam == WM_RBUTTONUP) {
//             HypnoEvent::getInstance().onRUp(x, y);
//         }
//         else if (wParam == WM_MOUSEMOVE) {
//             HypnoEvent::getInstance().onMove(x, y);
//         }
//     }
//     return CallNextHookEx(NULL, nCode, wParam, lParam);
// }

// void HypnoEvent::startHook() {
//     HypnoEvent::getInstance().keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardHookCallback, GetModuleHandle(NULL), 0);
//     HypnoEvent::getInstance().mouseHook = SetWindowsHookEx(WH_MOUSE_LL, GlobalMouseHookCallback, GetModuleHandle(NULL), 0);
// }

// void HypnoEvent::msgLoop() {
//     if (GetMessage(&HypnoEvent::getInstance().msg, NULL, 0, 0) > 0) {
//         TranslateMessage(&HypnoEvent::getInstance().msg);
//         DispatchMessage(&HypnoEvent::getInstance().msg);
//     }
// }

// void HypnoEvent::stopHook() {
//     UnhookWindowsHookEx(HypnoEvent::getInstance().keyboardHook);
// }

#else

HypnoEvent::HypnoEvent() {
    width = CGDisplayPixelsWide(CGMainDisplayID());
    height = CGDisplayPixelsHigh(CGMainDisplayID());
}

cv::Mat HypnoEvent::captureScreen()
{
    size_t width = HypnoEvent::getInstance().width;
    size_t height = HypnoEvent::getInstance().height;

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

void HypnoEvent::sendKeyDown(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void HypnoEvent::sendKeyUp(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void HypnoEvent::sendLDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::sendLUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::sendRDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::sendRUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::sendMove(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventMouseMoved);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

// void HypnoEvent::startHook()
// {
//     CFMachPortRef &eventTap = HypnoEvent::getInstance().eventTap;
//     eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, kCGEventMaskForAllEvents, MyCGEventCallback, NULL);

//     if (!eventTap) {
//         std::cerr << "Failed to create event tap!" << std::endl;
//         return;
//     }

//     CFRunLoopSourceRef &runLoopSource = HypnoEvent::getInstance().runLoopSource;
//     runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
//     CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
//     CGEventTapEnable(eventTap, true);
// }

// CGEventRef HypnoEvent::MyCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
//     if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
//         CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
//         if (type == kCGEventKeyDown) {
//             HypnoEvent::getInstance().onKeyDown(keyCode);
//         }
//         else if (type == kCGEventKeyUp) {
//             HypnoEvent::getInstance().onKeyUp(keyCode);
//         }
//     }
//     else if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventLeftMouseUp || type == kCGEventRightMouseUp || type == kCGEventMouseMoved) {
//         CGPoint cursor = CGEventGetLocation(event);
//         int x = cursor.x;
//         int y = cursor.y;
//         if (type == kCGEventLeftMouseDown) {
//             HypnoEvent::getInstance().onLDown(x, y);
//         }
//         else if (type == kCGEventLeftMouseUp) {
//             HypnoEvent::getInstance().onLUp(x, y);
//         }
//         else if (type == kCGEventRightMouseDown) {
//             HypnoEvent::getInstance().onRDown(x, y);
//         }
//         else if (type == kCGEventRightMouseUp) {
//             HypnoEvent::getInstance().onRUp(x, y);
//         }
//         else if (type == kCGEventMouseMoved) {
//             HypnoEvent::getInstance().onMove(x, y);
//         }
//     }
//     return event;
// }

// void HypnoEvent::msgLoop() {
//     CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false); // Poll for events with a 0.1 second timeout
// }

// void HypnoEvent::stopHook() {
//     CFMachPortRef &eventTap = HypnoEvent::getInstance().eventTap;
//     if (eventTap) {
//         CGEventTapEnable(eventTap, false);
//         CFRelease(eventTap);
//     }
//     CFRunLoopSourceRef &runLoopSource = HypnoEvent::getInstance().runLoopSource;
//     if (runLoopSource) {
//         CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
//         CFRelease(runLoopSource);
//     }
// }

#endif
