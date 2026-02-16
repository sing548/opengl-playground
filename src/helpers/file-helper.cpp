#include "file-helper.h"

std::string FileHelper::baseDir_ = "";
std::string FileHelper::shaderDir_ = "";
std::string FileHelper::assetsDir_ = "";

void FileHelper::SetBaseDir(std::string dir)
{
	baseDir_ = std::filesystem::weakly_canonical(dir).parent_path();
    shaderDir_ = (std::filesystem::path(baseDir_) / "shaders").lexically_normal().string();
    assetsDir_ = (std::filesystem::path(baseDir_) / "assets").lexically_normal().string();
}

std::string FileHelper::GetShaderDir()
{
    return shaderDir_;
}

std::string FileHelper::GetAssetsDir()
{
    return assetsDir_;
}