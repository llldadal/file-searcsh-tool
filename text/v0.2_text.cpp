#include "../src/search.h"
#include"../src/fileinfo.h"
#include <cassert>
#include <iostream>

int main() {
    
    std::vector<FileInfo> file_list;
    file_list = ScanDirectory("text/text_dir");

    SearchFile_return re1 = SearchRequiedInput("any", "any", "any", "any");//不限制测试
    SearchFile_return re2 = SearchRequiedInput("csapp", "any", "any", "any");//单限制测试
    SearchFile_return re3 = SearchRequiedInput("csapp", ".docx", "0", "100");//多限制测试
    SearchFile_return re4 = SearchRequiedInput("any", "ppt", "any", "any");//输入不合法测试
    SearchFile_return re5 = SearchRequiedInput("any", "any", "abc", "any");//输入不合法测试
    SearchFile_return re6 = SearchRequiedInput("any", "any", "any", "abc");//输入不合法测试

    std::vector<SearchFile_return> file_reach_list = { re1,re2,re3,re4,re5,re6 };
    for (int i = 0; i < 6; i++) {
        std::cout << "-----------text" << i << "-----------" << std::endl;
        std::vector<FileInfo> search_result = SearchFiles(file_list, file_reach_list[i]);
        if (search_result.empty()) {
            std::cout << "No matches found." << std::endl;
        }
        else {
            std::cout << "Found " << search_result.size() << " files." << std::endl;
        }
        for (const FileInfo& file : search_result) {
            std::cout << file.file_path << std::endl;
        }
    }
    
}