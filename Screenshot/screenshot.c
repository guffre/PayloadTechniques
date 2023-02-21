#include <stdio.h>
#include <Windows.h>
#include <wingdi.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

unsigned char* TakeScreenShot(void) {
    unsigned char*   bmpFile = NULL;
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFO       bmpInfo;
    SecureZeroMemory(&bmpInfo, sizeof(BITMAPINFO));

    BITMAP bmpScreen;

    // The width and height of the virtual screen, in pixels. The virtual screen is the bounding rectangle of all display monitors.
    int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // If there are multiple monitors on the system, calling CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL) will create a DC covering all the monitors.
    HDC hScreenDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    
    // Create an in-memory bitmap and handle to the bitmap data
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);

    // Copy screen data into the in-memory bitmap
    BitBlt(hMemoryDC,   // A handle to the destination device context.
           0,           // The x-coordinate, in logical units, of the upper-left corner of the destination rectangle.
           0,           // The y-coordinate, in logical units, of the upper-left corner of the destination rectangle.
           width,       // The width, in logical units, of the source and destination rectangles.
           height,      // The width, in logical units, of the source and destination rectangles.
           hScreenDC,   // A handle to the source device context.
           GetSystemMetrics(SM_XVIRTUALSCREEN), // The x-coordinate, in logical units, of the upper-left corner of the source rectangle.
           GetSystemMetrics(SM_YVIRTUALSCREEN), // The y-coordinate, in logical units, of the upper-left corner of the source rectangle.
           SRCCOPY      // A raster-operation code
    );

    // Create bitmap file header and file info
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    GetDIBits(hScreenDC, hBitmap, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
    if (bmpInfo.bmiHeader.biSizeImage <= 0) {
        bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth*abs(bmpInfo.bmiHeader.biHeight)*(bmpInfo.bmiHeader.biBitCount + 7) / 8;
    }

    bmpFileHeader.bfReserved1 = 0;
    bmpFileHeader.bfReserved2 = 0;
    bmpFileHeader.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpInfo.bmiHeader.biSizeImage;
    bmpFileHeader.bfType      = 'MB';
    bmpFileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    // Allocate space for the bitmap file in memory
    if ((bmpFile = calloc(1, bmpFileHeader.bfSize)) != NULL) {
        // Create the actual bitmap file in memory
        GetDIBits(hScreenDC,                    // Handle to device context
                  hBitmap,                      // Handle to bitmap
                  0,                            // First scan line to retrieve
                  bmpInfo.bmiHeader.biHeight,   // Number of scan lines to retrieve
                  bmpFile+bmpFileHeader.bfOffBits, // [out] Pointer to buffer to receive bitmap data
                  &bmpInfo,                     // [in, out] A pointer to a BITMAPINFO structure that specifies the desired format for the DIB data.
                  DIB_RGB_COLORS                // The format of the bmiColors member of the BITMAPINFO structure.
        );
        memcpy(bmpFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER));
        memcpy(bmpFile + sizeof(BITMAPFILEHEADER), &bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER));
    }

    // Clean-up
    if (hMemoryDC) { DeleteDC(hMemoryDC);  }
    if (hScreenDC) { DeleteDC(hScreenDC);  }
    if (hBitmap)   { DeleteObject(hBitmap);}

    return bmpFile;
}

int main(int argc, char** argv) {
    unsigned char* bmp = TakeScreenShot();

    FILE* fd = fopen("screenshot_example.bmp", "wb");
    fwrite(bmp, 1, (*(BITMAPFILEHEADER*)(bmp)).bfSize, fd);
    fflush(fd);
    fclose(fd);
}