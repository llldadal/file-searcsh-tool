#pragma once
#include<string>
#include<filesystem>
#include <cstdint>
#include <vector>
//文件信息
struct FileInfo {
	std::string file_name;
	std::filesystem::path file_path;
	std::uintmax_t file_size;
};
//获取文件
FileInfo GetFile(std::filesystem::directory_entry entry);
//搜索文件
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& files,const std::string& keyword);
