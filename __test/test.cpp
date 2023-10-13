#include <Windows.h>
#include <opencv2/opencv.hpp>

using namespace cv;

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER bi;

    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // This is the line that makes it draw upside down or not
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

Mat captureScreenMat(HWND hwnd)
{
    Mat src;

    // Get handles to a device context (DC)
    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // Define scale, height, and width
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = 2560; // GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = 1600; //GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // Create Mat object
    src.create(height, width, CV_8UC4);

    // Create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // Use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // Copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

int main()
{
    // Capture image
    HWND hwnd = GetDesktopWindow();
    Mat src = captureScreenMat(hwnd);

    // Save image
    // cv::imwrite("Screenshot.png", src);
    cv::imshow("Screenshoot", src);
    cv::waitKey(0);
}
