#pragma once
#include <AppCore/AppCore.h>
#include <thread>
#include <atomic>
#include <Windows.h>
#include "File.h"
#include <vector>
using namespace ultralight;

class MyApp : public AppListener,
              public WindowListener,
              public LoadListener,
              public ViewListener
{
public:
    MyApp();

    std::thread updateThread_;
    std::atomic<bool> running_ = true;
    ultralight::RefPtr<ultralight::View> mainCaller;

    HWND hwnd;

    unsigned int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 400;
    unsigned int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;

    const std::string storageFile = "C:/Dev/WindowsFileSearch/src/FileList.txt";
    std::vector<std::string> filesToGather = {"C:/Users/Shenasi/Downloads/", "C:/Users/Shenasi/Documents/",
                                              "C:/Users/Shenasi/Desktop/"};
    virtual ~MyApp();

    // Start the run loop.
    virtual void Run();

    // This is called continuously from the app's main loop.
    virtual void OnUpdate() override;

    // This is called when the window is closing.
    virtual void OnClose(ultralight::Window *window) override;

    // This is called whenever the window resizes.
    virtual void OnResize(ultralight::Window *window, uint32_t width, uint32_t height) override;

    // This is called when the page finishes a load in one of its frames.
    virtual void OnFinishLoading(ultralight::View *caller,
                                 uint64_t frame_id,
                                 bool is_main_frame,
                                 const String &url) override;

    // This is called when the DOM has loaded in one of its frames.
    virtual void OnDOMReady(ultralight::View *caller,
                            uint64_t frame_id,
                            bool is_main_frame,
                            const String &url) override;

    // This is called when the page requests to change the Cursor.
    virtual void OnChangeCursor(ultralight::View *caller,
                                Cursor cursor) override;

    virtual void ToLowerCase(std::string &str);

    virtual void OnChangeTitle(ultralight::View *caller,
                               const String &title) override;

    // Functions regarding the windows look
    virtual void RoundCorners(int width, int height, int radius);

    virtual void RemoveTaskbarIcon();

    virtual void ResizeWindow(int new_width, int new_height);

    // Functions that load the files
    virtual void InitFiles();

    virtual void LoadIndex();

    virtual void OpenFileByKeyboard(int index);

    // Functions for the interface
    virtual JSValueRef OnSearch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);

    virtual JSValueRef OnClick(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);

    virtual void GetFiles(const std::string &input);

    // Stored in the disk as {name}\t{path}
    struct FileEntry
    {
        std::string name, path;
    };


    std::vector<FileEntry> entries;
    std::unordered_map<std::string, std::vector<size_t>> prefixes; // {prefix, index of element with that prefix};



protected:
    RefPtr<App> app_;
    RefPtr<Window> window_;
    RefPtr<Overlay> overlay_;
};
