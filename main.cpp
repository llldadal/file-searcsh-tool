#include"src/fileinfo.h"
#include<vector>
#include<iostream>
using namespace std;

vector<FileInfo> FileList;

void text(FileInfo file) {
	cout << file.file_name << endl;
	cout << file.file_path << endl;
	cout << file.file_size << endl;
	cout << file.is_directory << endl;
}

int main() {
	//测试
	filesystem::directory_entry text1(L"E:/vsprojects/文件搜索项目练习/text_file_list/text1");
	filesystem::directory_entry text2(L"E:/vsprojects/文件搜索项目练习/text_file_list/text2.docx");
	filesystem::directory_entry text3(L"E:/vsprojects/文件搜索项目练习/text_file_list/text3.xlsx");

	FileInfo f1 = GetFile(text1);
	FileInfo f2 = GetFile(text2);
	FileInfo f3 = GetFile(text3);

	text(f1);
	text(f2);
	text(f3);
	
}