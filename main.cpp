#include"src/fileinfo.h"
#include"src/search.h"
#include"src/interaction.h"
#include<vector>
#include<iostream>
using namespace std;
namespace fs = std::filesystem;
int main() {
	
	//输入解析
	string search_directory_path;
	if (!RootInput(cin, cout, search_directory_path)) {
		return 0;
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
		SearchFile_return re;
		if (!MainInput(cin, cout, cout, re)) {
			return 0;
		}
		vector<FileInfo> files = SearchFiles(file_list, re.resulst);
		SearchResuluOutput(cout, files);
	}
}
