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

	cout << endl;
	cout << "Enter any to leave a condition unrestricted.\nSizes are in bytes.\nEnter exit at the keyword prompt to quit.\nUse double quotes for literal text, e.g. \"any\" or \"exit\"." << endl;
	cout << endl;

	//主循环
	while (true) {
		cout << "Search:" << endl;
		//keyword 输入
		cout << "keyword: ";
		string keyword;
		if (!getline(cin, keyword)) {
			return 0;
		}
		//退出循环
		if (keyword == "exit") {
			return 0;
		}
		cout << endl;

		//search_extname 输入
		cout << "search_extname: ";
		string search_extname;
		if (!getline(cin, search_extname)) {
			return 0;
		}
		cout << endl;

		//min_size 输入
		cout << "min_size: ";
		string min_size;
		if (!getline(cin, min_size)) {
			return 0;
		}
		cout << endl;

		//max_size 输入
		cout << "max_size: ";
		string max_size;
		if (!getline(cin, max_size)) {
			return 0;
		}
		cout << endl;

		//输入解析
		SearchFile_return  re = SearchRequiedInput(keyword, search_extname, min_size, max_size);
		int error_type = ErrorAnalysis(re);
		while(error_type) {
			switch (error_type) {
			case 1: {
				//keyword 输入
				cout << "input keyword again: ";
				string keyword_again;
				if (!getline(cin, keyword_again)) {
					return 0;
				}
				//退出循环
				if (keyword_again == "exit") {
					return 0;
				}
				keyword = keyword_again;
				cout << endl;
				break;
			}
				
			case 2: {
				//search_extname 输入
				cout << "intput search_extname again: ";
				string search_extname_again;
				if (!getline(cin, search_extname_again)) {
					return 0;
				}
				search_extname = search_extname_again;
				cout << endl;
				break;
			}

			case 3: {
				//min_size 输入
				cout << "input min_size again: ";
				string min_size_again;
				if (!getline(cin, min_size_again)) {
					return 0;
				}
				min_size = min_size_again;
				cout << endl;
				break;
			}

			case 4: {
				//max_size 输入
				cout << "input max_size again: ";
				string max_size_again;
				if (!getline(cin, max_size_again)) {
					return 0;
				}
				max_size = max_size_again;
				cout << endl;
				break;
			}
			}
			re = SearchRequiedInput(keyword, search_extname, min_size, max_size);
			error_type = ErrorAnalysis(re);
		}

		vector<FileInfo> files = SearchFiles(file_list, re.resulst);
		if (files.empty()) {
			cout << "No matches found." << endl;
		}
		else {
			cout << "Find " << files.size() << " files." << endl;
		}
		for (const FileInfo& file : files) {
			cout <<file.file_path<< endl;
		}
	}
}
