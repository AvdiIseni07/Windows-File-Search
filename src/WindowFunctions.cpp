#include "MyApp.h"
#include <ShlObj_core.h>

void MyApp::ResizeWindow(int new_width, int new_height)
{
    if (!window_)
        return;

    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
        return;

    // Resize in place
    SetWindowPos(
        hwnd,
        nullptr,               // keep same Z-order
        rect.left, rect.top,   // keep same x/y
        new_width, new_height, // new width/height
        SWP_NOZORDER           // don’t move in z-order
    );

    overlay_->Resize((uint32_t)new_width, (uint32_t)new_height);
}

void MyApp::RemoveTaskbarIcon()
{
    ITaskbarList *pTaskList = NULL;
    HRESULT initRet = CoInitialize(NULL);
    HRESULT createRet = CoCreateInstance(
        CLSID_TaskbarList,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskbarList,
        (LPVOID *)&pTaskList);

    if (createRet == S_OK)
    {
        pTaskList->DeleteTab(hwnd);
        pTaskList->Release();
    }

    CoUninitialize();
}

void MyApp::RoundCorners(int width, int height, int radius)
{
    HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, radius * 2, radius * 2);
    SetWindowRgn(hwnd, region, TRUE);
}