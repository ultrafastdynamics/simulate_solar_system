#include "ioparser.hpp"
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <algorithm>

std::vector<std::string> IOParser::getFilesInDirectory(std::string directory)
{
    std::vector<std::string> files;
    DIR* dir;
    if ((dir = opendir(directory.c_str())) == nullptr){
        std::cerr << "Failed to open directory: " << directory << std::endl;
        exit(1);
    }
    struct dirent* ent = readdir(dir);
    while(ent != nullptr)
    {
        if(ent->d_type == DT_REG)
        {
            files.push_back(ent->d_name);
        }
        ent = readdir(dir);
    }
    closedir(dir);
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::string> IOParser::fileAsLines(std::string filename)
{
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }
    std::string line;
    std::vector<std::string> lines;
    while(std::getline(file, line))
    {
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> IOParser::split(std::string str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    for(char c : str)
    {
        if(c == delimiter)
        {
            if(!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }
    if(!token.empty())
    {
        tokens.push_back(token);
    }
    return tokens;
}
