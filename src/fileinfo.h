#pragma once
#include<string>
#include<filesystem>
#include <cstdint>
#include <vector>
#include <optional>
//文件信息
struct FileInfo {
	std::string file_name;
	std::filesystem::path file_path;
	std::uintmax_t file_size;
};
//获取文件
FileInfo GetFile(std::filesystem::directory_entry entry);
//扫描目录
std::vector<FileInfo> ScanDirectory(const std::string& search_directory_path);

