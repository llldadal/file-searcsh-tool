#include<iostream>
#include"fileinfo.h"

//文件信息提取
FileInfo GetFile(std::filesystem::directory_entry entry) {
	FileInfo file;
	file.file_path = entry.path();
	file.file_name = entry.path().filename().string();
	file.is_directory = entry.is_directory();
	//文件夹不可以读大小
	if (file.is_directory) {
		file.file_size = 0;
	}
	else {
		file.file_size = entry.file_size();
	}
	return file;
}

//字符比较
bool is_equal(FileInfo file, std::string in) {
	if (file.file_name.find(in) != std::string::npos) {
		return true;
	}
	else {
		return false;
	}
}
