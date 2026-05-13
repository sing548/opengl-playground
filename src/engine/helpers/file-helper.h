#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include <string>
#include <filesystem>
#include <iostream>

class FileHelper
{
public:
    static void SetBaseDir(std::string dir);
    
    static std::string GetShaderDir();
    static std::string GetAssetsDir();

private:
    static std::string baseDir_;
    static std::string assetsDir_;
    static std::string shaderDir_;
};

#endif