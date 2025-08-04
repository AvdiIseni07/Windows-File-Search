#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <string>
#include "MyApp.h"

namespace fs = std::filesystem;

class File {

    ultralight::String name, path;

public:
    File(const ultralight::String& _name, const ultralight::String& _path);

    void OpenFile();
    ultralight::String GetName();
    ultralight::String GetPath();

};


#endif //FILE_H