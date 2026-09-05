#include"src/fileinfo.h"
#include<vector>
#include<iostream>
using namespace std;
namespace fs = std::filesystem;

int main() {

	vector<FileInfo> file_list;

	string serach_directory_path;
	cout << "Scan directory:";
	if (!getline(cin, serach_directory_path)) {
		return 0;
	}
	fs::directory_entry serach_directory(serach_directory_path);

	while (!serach_directory.is_directory()) {
		cout << "is not a directory or no exit" << endl;
		cout << "Scan directory:";
		if (!getline(cin, serach_directory_path)) {
			return 0;
		}
		serach_directory.assign(serach_directory_path);
	}
	
	//扫描目录
	std::error_code ec;
	//待访问目录栈
	std::vector<fs::path> directories;

	directories.push_back(serach_directory_path);
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

	cout << "Search:";
	string keyword;
	if (!getline(cin, keyword)) {
		return 0;
	}
	vector<FileInfo> serach_result = SearchFiles(file_list, keyword);
	if (serach_result.empty()) {
		cout << "No matches found." << endl;
	}
	else {
		cout << "Find " << serach_result.size() << " files." << endl;
	}
	for (const FileInfo& file : serach_result) {
		cout <<file.file_path << endl;
	}
}