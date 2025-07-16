#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <string>
#include "MyApp.h"

namespace fs = std::filesystem;

class File {
    fs::directory_entry directoryEntry;
    ultralight::String name, path;

public:
    File(const fs::directory_entry& _directoryEntry,const ultralight::String& _name, const ultralight::String& _path);

    void OpenFile();
    ultralight::String GetName();
    ultralight::String GetPath();

};



#endif //FILE_H
