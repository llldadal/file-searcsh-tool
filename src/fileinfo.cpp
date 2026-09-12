#include"fileinfo.h"
#include<iostream>
namespace fs = std::filesystem;
FileInfo GetFile(std::filesystem::directory_entry entry) {
	FileInfo file;
	file.file_path = entry.path();
	file.file_name = entry.path().filename().string();
	if (!entry.is_directory()) {
		file.file_size = entry.file_size();
	}
	return file;
}
std::vector<FileInfo> ScanDirectory(const std::string& search_directory_path) {
	std::vector<FileInfo> file_list;
	//扫描目录
	std::error_code ec;
	//待访问目录栈
	std::vector<fs::path> directories;
	directories.push_back(search_directory_path);
	while (!directories.empty()) {
		fs::directory_iterator end;
		fs::path current_directory = directories.back();
		directories.pop_back();
		fs::directory_iterator it(current_directory, ec);
		if (ec) {
			std::cout << "warning: " << current_directory << " is not open" << std::endl;
			continue;
		}
		else {
			while (it != end) {
				fs::directory_entry entry = *it;
				if (entry.is_symlink()) {
					//跳过链接
				}
				else if (entry.is_regular_file()) {
					file_list.push_back(GetFile(entry));
				}
				else if (entry.is_directory()) {
					directories.push_back(entry.path());
				}
				it.increment(ec);
				if (ec) {
					std::cout << "error " << ec.message() << " in " << current_directory << std::endl;
					break;
				}
			}
		}
	}
	return file_list;
}
