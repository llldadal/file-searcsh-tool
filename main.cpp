#include"src/fileinfo.h"
#include<vector>
#include<iostream>
using namespace std;



int main() {
	//测试
	FileInfo f1, f2, f3;
	f1.file_name = "text1";
	f2.file_name = "text2";
	f3.file_name = "text3";
	f1.file_path = "/text1";
	f2.file_path = "/text2";
	f3.file_path = "/text3";
	f1.file_size = 123;
	f2.file_size = 1234;
	f3.file_size = 1235;
	vector<FileInfo> FileList;
	FileList.push_back(f1);
	FileList.push_back(f2);
	FileList.push_back(f3);
	for (FileInfo f : FileList) {
		cout << f.file_name << " " << f.file_path << " " << f.file_size << endl;
	}
}