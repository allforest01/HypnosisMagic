#include "EasyEvent.h"

void EasyEvent::setKeyDownCallback(void (*KeyDownCallback)(int)) {
    EasyEvent::getInstance().KeyDownCallback = KeyDownCallback;
}

void EasyEvent::setKeyUpCallback(void (*KeyUpCallback)(int)) {
    EasyEvent::getInstance().KeyUpCallback = KeyUpCallback;
}

void EasyEvent::setLDownCallback(void (*LDownCallback)()) {
    EasyEvent::getInstance().LDownCallback = LDownCallback;
}

void EasyEvent::setLUpCallback(void (*LUpCallback)()) {
    EasyEvent::getInstance().LUpCallback = LUpCallback;
}
void EasyEvent::setRDownCallback(void (*RDownCallback)()) {
    EasyEvent::getInstance().RDownCallback = RDownCallback;
}

void EasyEvent::setRUpCallback(void (*RUpCallback)()) {
    EasyEvent::getInstance().RUpCallback = RUpCallback;
}

void EasyEvent::setMoveCallback(void (*MoveCallback)(int, int)) {
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

cv::Mat captureScreenMat(HWND hwnd)
{
    cv::Mat src;

    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = 2560; // GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = 1600; //GetSystemMetrics(SM_CYVIRTUALSCREEN);

    src.create(height, width, CV_8UC4);

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    SelectObject(hwindowCompatibleDC, hbwindow);

    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

cv::Mat EasyEvent::CaptureScreen() {
    return captureScreenMat(GetDesktopWindow());
}

void EasyEvent::SendKeyPress(int os, int keyCode) {
    if (os == 1) keyCode = KeyMapping::getInstance().convertMacToWindows(keyCode);
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void EasyEvent::SendKeyRelease(int os, int keyCode) {
    if (os == 1) keyCode = KeyMapping::getInstance().convertMacToWindows(keyCode);
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
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
        if (wParam == WM_LBUTTONDOWN) {
            EasyEvent::getInstance().LDownCallback();
        }
        else if (wParam == WM_LBUTTONUP) {
            EasyEvent::getInstance().LUpCallback();
        }
        else if (wParam == WM_RBUTTONDOWN) {
            EasyEvent::getInstance().RDownCallback();
        }
        else if (wParam == WM_RBUTTONUP) {
            EasyEvent::getInstance().RUpCallback();
        }
        else if (wParam == WM_MOUSEMOVE) {
            int x = mouseInfo->pt.x;
            int y = mouseInfo->pt.y;
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

cv::Mat EasyEvent::CaptureScreen()
{
    size_t width = 1440; // CGDisplayPixelsWide(CGMainDisplayID());
    size_t height = 900; // CGDisplayPixelsHigh(CGMainDisplayID());

    cv::Mat im(cv::Size(width,height), CV_8UC4);
    cv::Mat bgrim(cv::Size(width,height), CV_8UC3);
    cv::Mat resizedim(cv::Size(width,height), CV_8UC3);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef contextRef = CGBitmapContextCreate(im.data, im.cols, im.rows, 8, im.step[0], colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault);

    CGImageRef imageRef = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), imageRef);
    cvtColor(im, bgrim, cv::COLOR_RGBA2BGR);
    // resize(bgrim, resizedim,cv::Size(),0.5,0.5);
    CGImageRelease(imageRef);
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpace);

    return bgrim;
}

void EasyEvent::SendKeyPress(int os, int keyCode) {
    if (os == 0) keyCode = KeyMapping::getInstance().convertWindowsToMac(keyCode);
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void EasyEvent::SendKeyRelease(int os, int keyCode) {
    if (os == 0) keyCode = KeyMapping::getInstance().convertWindowsToMac(keyCode);
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void EasyEvent::StartHook()
{
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
        CGEventMaskBit(kCGEventKeyDown), MyCGEventCallback, NULL
    );

    if (!eventTap) {
        std::cerr << "Failed to create event tap!" << std::endl;
        return;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

    CGEventTapEnable(eventTap, true);
}

CGEventRef EasyEvent::MyCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    if (type == kCGEventKeyDown) {
        EasyEvent::getInstance().KeydownCallback(keyCode);
    }
    else if (type == kCGEventKeyUp) {
        EasyEvent::getInstance().KeydownCallback(keyCode);
    }
    return event;
}

void EasyEvent::MsgLoop() {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false); // Poll for events with a 0.1 second timeout
}

void EasyEvent::Unhook() {
    if (eventTap) {
        CGEventTapEnable(eventTap, false);
        CFRelease(eventTap);
    }
    if (runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CFRelease(runLoopSource);
    }
}

#endif
