#include"search.h"
#include"fileinfo.h"
#include <cinttypes>
#include <iostream>
#include <limits>
const std::string MAX_SIZE = std::to_string(std::numeric_limits<std::uintmax_t>::max());;
//引号处理函数
bool RemoveQuotationMarks(std::string& s) {
	std::string re_s = "";
	int l = s.length();
	if (s == "\"") {
		return false;
	}
	if (s[0] == '\"' && s[l - 1] != '\"') {
		return false;
	}
	if (s[0] != '\"' && s[l - 1] == '\"') {
		return false;
	}
	if (s[0] == '\"' && s[l - 1] == '\"') {
		for (int i = 1; i < l - 1; i++) {
			re_s += s[i];
		}
		if (re_s != "any") {
			s = re_s;
		}
		return true;
	}
	return true;
}
//检查去除前导零后是否为数字
bool isNumber(std::string& s) {
	std::string re_s = "";
	bool leading_zero = true;
	if (s.empty()) {
		return false;
	}
	for (char c : s) {
		if (c < '0' || c>'9') {
			return false;
		}
		if (c != '0') {
			leading_zero = false;
		}
		if (!leading_zero) {
			re_s += c;
		}
	}
	if (re_s.empty()) {
		re_s = "0";
	}
	s = re_s;
	return true;
}
bool strNuberCompare(const std::string& s1, const std::string& s2) {
	int l1 = s1.length();
	int l2 = s2.length();
	if (l1 > l2) {
		return true;
	}
	else if (l1 < l2) {
		return false;
	}
	else {
		return s1 > s2;
	}
}
//无输入检查的输入函数
FileSearch SetFileSearch(std::string keyword, std::string search_extname, std::string min_size, std::string max_size) {
	FileSearch search;
	if (keyword != "any") {
		if (keyword == "\"any\"") {
			search.keyword = "any";
		}
		else {
			search.keyword = keyword;
		}
	}
	if (search_extname != "any") {
		if (search_extname == "\"any\"") {
			search.search_extname = "any";
		}
		else {
			search.search_extname = search_extname;
		}
	}
	if (min_size != "any") {
		if (min_size == "\"any\"") {
			search.min_size = std::nullopt;
		}
		else {
			search.min_size = std::strtoumax(min_size.c_str(), nullptr, 10);
		}
	}
	if (max_size != "any") {
		if (max_size == "\"any\"") {
			search.max_size = std::nullopt;
		}
		else {
			search.max_size = std::strtoumax(max_size.c_str(), nullptr, 10);
		}
	}
	return search;
}
//解析函数
SearchFile_return SearchRequiedInput(std::string keyword, std::string search_extname, std::string min_size, std::string max_size) {
	SearchFile_return re;
	//keyword
	//空输入处理
	if (keyword.empty() || keyword == "\"\"") {
		re.effective = false;
		re.errror_message = "keyword is empty";
		re.error_type = 1;
		return re;
	}
	//引号处理
	if (!RemoveQuotationMarks(keyword)) {
		re.effective = false;
		re.errror_message = "keyword input's qoutes is incomplete";
		re.error_type = 1;
		return re;
	}

	//search_extname
	if (search_extname.empty() || search_extname == "" || search_extname == "." || search_extname == "\".\"") {
		re.effective = false;
		re.errror_message = "search_extname is empty";
		re.error_type = 2;
		return re;
	}
	if (!RemoveQuotationMarks(search_extname)) {
		re.effective = false;
		re.errror_message = "search_extname input's qoutes is incomplete";
		re.error_type = 2;
		return re;
	}
	if (search_extname[0] != '.' && search_extname != "any") {
		re.effective = false;
		re.errror_message = "search_extname input error";
		re.error_type = 2;
		return re;
	}
	if ((search_extname.find('/') != std::string::npos || search_extname.find('\\') != std::string::npos) && search_extname != "any") {
		re.effective = false;
		re.errror_message = "search_extname has / or \\";
		re.error_type = 2;
		return re;
	}

	//min_size
	if (min_size.empty() || min_size == "\"\"") {
		re.effective = false;
		re.errror_message = "min_size is empty";
		re.error_type = 3;
		return re;
	}
	if ((!isNumber(min_size) && min_size != "any")) {
		re.effective = false;
		re.errror_message = "min_size input is not number or not a postive int";
		re.error_type = 3;
		return re;
	}
	if ((strNuberCompare(min_size, MAX_SIZE) && min_size != "any")) {
		re.effective = false;
		re.errror_message = "min_size input is upper than MAX_SIZE";
		re.error_type = 3;
		return re;
	}

	//max_size
	if (max_size.empty() || max_size == "\"\"") {
		re.effective = false;
		re.errror_message = "max_size is empty";
		re.error_type = 4;
		return re;
	}
	if ((!isNumber(max_size) && max_size != "any")) {
		re.effective = false;
		re.errror_message = "max_size input is not number or not a postive int";
		re.error_type = 4;
		return re;
	}
	if ((strNuberCompare(max_size, MAX_SIZE) && max_size != "any")) {
		re.effective = false;
		re.errror_message = "max_size input is upper than MAX_SIZE";
		re.error_type = 4;
		return re;
	}
	if ((strNuberCompare(min_size, max_size) && max_size != "any" && min_size != "any")) {
		re.effective = false;
		re.errror_message = "min_size input is upper than max_size";
		re.error_type = 4;
		return re;
	}

	re.resulst = SetFileSearch(keyword, search_extname, min_size, max_size);
	re.effective = true;
	return re;
}
//错误解析函数
int ErrorAnalysis(const SearchFile_return& re) {
	if (re.effective) {
		return 0;
	}
	else {
		std::cout << "Error:" << re.errror_message << std::endl;
		return re.error_type;
	}
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
std::vector<FileInfo> SearchFiles(const std::vector<FileInfo>& file_list, const FileSearch& search_file) {
	std::vector<FileInfo> result;
	std::string search_bool_base = "0000";
	if (!search_file.keyword.has_value()) {
		search_bool_base[0] = '1';
	}
	if (!search_file.search_extname.has_value()) {
		search_bool_base[1] = '1';
	}
	if (!search_file.min_size.has_value()) {
		search_bool_base[2] = '1';
	}
	if (!search_file.max_size.has_value()) {
		search_bool_base[3] = '1';
	}
	for (const FileInfo& file : file_list) {
		std::string search_bool = search_bool_base;
		if (!(search_bool[0] - '0')) {
			if (file.file_name.find(search_file.keyword.value()) != std::string::npos) {
				search_bool[0] = '1';
			}
		}
		if (!(search_bool[1] - '0')) {
			if (file.file_path.extension().string() == search_file.search_extname) {
				search_bool[1] = '1';
			}
		}
		if (!(search_bool[2] - '0')) {
			if (file.file_size >= search_file.min_size.value()) {
				search_bool[2] = '1';
			}
		}
		if (!(search_bool[3] - '0')) {
			if (file.file_size <= search_file.max_size.value()) {
				search_bool[3] = '1';
			}
		}

		if (search_bool == "1111") {
			result.push_back(file);
		}
	}
	return result;
}
