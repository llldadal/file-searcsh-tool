#pragma once
#include"fileinfo.h"
#include"search.h"
#include<iostream>
//输出函数
void SearchResuluOutput(std::ostream& output, const std::vector<FileInfo>& files);
//输入函数,EOF返回0
bool Input(std::istream& input, std::string& target);
//主输入过程,退出返回0
bool MainInput(std::istream& input, std::ostream& output, std::ostream& errorout, SearchFile_return& re);
//搜索根目录输入过程
bool RootInput(std::istream& input, std::ostream& output, std::string& search_directory_path);
//主循环
int RunInteraction(std::istream& input, std::ostream& output, std::ostream& errorout);
//错误解析函数
int ErrorAnalysis(const SearchFile_return& re, std::ostream& output);