#pragma once
#include<string>
#include<filesystem>
#include<vector>

//文件信息
struct FileInfo {
	std::string file_name;
	std::filesystem::path file_path;
	uintmax_t file_size;
	//是否为文件夹
	bool is_directory;
};

//文件信息提取
FileInfo GetFile(std::filesystem::directory_entry entry);

//字符比较
bool is_equal(FileInfo file, std::string in);
