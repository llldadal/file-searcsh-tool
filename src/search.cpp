#include"search.h"
#include <cinttypes>
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
			search.min_size = std::strtoumax(min_size.c_str(), nullptr, 10);;
		}
	}
	if (max_size != "any") {
		if (max_size == "\"any\"" || max_size == "\'any\'") {
			search.max_size = std::nullopt;
		}
		else {
			search.max_size = std::strtoumax(max_size.c_str(), nullptr, 10);;
		}
	}
	return search;
}
