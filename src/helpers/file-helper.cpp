#include "file-helper.h"

std::string FileHelper::baseDir = "";

void FileHelper::SetBaseDir(std::string dir)
{
	baseDir = dir;
}

std::string FileHelper::GetShaderDir()
{
    namespace fs = std::filesystem;

    try {
        fs::path exeDir = fs::weakly_canonical(baseDir).parent_path();
        fs::path shaderDir = exeDir / "shaders";
        return shaderDir.lexically_normal().string();
    }
    catch (const fs::filesystem_error&) {
        return (fs::current_path() / "shaders").string();
    }
}

std::string FileHelper::GetAssetsDir()
{
    namespace fs = std::filesystem;

    try {
        fs::path exeDir = fs::weakly_canonical(baseDir).parent_path();
        fs::path assetsDir = exeDir / "assets";
        return assetsDir.lexically_normal().string();
    }
    catch (const fs::filesystem_error&) {
        std::cout << "Catch output" << std::endl;
        return (fs::current_path() / "assets").string();
    }
}