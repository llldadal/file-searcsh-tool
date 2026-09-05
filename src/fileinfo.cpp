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