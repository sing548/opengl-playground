#include "file-helper.h"

std::string FileHelper::baseDir = "";

void FileHelper::SetBaseDir(std::string dir)
{
	baseDir = dir;
}

std::string FileHelper::GetShaderDir() 
{
    try {
        return (std::filesystem::canonical(baseDir).parent_path() / "shaders").string();
    } catch (...) {
        return "./shaders";  // Fallback to current directory
    }
}

std::string FileHelper::GetAssetsDir()
{
    try {
        return (std::filesystem::canonical(baseDir).parent_path() / "assets").string();
    } catch (...) {
        return "./assets";  // Fallback to current directory
    }
}