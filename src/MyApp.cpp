#include "MyApp.h"
#include <iostream>
#include "JavaScriptCore/JavaScript.h"
#include <Ultralight/Ultralight.h>
#include <Ultralight/ConsoleMessage.h>
#include "platform/EmbeddedFileSystem.h"
#include "AppCore/Window.h"
#include <Windows.h>

#include <string>
#include <inttypes.h>
#include "File.h"
#include <fstream>

#include <WinUser.h>
#include <chrono>
#include <thread>

#include <UIAutomation.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

using namespace ultralight;

bool open = true;

std::vector<std::pair<std::string, std::string>> allFiles; //{{name, path}}

// Stored in the disk as {name}\t{path}
struct FileEntry
{
    std::string name, path;
};

std::vector<FileEntry> entries;
std::unordered_map<std::string, std::vector<size_t>> prefixes; // {prefix, index of element with that prefix};

MyApp *_app;

JSValueRef OnSearchWrapper(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    return _app->OnSearch(ctx, function, thisObject, argumentCount, arguments, exception);
}

JSValueRef OnClickWrapper(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    return _app->OnClick(ctx, function, thisObject, argumentCount, arguments, exception);
}

MyApp::MyApp()
{
    ///
    /// Create our main App instance.
    ///
    app_ = App::Create();
    // auto temp = this;
    _app = this;
    ///
    /// Create a resizable window by passing by OR'ing our window flags with
    /// kWindowFlags_Resizable.
    ///
    window_ = Window::Create(app_->main_monitor(), WINDOW_WIDTH, WINDOW_HEIGHT,
                             false, kWindowFlags_Borderless);
    window_->MoveToCenter();

    ///
    /// Create our HTML overlay-- we don't care about its initial size and
    /// position because it'll be calculated when we call OnResize() below.
    ///
    overlay_ = Overlay::Create(window_, 1, 1, 0, 0);

    hwnd = reinterpret_cast<HWND>(window_->native_handle());

    ///
    /// Force a call to OnResize to perform size/layout of our overlay.
    ///
    OnResize(window_.get(), window_->width(), window_->height());

    ///
    /// Load a   into our overlay's View
    ///
    overlay_->view()->LoadURL("file:///app.html");
    // overlay_->view()->is_transparent = true;

    ///
    /// Register our MyApp instance as an AppListener so we can handle the
    /// App's OnUpdate event below.
    ///

    app_->set_listener(this);

    ///
    /// Show/Hide the window.
    /// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerhotkey
    ///

    updateThread_ = std::thread([this]()
                                {
                                    MSG msg{0};
                                    RegisterHotKey(NULL, 0, MOD_ALT | MOD_NOREPEAT, 0x42);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x61);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x62);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x63);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x64);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x65);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x66);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x67);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x68);
                                    RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x69);

                                    
                                    while(GetMessage(&msg, NULL, 0, 0))
                                         if (msg.message == WM_HOTKEY)
                                         {
                                            if (msg.wParam == 0)
                                            {
                                                if (open)
                                                    ResizeWindow(0, 0);
                                                else
                                                {
                                                    ResizeWindow(WINDOW_WIDTH, height);
                                                  
                                                }
                                                open = !open;

                                            } 
                                            else if (msg.wParam == 1)
                                                {
                                                    OpenFileByKeyboard(msg.wParam - 1);
                                                }
                                            
                                         } });

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
    ResizeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
}

MyApp::~MyApp()
{
    running_ = false;

    if (updateThread_.joinable())
        updateThread_.join();
}

void MyApp::Run()
{
    app_->Run();
}

void MyApp::OnUpdate()
{
    ///
    /// This is called repeatedly from the application's update loop.
    ///
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

void MyApp::OnDOMReady(ultralight::View *caller,
                       uint64_t frame_id,
                       bool is_main_frame,
                       const String &url)
{

    RemoveTaskbarIcon();
    InitFiles();
    LoadIndex();

    ///
    /// This is called when a frame's DOM has finished loading on the page.
    ///
    /// This is the best time to setup any JavaScript bindings.
    ///

    if (!is_main_frame)
        return;

    // Acquire the JS execution context for the current page.
    auto scoped_context = caller->LockJSContext();

    // Typecast to the underlying JSContextRef.
    JSContextRef ctx = (*scoped_context);

    // Create a JavaScript String containing the name of our callback.
    JSStringRef onSearchName = JSStringCreateWithUTF8CString("OnSearch");

    // Create a garbage-collected JavaScript function that is bound to our
    // native C callback 'OnSearch()'.

    JSObjectRef OnSearchFunc = JSObjectMakeFunctionWithCallback(ctx, onSearchName,
                                                                OnSearchWrapper);

    // Get the global JavaScript object (aka 'window')
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx);

    // Store our function in the page's global JavaScript object so that it
    // accessible from the page as 'OnButtonClick()'.
    JSObjectSetProperty(ctx, globalObj, onSearchName, OnSearchFunc, 0, 0);

    // Release the JavaScript String we created earlier.
    JSStringRelease(onSearchName);

    JSStringRef onClickName = JSStringCreateWithUTF8CString("OnClick");
    JSObjectRef onClickFunc = JSObjectMakeFunctionWithCallback(ctx, onClickName, OnClickWrapper);
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
