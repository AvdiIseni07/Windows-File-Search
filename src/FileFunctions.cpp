#include "MyApp.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void MyApp::ToLowerCase(std::string &str)
{
    for (char &c : str)
    {
        if (c >= 65 && c <= 90)
        {
            c += 32;
        }
    }
}

void MyApp::InitFiles()
{
    std::ofstream out(storageFile, std::ios::trunc); // trunc = if file exist delete the content

    fs::directory_options options = fs::directory_options::skip_permission_denied;
    for (auto &startingPoint : filesToGather)
    {
        try
        {
            for (auto &entry : fs::recursive_directory_iterator(startingPoint))
            {
                std::string name = entry.path().filename().u8string();
                std::string path = entry.path().u8string();

                for (auto &c : path)
                {
                    if (c == '\\')
                        c = '/';
                }

                ToLowerCase(name);
                out << name << '\t' << path << "\n";
            }
        }
        catch (std::system_error e)
        {
        }
    }
}

void MyApp::LoadIndex()
{
    std::ifstream in(storageFile);
    std::string line;

    size_t index = 0;

    while (std::getline(in, line))
    {
        auto tab = line.find('\t');

        if (tab == std::string::npos)
            continue;

        FileEntry e{line.substr(0, tab), line.substr(tab + 1)};
        entries.push_back(e);

        auto name = e.name;

        // Getting the prefixes, TODO: Get all possible prefixes of a fixed size
        int prefixSize = 3;

        for (int i = 0; i < name.length(); i++)
        {
            prefixes[name.substr(0, i)].push_back(index);
            //prefixes[name.substr(i, prefixSize)].push_back(index);
        }

        index++;
    }
}
