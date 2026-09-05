#include"fileinfo.h"
FileInfo GetFile(std::filesystem::directory_entry entry) {
	FileInfo file;
	file.file_path = entry.path();
	file.file_name = entry.path().filename().string();
	if (!entry.is_directory()) {
		file.file_size = entry.file_size();
	}
	return file;
}
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const std::string& keyword) {
	std::vector<FileInfo> result;
	for (const FileInfo& file : file_list) {
		if (file.file_name.find(keyword) != std::string::npos) {
			result.push_back(file);
		}
	}
	return result;
}
