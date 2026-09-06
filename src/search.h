#pragma once
#include <cstdint>
#include <optional>
#include<string>

//搜索条目信息
struct FileSearch {
	std::optional<std::string> keyword = std::nullopt;
	std::optional<std::string> search_extname = std::nullopt;
	std::optional<uintmax_t> min_size = std::nullopt;
	std::optional<uintmax_t> max_size = std::nullopt;
};

//设置搜索条目
FileSearch SetFileSearch(std::string keyword = "any", std::string search_extname = "any", std::string min_size = "any", std::string max_size = "any");