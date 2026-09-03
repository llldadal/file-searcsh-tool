#pragma once
#include<string>
#include<filesystem>
#include<vector>

//文件信息
struct FileInfo {
	std::string file_name;
	std::filesystem::path file_path;
	uintmax_t file_size;
};

