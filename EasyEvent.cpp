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

#endif
