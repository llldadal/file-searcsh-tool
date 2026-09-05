#include"src/fileinfo.h"
#include<vector>
#include<iostream>
using namespace std;
namespace fs = std::filesystem;

int main() {

	vector<FileInfo> file_list;

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
	
	//扫描目录
	std::error_code ec;
	//待访问目录栈
	std::vector<fs::path> directories;

	directories.push_back(search_directory_path);
	while (!directories.empty()) {

		fs::directory_iterator end;

		fs::path current_directory =directories.back();
		directories.pop_back();
		fs::directory_iterator it(current_directory, ec);
		if (ec) {
			cout << "warning: " << current_directory << " is not open" << endl;
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
					cout << "error " << ec.message() << " in " << current_directory << endl;
					break;
				}
			}
		}
	}
	
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
