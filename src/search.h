#pragma once
#include"fileinfo.h"
#include <cstdint>
#include <optional>
#include<string>
#include <vector>

//搜索条目信息
struct FileSearch {
	std::optional<std::string> keyword = std::nullopt;
	std::optional<std::string> search_extname = std::nullopt;
	std::optional<uintmax_t> min_size = std::nullopt;
	std::optional<uintmax_t> max_size = std::nullopt;
};

//解析结果结构体
struct SearchFile_return {
	FileSearch resulst;
	bool effective = false;
	std::string errror_message;
	int error_type = 0;//表示第error_type个字段输入错误
};

//设置搜索条目
FileSearch SetFileSearch(std::string keyword = "any", std::string search_extname = "any", std::string min_size = "any", std::string max_size = "any");
//解析用户输入
SearchFile_return SearchRequiedInput(std::string keyword = "any", std::string search_extname = "any", std::string min_size = "any", std::string max_size = "any");
bool isNumber(std::string& s);
bool RemoveQuotationMarks(std::string& s);
bool strNuberCompare(const std::string& s1, const std::string& s2);
//错误解析函数
int ErrorAnalysis(const SearchFile_return& re);
//搜索函数
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const std::string& keyword);
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const FileSearch& search_file);