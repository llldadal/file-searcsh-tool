#include"src/fileinfo.h"
#include<vector>
#include<iostream>
using namespace std;
namespace fs = std::filesystem;

int main() {

	vector<FileInfo> file_list;

	string serach_directory_path;
	cout << "Scan directory:";
	getline(cin, serach_directory_path);
	fs::directory_entry serach_directory(serach_directory_path);
	while (!serach_directory.is_directory()) {
		cout << "is ont a directory" << endl;
		cout << "Scan directory:";
		getline(cin, serach_directory_path);
		serach_directory.assign(serach_directory_path);
	}
	
	//扫描目录
	int file_num = 0;
	for (const auto& entry : fs::recursive_directory_iterator(serach_directory_path, fs::directory_options::skip_permission_denied)) {
		if (!entry.is_directory()) {
			file_list.push_back(GetFile(entry));
			file_num++;
		}
	}
	
	cout << "Found " << file_num << " Files" << endl;

	for (FileInfo file : file_list) {
		cout << file.file_name << ' ' << file.file_path << ' ' << file.file_size << endl;
	}
}