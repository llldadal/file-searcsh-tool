#include"src/fileinfo.h"
#include"src/search.h"
#include<vector>
#include<iostream>
using namespace std;
namespace fs = std::filesystem;
int main() {
	
	//输入解析
	string search_directory_path;
	cout << "Scan directory:";
	if (!getline(cin, search_directory_path)) {
		return 0;
	}
	fs::directory_entry search_directory(search_directory_path);
	while (!search_directory.is_directory()) {
		cout << "is not a directory or no exit" << endl;
		cout << "Scan directory:";
		if (!getline(cin, search_directory_path)) {
			return 0;
		}
		search_directory.assign(search_directory_path);
	}
	//目录扫描
	vector<FileInfo> file_list;
	file_list = ScanDirectory(search_directory_path);

	cout << "Found " << file_list.size() << " files." << endl;
	//主循环
	while (true) {
		cout << "Search:";
		string keyword;
		if (!getline(cin, keyword)) {
			return 0;
		}
		//退出循环
		if (keyword == "exit") {
			return 0;
		}
		//输入为空提示
		if (keyword == "") {
			cout << "Keyword cannot be empty." << endl;
			continue;
		}
		vector<FileInfo> search_result = SearchFiles(file_list, keyword);
		if (search_result.empty()) {
			cout << "No matches found." << endl;
		}
		else {
			cout << "Found " << search_result.size() << " files." << endl;
		}
		for (const FileInfo& file : search_result) {
			cout << file.file_path << endl;
		}
	}
}
