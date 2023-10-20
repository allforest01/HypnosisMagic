#include "EasyEvent.h"

void EasyEvent::setKeyDownCallback(std::function<void(int)> KeyDownCallback) {
    EasyEvent::getInstance().KeyDownCallback = KeyDownCallback;
}

void EasyEvent::setKeyUpCallback(std::function<void(int)> KeyUpCallback) {
    EasyEvent::getInstance().KeyUpCallback = KeyUpCallback;
}

void EasyEvent::setLDownCallback(std::function<void(int, int)> LDownCallback) {
    EasyEvent::getInstance().LDownCallback = LDownCallback;
}

void EasyEvent::setLUpCallback(std::function<void(int, int)> LUpCallback) {
    EasyEvent::getInstance().LUpCallback = LUpCallback;
}
void EasyEvent::setRDownCallback(std::function<void(int, int)> RDownCallback) {
    EasyEvent::getInstance().RDownCallback = RDownCallback;
}

void EasyEvent::setRUpCallback(std::function<void(int, int)> RUpCallback) {
    EasyEvent::getInstance().RUpCallback = RUpCallback;
}

void EasyEvent::setMoveCallback(std::function<void(int, int)> MoveCallback) {
    EasyEvent::getInstance().MoveCallback = MoveCallback;
}

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

EasyEvent::EasyEvent() {
    screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

cv::Mat captureScreenMat(HWND hwnd)
{
    cv::Mat img, bgrim;

    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    int screenx = EasyEvent::getInstance().screenx;
    int screeny = EasyEvent::getInstance().screeny;
    int width = EasyEvent::getInstance().width;
    int height = EasyEvent::getInstance().height;

    img.create(height, width, CV_8UC4);
    bgrim.create(height, width, CV_8UC3);

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    SelectObject(hwindowCompatibleDC, hbwindow);

    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, img.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    cvtColor(img, bgrim, cv::COLOR_BGRA2BGR);

    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return bgrim;
}

cv::Mat EasyEvent::CaptureScreen() {
    return captureScreenMat(GetDesktopWindow());
}

void EasyEvent::SendKeyDown(os keyos, int keyCode) {
    if (keyos == mac) keyCode = MacKeyToWinKey(keyCode);
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void EasyEvent::SendKeyUp(os keyos, int keyCode) {
    if (keyos == mac) keyCode = MacKeyToWinKey(keyCode);
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void EasyEvent::toScrCoor(int &x, int &y) {
    x = (x * 65536) / EasyEvent::getInstance().width;
    y = (y * 65536) / EasyEvent::getInstance().height;
}

void EasyEvent::SendLDown(int x, int y) {
    toScrCoor(x, y);
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

void EasyEvent::SendRDown(int x, int y) {
    toScrCoor(x, y);
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

void EasyEvent::SendLUp(int x, int y) {
    toScrCoor(x, y);
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

void EasyEvent::SendRUp(int x, int y) {
    toScrCoor(x, y);
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

void EasyEvent::SendMove(int x, int y) {
    toScrCoor(x, y);
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

LRESULT CALLBACK EasyEvent::GlobalKeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    return EasyEvent::getInstance().KeyboardHookCallback(nCode, wParam, lParam);
}

LRESULT CALLBACK EasyEvent::KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN) {
            EasyEvent::getInstance().KeyDownCallback(kbdStruct->vkCode);
        }
        else if (wParam == WM_KEYUP) {
            EasyEvent::getInstance().KeyUpCallback(kbdStruct->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK EasyEvent::GlobalMouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    return EasyEvent::getInstance().MouseHookCallback(nCode, wParam, lParam);
}

LRESULT CALLBACK EasyEvent::MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        int x = mouseInfo->pt.x;
        int y = mouseInfo->pt.y;
        if (wParam == WM_LBUTTONDOWN) {
            EasyEvent::getInstance().LDownCallback(x, y);
        }
        else if (wParam == WM_LBUTTONUP) {
            EasyEvent::getInstance().LUpCallback(x, y);
        }
        else if (wParam == WM_RBUTTONDOWN) {
            EasyEvent::getInstance().RDownCallback(x, y);
        }
        else if (wParam == WM_RBUTTONUP) {
            EasyEvent::getInstance().RUpCallback(x, y);
        }
        else if (wParam == WM_MOUSEMOVE) {
            EasyEvent::getInstance().MoveCallback(x, y);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void EasyEvent::StartHook() {
    EasyEvent::getInstance().keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardHookCallback, GetModuleHandle(NULL), 0);
    EasyEvent::getInstance().mouseHook = SetWindowsHookEx(WH_MOUSE_LL, GlobalMouseHookCallback, GetModuleHandle(NULL), 0);
}

void EasyEvent::MsgLoop() {
    if (GetMessage(&EasyEvent::getInstance().msg, NULL, 0, 0) > 0) {
        TranslateMessage(&EasyEvent::getInstance().msg);
        DispatchMessage(&EasyEvent::getInstance().msg);
    }
}

void EasyEvent::Unhook() {
    UnhookWindowsHookEx(EasyEvent::getInstance().keyboardHook);
}

#else

EasyEvent::EasyEvent() {
    width = CGDisplayPixelsWide(CGMainDisplayID());
    height = CGDisplayPixelsHigh(CGMainDisplayID());
}

cv::Mat EasyEvent::CaptureScreen()
{
    size_t width = EasyEvent::getInstance().width;
    size_t height = EasyEvent::getInstance().height;

    cv::Mat im(cv::Size(width,height), CV_8UC4);
    cv::Mat bgrim(cv::Size(width,height), CV_8UC3);
    // cv::Mat resizedim(cv::Size(width,height), CV_8UC3);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef contextRef = CGBitmapContextCreate(im.data, im.cols, im.rows, 8, im.step[0], colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault);

    CGImageRef imageRef = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), imageRef);
    cvtColor(im, bgrim, cv::COLOR_RGBA2BGR);
    // resize(bgrim, resizedim,cv::Size(),0.5,0.5);
    CGImageRelease(imageRef);
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpace);

    return bgrim; // resizedim;
}

void EasyEvent::SendKeyDown(os keyos, int keyCode) {
    if (keyos == win) keyCode = WinKeyToMacKey(keyCode);
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void EasyEvent::SendKeyUp(os keyos, int keyCode) {
    if (keyos == win) keyCode = WinKeyToMacKey(keyCode);
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void EasyEvent::SendLDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void EasyEvent::SendLUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void EasyEvent::SendRDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void EasyEvent::SendRUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void EasyEvent::SendMove(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventMouseMoved);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void EasyEvent::StartHook()
{
    CFMachPortRef &eventTap = EasyEvent::getInstance().eventTap;
    eventTap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, kCGEventMaskForAllEvents, MyCGEventCallback, NULL);

    if (!eventTap) {
        std::cerr << "Failed to create event tap!" << std::endl;
        return;
    }

    CFRunLoopSourceRef &runLoopSource = EasyEvent::getInstance().runLoopSource;
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
}

CGEventRef EasyEvent::MyCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        if (type == kCGEventKeyDown) {
            EasyEvent::getInstance().KeyDownCallback(keyCode);
        }
        else if (type == kCGEventKeyUp) {
            EasyEvent::getInstance().KeyUpCallback(keyCode);
        }
    }
    else if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventLeftMouseUp || type == kCGEventRightMouseUp || type == kCGEventMouseMoved) {
        CGPoint cursor = CGEventGetLocation(event);
        int x = cursor.x;
        int y = cursor.y;
        if (type == kCGEventLeftMouseDown) {
            EasyEvent::getInstance().LDownCallback(x, y);
        }
        else if (type == kCGEventLeftMouseUp) {
            EasyEvent::getInstance().LUpCallback(x, y);
        }
        else if (type == kCGEventRightMouseDown) {
            EasyEvent::getInstance().RDownCallback(x, y);
        }
        else if (type == kCGEventRightMouseUp) {
            EasyEvent::getInstance().RUpCallback(x, y);
        }
        else if (type == kCGEventMouseMoved) {
            EasyEvent::getInstance().MoveCallback(x, y);
        }
    }
    return event;
}

void EasyEvent::MsgLoop() {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false); // Poll for events with a 0.1 second timeout
}

void EasyEvent::Unhook() {
    CFMachPortRef &eventTap = EasyEvent::getInstance().eventTap;
    if (eventTap) {
        CGEventTapEnable(eventTap, false);
        CFRelease(eventTap);
    }
    CFRunLoopSourceRef &runLoopSource = EasyEvent::getInstance().runLoopSource;
    if (runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CFRelease(runLoopSource);
    }
}

#endif
