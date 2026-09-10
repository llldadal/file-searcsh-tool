#include"search.h"
#include"fileinfo.h"
#include <cinttypes>
#include <iostream>
//检查是否为数字
bool isNumber(const std::string& s) {
	if (s.empty()) {
		return false;
	}
	for (char c : s) {
		if (c < '0' || c>'9') {
			return false;
		}
	}
	return true;
}
//无输入检查的解析函数，输入检查将在后续函数中加入
FileSearch SetFileSearch(std::string keyword, std::string search_extname, std::string min_size, std::string max_size) {
	FileSearch search;
	if (keyword != "any") {
		if (keyword == "\"any\"" || keyword == "\'any\'") {
			search.keyword = "any";
		}
		else {
			search.keyword = keyword;
		}
	}
	if (search_extname != "any") {
		if (search_extname == "\"any\"" || search_extname == "\'any\'") {
			search.search_extname = "any";
		}
		else {
			search.search_extname = search_extname;
		}
	}
	if (min_size != "any") {
		if (min_size == "\"any\"" || min_size == "\'any\'") {
			search.min_size = std::nullopt;
		}
		else {
			search.min_size = std::strtoumax(min_size.c_str(), nullptr, 10);
		}
	}
	if (max_size != "any") {
		if (max_size == "\"any\"" || max_size == "\'any\'") {
			search.max_size = std::nullopt;
		}
		else {
			search.max_size = std::strtoumax(max_size.c_str(), nullptr, 10);
		}
	}
	return search;
}
SearchFile_return SearchRequiedInput(std::string keyword, std::string search_extname, std::string min_size, std::string max_size) {
	SearchFile_return re;
	if ((!isNumber(min_size) && min_size != "any")) {
		re.effective = false;
		re.errror_message = "min_size input is not number";
		return re;
	}
	if ((!isNumber(max_size) && max_size != "any")) {
		re.effective = false;
		re.errror_message = "max_size input is not number";
		return re;
	}
	if (search_extname[0] != '.' && search_extname != "any") {
		re.effective = false;
		re.errror_message = "search_extname input error";
		return re;
	}
	if (min_size != "any") {
		if (std::strtoumax(min_size.c_str(), nullptr, 10) < 0) {
			min_size = "0";
		}
	}
	re.resulst = SetFileSearch(keyword, search_extname, min_size, max_size);
	re.effective = true;
	return re;
}

//搜索函数
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const std::string& keyword) {
	std::vector<FileInfo> result;
	for (const FileInfo& file : file_list) {
		if (file.file_name.find(keyword) != std::string::npos) {
			result.push_back(file);
		}
	}
	return result;
}
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const SearchFile_return& search_file) {
	std::vector<FileInfo> result;
	if (!search_file.effective) {
		std::cout << "search error" << std::endl;
		std::cout << "error message: " << search_file.errror_message << std::endl;
		return result;
	}
	else {
		std::string search_bool_base = "0000";
		if (!search_file.resulst.keyword.has_value()) {
			search_bool_base[0] = '1';
		}
		if (!search_file.resulst.search_extname.has_value()) {
			search_bool_base[1] = '1';
		}
		if (!search_file.resulst.min_size.has_value()) {
			search_bool_base[2] = '1';
		}
		if (!search_file.resulst.max_size.has_value()) {
			search_bool_base[3] = '1';
		}
		for (const FileInfo& file : file_list) {
			std::string search_bool = search_bool_base;
			if (!(search_bool[0] - '0')) {
				if (file.file_name.find(search_file.resulst.keyword.value()) != std::string::npos) {
					search_bool[0] = '1';
				}
			}
			if (!(search_bool[1] - '0')) {
				if (file.file_path.extension().string().find(search_file.resulst.search_extname.value()) != std::string::npos) {
					search_bool[1] = '1';
				}
			}
			if (!(search_bool[2] - '0')) {
				if (file.file_size >= search_file.resulst.min_size.value()) {
					search_bool[2] = '1';
				}
			}
			if (!(search_bool[3] - '0')) {
				if (file.file_size <= search_file.resulst.max_size.value()) {
					search_bool[3] = '1';
				}
			}

			if (search_bool == "1111") {
				result.push_back(file);
			}
		}
		return result;
	}
}
