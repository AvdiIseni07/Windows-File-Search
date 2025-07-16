#include "MyApp.h"
#include <iostream>
#include "JavaScriptCore/JavaScript.h"
#include <Ultralight/Ultralight.h>
#include <Ultralight/ConsoleMessage.h>
#include "platform/EmbeddedFileSystem.h"
#include "AppCore/Window.h"
#include <Windows.h>
#include <ShlObj_core.h>

#include <string>
#include <inttypes.h>
#include "File.h"

#include <WinUser.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

using namespace ultralight;

RefPtr<Window> win;
RefPtr<Overlay> _ovrlay;

void RoundCorners(HWND hwnd, int width, int height, int radius)
{
    HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, radius * 2, radius * 2);
    SetWindowRgn(hwnd, region, TRUE);
}

void removeTaskbarIcon()
{
    HWND hwnd = reinterpret_cast<HWND>(win->native_handle());

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

// Needs to be done via windows functions so it is not cross-platform.
void ResizeWindow(int new_width, int new_height)
{
    if (!win)
        return;

    // Grab the native HWND out of the RefPtr
    HWND hwnd = reinterpret_cast<HWND>(win->native_handle());
    // ShowWindow(hwnd, SW_HIDE);

    // RoundCorners(hwnd, new_width, new_height, 5);
    // Fetch current position so we only change size
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

    _ovrlay->Resize((uint32_t)new_width, (uint32_t)new_height);
}

MyApp::MyApp()
{
    ///
    /// Create our main App instance.
    ///
    app_ = App::Create();

    ///
    /// Create a resizable window by passing by OR'ing our window flags with
    /// kWindowFlags_Resizable.
    ///
    window_ = Window::Create(app_->main_monitor(), WINDOW_WIDTH, WINDOW_HEIGHT,
                             false, kWindowFlags_Borderless);

    ///
    /// Create our HTML overlay-- we don't care about its initial size and
    /// position because it'll be calculated when we call OnResize() below.
    ///
    overlay_ = Overlay::Create(window_, 1, 1, 0, 0);
    _ovrlay = overlay_;

    // Set the size
    win = window_;
    // ResizeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

    ///
    /// Force a call to OnResize to perform size/layout of our overlay.
    ///
    OnResize(window_.get(), window_->width(), window_->height());

    ///
    /// Load a   into our overlay's View
    ///
    overlay_->view()->LoadURL("file:///app.html");

    ///
    /// Register our MyApp instance as an AppListener so we can handle the
    /// App's OnUpdate event below.
    ///
    app_->set_listener(this);

    ///
    /// Register our MyApp instance as a WindowListener so we can handle the
    /// Window's OnResize event below.
    ///
    window_->set_listener(this);

    ///
    /// Register our MyApp instance as a LoadListener so we can handle the
    /// View's OnFinishLoading and OnDOMReady events below.
    ///
    overlay_->view()->set_load_listener(this);

    ///
    /// Register our MyApp instance as a ViewListener so we can handle the
    /// View's OnChangeCursor and OnChangeTitle events below.
    ///
    overlay_->view()->set_view_listener(this);
}

MyApp::~MyApp()
{
}

void MyApp::Run()
{
    app_->Run();
}

void toLowerCase(std::string &str)
{
    for (char &c : str)
    {
        if (c >= 65 && c <= 90)
        {
            c += 32;
        }
    }
}

ultralight::View *mainCaller;
std::string rootPath = "C:";
std::vector<File> results;

void listenForOpenCommand()
{
}

bool open = true;
unsigned int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;

void MyApp::OnUpdate()
{
    ///
    /// This is called repeatedly from the application's update loop.
    ///

    // Show/Hide the window.
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate
    if (GetAsyncKeyState(VK_LMENU) == -32767)
    {
        if (open)
            ResizeWindow(0, 0);
        else
            ResizeWindow(WINDOW_WIDTH, height);

        open = !open;
    }
}

void MyApp::OnClose(ultralight::Window *window)
{
    app_->Quit();
}

void MyApp::OnResize(ultralight::Window *window, uint32_t width, uint32_t height)
{
    ///
    /// This is called whenever the window changes size (values in pixels).
    ///
    /// We resize our overlay here to take up the entire window.
    ///
    overlay_->Resize(width, height);
}

void MyApp::OnFinishLoading(ultralight::View *caller,
                            uint64_t frame_id,
                            bool is_main_frame,
                            const String &url)
{
    ///
    /// This is called when a frame finishes loading on the page.
    ///
}

JSValueRef OnSearch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    if (mainCaller == nullptr)
        return JSValueMakeNull(ctx);

    String res = mainCaller->EvaluateScript("document.getElementById('search').value;");
    String script = "message('" + res + "')";

    std::string input = std::string(res.utf8().data(), res.utf8().length());
    toLowerCase(input);

    std::string filtered = input;
    filtered.erase(std::remove(filtered.begin(), filtered.end(), ' '), filtered.end());

    /*
    int len = filtered.length();
    String adaptedLen = String(std::to_string(len).c_str());
    mainCaller->EvaluateScript("message('" + adaptedLen + "')");
    */

    // uint32_t windth = 100, height = 100;

    // win->SetSize(windth, height);
    // n++;
    /* String getElements = mainCaller->EvaluateScript("document.getElementById('list').childElementCount;");
     std::string numOfElements = std::string(getElements.utf8().data(), getElements.utf8().length());
     int el = std::stoi(numOfElements);
     height = 100 + el * 50;

     if (height > 300 || el > 100)
         height = 300;

     ResizeWindow(WINDOW_WIDTH, height);*/

    String getHeight = mainCaller->EvaluateScript("document.getElementById('list').getBoundingClientRect().height;");
    std::string heightStr = std::string(getHeight.utf8().data(), getHeight.utf8().length());
    int listHeight = std::stoi(heightStr);

    // Add top padding, input, title, etc.
    height = 300 + listHeight;
    if (height > 600)
        height = 600;
    ResizeWindow(WINDOW_WIDTH, height);

    if (filtered.length() > 0)
    {

        // Removing old results
        // if (results.size() > 0)

        for (int i = 0; i < results.size(); i++)
        {
            auto file = results[i];
            std::string nameToString = std::string(file.GetName().utf8().data(), file.GetName().utf8().length());
            toLowerCase(nameToString);

            if (nameToString.find(input) == std::string::npos)
            {
                // mainCaller->EvaluateScript("message('deleting')");
                String script = "deleteElement('" + file.GetName() + "')";
                mainCaller->EvaluateScript(script);
                results.erase(results.begin() + i);
                i--;
            }
        }

        // Showing results
        try
        {
            for (const fs::directory_entry &entry : fs::directory_iterator(rootPath))
            {
                std::string fileName = entry.path().filename().u8string();
                toLowerCase(fileName);

                String adaptedFileName = String(fileName.c_str());

                if (fileName.find(input) != std::string::npos)
                {
                    // Making sure the same file doesn't appear twice
                    bool skip = false;
                    for (auto &file : results)
                    {
                        if (file.GetName() == adaptedFileName)
                        {
                            skip = true;
                            break;
                        }
                    }

                    if (skip)
                        continue;

                    // mainCaller->EvaluateScript("message('Adding')");

                    // Adding the file
                    std::string path = entry.path().u8string();
                    String adaptedPath = String(path.c_str());
                    auto currentFile = File(entry, adaptedFileName, adaptedPath);
                    results.push_back(currentFile);

                    String script = "create('" + adaptedFileName + "')";
                    mainCaller->EvaluateScript(script);
                }
            }
        }
        catch (std::system_error e)
        {
        }
    }
    else
    {
        if (results.size() == 0)
            return JSValueMakeNull(ctx);

        /// Clear the search.
        for (auto &file : results)
        {
            String script = "deleteElement('" + file.GetName() + "')";
            mainCaller->EvaluateScript(script);
        }
        results.clear();
    }

    return JSValueMakeNull(ctx);
}

JSValueRef OnClick(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    JSStringRef jsStr = JSValueToStringCopy(ctx, arguments[0], nullptr);
    size_t maxSize = JSStringGetMaximumUTF8CStringSize(jsStr);
    std::vector<char> buffer(maxSize);
    JSStringGetUTF8CString(jsStr, buffer.data(), maxSize);
    JSStringRelease(jsStr);

    std::string id = buffer.data();
    String msg = String(id.c_str());

    for (auto &file : results)
    {
        std::string nm = std::string(file.GetName().utf8().data(), file.GetName().utf8().length());
        if (id == nm)
        {
            // std::string pth = std::string(file.GetPath().utf8().data(),file.GetPath().utf8().length());
            // mainCaller->EvaluateScript("message('" + file.GetPath() + "');");

            file.OpenFile();
            break;
        }
    }

    return JSValueMakeNull(ctx);
}

void MyApp::OnDOMReady(ultralight::View *caller,
                       uint64_t frame_id,
                       bool is_main_frame,
                       const String &url)
{
    listenForOpenCommand();
    removeTaskbarIcon();

    ///
    /// This is called when a frame's DOM has finished loading on the page.
    ///
    /// This is the best time to setup any JavaScript bindings.
    ///

    if (!is_main_frame)
        return;

    mainCaller = caller;

    // Acquire the JS execution context for the current page.
    auto scoped_context = caller->LockJSContext();

    // Typecast to the underlying JSContextRef.
    JSContextRef ctx = (*scoped_context);

    // Create a JavaScript String containing the name of our callback.
    JSStringRef onSearchName = JSStringCreateWithUTF8CString("OnSearch");

    // Create a garbage-collected JavaScript function that is bound to our
    // native C callback 'OnSearch()'.
    JSObjectRef OnSearchFunc = JSObjectMakeFunctionWithCallback(ctx, onSearchName,
                                                                OnSearch);

    // Get the global JavaScript object (aka 'window')
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx);

    // Store our function in the page's global JavaScript object so that it
    // accessible from the page as 'OnButtonClick()'.
    JSObjectSetProperty(ctx, globalObj, onSearchName, OnSearchFunc, 0, 0);

    // Release the JavaScript String we created earlier.
    JSStringRelease(onSearchName);

    JSStringRef onClickName = JSStringCreateWithUTF8CString("OnClick");
    JSObjectRef onClickFunc = JSObjectMakeFunctionWithCallback(ctx, onClickName, OnClick);
    JSObjectSetProperty(ctx, globalObj, onClickName, onClickFunc, 0, 0);

    JSStringRelease(onClickName);
}

void MyApp::OnChangeCursor(ultralight::View *caller,
                           Cursor cursor)
{
    ///
    /// This is called whenever the page requests to change the cursor.
    ///
    /// We update the main window's cursor here.
    ///
    // test++;
    window_->SetCursor(cursor);
}

void MyApp::OnChangeTitle(ultralight::View *caller,
                          const String &title)
{
    ///
    /// This is called whenever the page requests to change the title.
    ///
    /// We update the main window's title here.
    ///
    window_->SetTitle(title.utf8().data());
}
