#include "../src/search.h"
#include "../src/interaction.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

enum class ErrorField { keyword, extension, minimum_size, maximum_size, unknown };
enum class ErrorReason { format, out_of_range, invalid_range, unknown };

struct TestRunner {
    int total = 0;
    int failed = 0;

    void check(const std::string& name, bool passed, const std::string& detail = {}) {
        // 单个断言的基本格式：
        // runner.check("想验证的行为", 实际值 == 期望值, "失败时的说明");
        // passed 只放一个明确的判断，这样失败时才能立刻知道哪个行为不对。
        ++total;
        if (passed) {
            std::cout << "[PASS] " << name << '\n';
            return;
        }
        ++failed;
        std::cout << "[FAIL] " << name << '\n';
        if (!detail.empty()) {
            std::cout << "       " << detail << '\n';
        }
    }
};

std::string lowerCopy(std::string text);
bool contains(const std::string& text, const std::string& part);
bool containsAny(const std::string& text, const std::vector<std::string>& parts);
ErrorField detectField(const std::string& message);
ErrorReason detectReason(const std::string& message);
std::string fieldName(ErrorField field);
std::string reasonName(ErrorReason reason);
void checkSuccessResult(TestRunner& runner, const std::string& name,
    const SearchFile_return& actual, const FileSearch& expected);
void checkSuccess(TestRunner& runner, const std::string& name,
    const std::string& keyword, const std::string& extension,
    const std::string& minimum, const std::string& maximum,
    const FileSearch& expected);
void checkFailureResult(TestRunner& runner, const std::string& name,
    const SearchFile_return& actual, ErrorField expected_field,
    ErrorReason expected_reason);
void checkFailure(TestRunner& runner, const std::string& name,
    const std::string& keyword, const std::string& extension,
    const std::string& minimum, const std::string& maximum,
    ErrorField expected_field, ErrorReason expected_reason);
FileSearch keywordOnly(const std::string& value);
FileSearch extensionOnly(const std::string& value);
FileSearch minimumOnly(std::uintmax_t value);
FileSearch maximumOnly(std::uintmax_t value);
bool sameFile(const FileInfo& left, const FileInfo& right);
bool sameFiles(const std::vector<FileInfo>& left, const std::vector<FileInfo>& right);
bool sameCriteria(const FileSearch& left, const FileSearch& right);
std::size_t occurrenceCount(const std::string& text, const std::string& part);
bool appearsBefore(const std::string& text, const std::string& first,
    const std::string& second);

class RealFileFixture {
public:
    RealFileFixture() {
        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path()
            / ("file_search_t4_test_" + std::to_string(suffix));
        std::filesystem::create_directory(path_);
        std::filesystem::create_directory(path_ / "nested");

        createFile("csapp.pdf", 100, 'A');
        createFile("csapp_notes.txt", 200, 'B');
        createFile("CSAPP_upper.pdf", 300, 'C');
        createFile(std::filesystem::path("nested") / "archive.tar.gz", 400, 'D');
        createFile(std::filesystem::path("nested") / "empty.pdf", 0, 'E');
        createFile("any.txt", 250, 'F');
    }

    ~RealFileFixture() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    std::string text() const {
        return path_.string();
    }

private:
    void createFile(const std::filesystem::path& relative_path,
        std::size_t size, char contents) {
        std::ofstream file(path_ / relative_path, std::ios::binary);
        file << std::string(size, contents);
    }

    std::filesystem::path path_;
};

std::string runInteraction(const std::vector<std::string>& lines);
