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
		cout << "is ont a directory or no exit" << endl;
		cout << "Scan directory:";
		getline(cin, serach_directory_path);
		serach_directory.assign(serach_directory_path);
	}
	
	//扫描目录
	std::error_code ec;
	fs::recursive_directory_iterator it(serach_directory_path,ec);
	fs::recursive_directory_iterator end;
	int file_num = 0;
	while (ec) {
		cout << "warring: " << serach_directory_path << " is not open" << endl;
		ec.clear();
		cout << "Scan directory:";
		getline(cin, serach_directory_path);
		serach_directory.assign(serach_directory_path);
		fs::recursive_directory_iterator it(serach_directory_path, ec);
	}
	while (it != end) {
		//遇上无法创建迭代器的目录（无法打开）即跳过
		fs::directory_iterator i(it->path(),ec);
		if (ec && it->is_directory()) {
			it.disable_recursion_pending();
			cout << "warring: " << it->path() << " is not open" << endl;
			ec.clear();
		}
		else {
			if (!(it->is_directory())) {
				file_list.push_back(GetFile(*it));
				file_num++;
			}
		}
		++it;
	}
	
	cout << "Found " << file_num << " Files" << endl;

	for (FileInfo file : file_list) {
		cout << file.file_name << ' ' << file.file_path << ' ' << file.file_size << endl;
	}
}