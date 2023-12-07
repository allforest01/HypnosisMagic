#include "hypno_event.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

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

void HypnoEvent::emitKeyDown(int keyCode) {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void HypnoEvent::emitKeyUp(int keyCode) {
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

void HypnoEvent::emitLDown(int x, int y) {
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

void HypnoEvent::emitRDown(int x, int y) {
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

void HypnoEvent::emitLUp(int x, int y) {
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

void HypnoEvent::emitRUp(int x, int y) {
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

void HypnoEvent::emitMove(int x, int y) {
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

void HypnoEvent::emitKeyDown(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void HypnoEvent::emitKeyUp(int keyCode) {
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, keyCode, true);
    CGEventSetType(keyEvent, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void HypnoEvent::emitLDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::emitLUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventLeftMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::emitRDown(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseDown);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::emitRUp(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventRightMouseUp);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

void HypnoEvent::emitMove(int x, int y) {
    CGEventRef mouseEvent = CGEventCreate(nullptr);
    CGEventSetType(mouseEvent, kCGEventMouseMoved);
    CGEventSetLocation(mouseEvent, CGPointMake(x, y));
    CGEventPost(kCGHIDEventTap, mouseEvent);
    CFRelease(mouseEvent);
}

#endif