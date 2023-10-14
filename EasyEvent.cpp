#include "EasyEvent.h"

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

void EasyEvent::setKeydownCallback(void (*KeydownCallback)(int)) {
    this->KeydownCallback = KeydownCallback;
}

void EasyEvent::setKeyupCallback(void (*KeyupCallback)(int)) {
    this->KeyupCallback = KeyupCallback;
}

LRESULT CALLBACK GlobalKeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    return EasyEvent::getInstance().KeyboardHookCallback(nCode, wParam, lParam);
}

LRESULT CALLBACK EasyEvent::KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN) {
            this->KeydownCallback(kbdStruct->vkCode);
        }
        else if (wParam == WM_KEYUP) {
            this->KeyupCallback(kbdStruct->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void EasyEvent::StartHook() {
    this->keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardHookCallback, GetModuleHandle(NULL), 0);
}

void EasyEvent::MsgLoop() {
    if (GetMessage(&this->msg, NULL, 0, 0) > 0) {
        TranslateMessage(&this->msg);
        DispatchMessage(&this->msg);
    }
}

void EasyEvent::Unhook() {
    UnhookWindowsHookEx(this->keyboardHook);
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

#endif
