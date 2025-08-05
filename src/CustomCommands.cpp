#include "MyApp.h"
#include <fstream>

// Stored if format {command    path-to-file}
const std::string commandsFile = "";

struct CommandStructure
{
    std::string cmd, path;
};

void MyApp::RunCustomCommand(const std::string &input)
{
    std::ifstream in(commandsFile);
    std::string line;

    while (getline(in, line))
    {
        auto tab = line.find('\t');

        if (tab == std::string::npos)
            continue;

        std::string cmd = line.substr(0, tab);
        std::string path = line.substr(tab + 1);

        if (cmd == input)
        {

        }
    }
}