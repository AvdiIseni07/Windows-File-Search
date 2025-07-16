

#include "File.h"
#include <Windows.h>
#include <filesystem>
#include <locale>
#include <codecvt>

File::File(const fs::directory_entry &_directoryEntry, const ultralight::String &_name, const ultralight::String &_path)
{
    this->directoryEntry = _directoryEntry;
    this->name = _name;
    this->path = _path;
}

//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
void File::OpenFile()
{
    std::string cmd = std::string(this->path.utf8().data(), this->path.utf8().length());
    //std::wstring pth;

    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    // use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::wstring pth = converter.from_bytes(cmd);

    ShellExecuteW(nullptr, L"open", pth.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

    // system(cmd.c_str());
}

ultralight::String File::GetName()
{
    return this->name;
}

ultralight::String File::GetPath()
{
    return this->path;
}
