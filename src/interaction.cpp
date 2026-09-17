#include"interaction.h"
#include"fileinfo.h"
#include"search.h"

void SearchResuluOutput(std::ostream& output, const std::vector<FileInfo>& files) {
	if (files.empty()) {
		output << "No matches found." << std::endl;
	}
	else {
		output << "Find " << files.size() << " files." << std::endl;
	}
	for (const FileInfo& file : files) {
		output << file.file_path << std::endl;
	}
}

bool Input(std::istream& input, std::string& target) {
	if (!std::getline(input,target)) {
		return 1;
	}
	return 0;
}

bool MainInput(std::istream& input, std::ostream& output, std::ostream& errorout, SearchFile_return& re) {
	output << "Search:" << std::endl;
	//keyword 输入
	output << "keyword: ";
	std::string keyword;
	if (Input(input, keyword)) {
		return 0;
	}
	//退出循环
	if (keyword == "exit") {
		return 0;
	}
	output << std::endl;

	//search_extname 输入
	output << "search_extname: ";
	std::string search_extname;
	if (Input(input, search_extname)) {
		return 0;
	}
	output << std::endl;

	//min_size 输入
	output << "min_size: ";
	std::string min_size;
	if (Input(input, min_size)) {
		return 0;
	}
	output << std::endl;

	//max_size 输入
	output << "max_size: ";
	std::string max_size;
	if (Input(input, max_size)) {
		return 0;
	}
	output << std::endl;

	//输入解析
	re = SearchRequiedInput(keyword, search_extname, min_size, max_size);
	int error_type = ErrorAnalysis(re, errorout);

	while (error_type) {
		switch (error_type) {
		case 1: {
			//keyword 输入
			output << "input keyword again: ";
			std::string keyword_again;
			if (Input(input, keyword_again)) {
				return 0;
			}
			//退出循环
			if (keyword_again == "exit") {
				return 0;
			}
			keyword = keyword_again;
			output << std::endl;
			break;
		}

		case 2: {
			//search_extname 输入
			output << "intput search_extname again: ";
			std::string search_extname_again;
			if (Input(input, search_extname_again)) {
				return 0;
			}
			search_extname = search_extname_again;
			output << std::endl;
			break;
		}

		case 3: {
			//min_size 输入
			output << "input min_size again: ";
			std::string min_size_again;
			if (Input(input, min_size_again)) {
				return 0;
			}
			min_size = min_size_again;
			output << std::endl;
			break;
		}

		case 4: {
			//max_size 输入
			output << "input max_size again: ";
			std::string max_size_again;
			if (Input(input, max_size_again)) {
				return 0;
			}
			max_size = max_size_again;
			output << std::endl;
			break;
		}
		}
		re = SearchRequiedInput(keyword, search_extname, min_size, max_size);
		error_type = ErrorAnalysis(re, errorout);
	}

	return 1;
}
bool RootInput(std::istream& input, std::ostream& output, std::string& search_directory_path) {
	output << "Scan directory:";
	if (Input(input, search_directory_path)) {
		return 0;
	}
	std::filesystem::directory_entry search_directory(search_directory_path);
	while (!search_directory.is_directory()) {
		output << "is not a directory or no exit" << std::endl;
		output << "Scan directory:";
		if (Input(input, search_directory_path)) {
			return 0;
		}
		search_directory.assign(search_directory_path);
	}
}
