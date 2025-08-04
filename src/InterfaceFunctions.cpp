#include "MyApp.h"
#include "File.h"

std::vector<File> results;

ultralight::RefPtr<ultralight::View> mainCaller;

void MyApp::OpenFileByKeyboard(int index)
{
    if (index < 0 || index >= results.size())
        return;

    results[index].OpenFile();
}

JSValueRef MyApp::OnSearch(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    if (mainCaller != overlay_->view())
        mainCaller = overlay_->view();

    String res = mainCaller->EvaluateScript("document.getElementById('search').value;");
    String script = "message('" + res + "')";

    std::string input = std::string(res.utf8().data(), res.utf8().length());
    ToLowerCase(input);

    std::string filtered = input;
    filtered.erase(std::remove(filtered.begin(), filtered.end(), ' '), filtered.end());

    String getHeight = mainCaller->EvaluateScript("document.getElementById('list').getBoundingClientRect().height;");
    std::string heightStr = std::string(getHeight.utf8().data(), getHeight.utf8().length());
    int listHeight = std::stoi(heightStr);

    height = 300 + listHeight;
    if (height > 600)
        height = 600;
    ResizeWindow(WINDOW_WIDTH, height);

    if (filtered.length() > 0 && filtered.length() >= 3)
    {

        // Removing old results

        for (int i = 0; i < results.size(); i++)
        {
            auto file = results[i];
            std::string nameToString = std::string(file.GetName().utf8().data(), file.GetName().utf8().length());
            ToLowerCase(nameToString);

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
            GetFiles(input);
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

JSValueRef MyApp::OnClick(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
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
            file.OpenFile();
            break;
        }
    }

    return JSValueMakeNull(ctx);
}

void MyApp::GetFiles(const std::string &input)
{
    for (auto &index : prefixes[input])
    {
        auto entry = entries[index];
        std::string fileName = entry.name;
        std::string path = entry.path;

        String adaptedFileName = String(fileName.c_str());
        String adaptedPath = String(path.c_str());

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

            // Adding the file

            auto currentFile = File(adaptedFileName, adaptedPath);
            results.push_back(currentFile);

            String script = "create('" + adaptedFileName + "')";
            mainCaller->EvaluateScript(script);
        }
    }
}