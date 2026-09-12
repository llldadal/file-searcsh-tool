#include "../src/search.h"

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

#define main runFileSearchProgram
#include "../main.cpp"
#undef main

namespace {

enum class ErrorField { keyword, extension, minimum_size, maximum_size, unknown };
enum class ErrorReason { format, out_of_range, invalid_range, unknown };

struct TestRunner {
    int total = 0;
    int failed = 0;

    void check(const std::string& name, bool passed, const std::string& detail = {}) {
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

std::string lowerCopy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

bool containsAny(const std::string& text, const std::vector<std::string>& parts) {
    for (const auto& part : parts) {
        if (contains(text, part)) {
            return true;
        }
    }
    return false;
}

ErrorField detectField(const std::string& message) {
    const std::string text = lowerCopy(message);
    if (contains(text, "keyword")) {
        return ErrorField::keyword;
    }
    if (containsAny(text, { "search_extname", "extension", "extname" })) {
        return ErrorField::extension;
    }
    if (contains(text, "min_size") || contains(text, "minimum size")) {
        if (!contains(message, "MAX_SIZE")
            && containsAny(text, { "upper than max", "greater than max", "larger than max" })) {
            return ErrorField::maximum_size;
        }
        return ErrorField::minimum_size;
    }
    if (contains(text, "max_size") || contains(text, "maximum size")) {
        return ErrorField::maximum_size;
    }
    return ErrorField::unknown;
}

ErrorReason detectReason(const std::string& message) {
    const std::string text = lowerCopy(message);
    if (contains(message, "MAX_SIZE")
        || containsAny(text, { "out of range", "overflow", "too large", "exceed", "type maximum" })) {
        return ErrorReason::out_of_range;
    }
    if (containsAny(text, {
            "upper than max", "greater than max", "larger than max",
            "minimum is greater", "lower bound", "invalid range"
        })) {
        return ErrorReason::invalid_range;
    }
    if (containsAny(text, {
            "empty", "quote", "qoute", "format", "number", "positive",
            "input error", "invalid", "separator", "must start", "has / or \\"
        })) {
        return ErrorReason::format;
    }
    return ErrorReason::unknown;
}

std::string fieldName(ErrorField field) {
    switch (field) {
    case ErrorField::keyword: return "keyword";
    case ErrorField::extension: return "extension";
    case ErrorField::minimum_size: return "minimum size";
    case ErrorField::maximum_size: return "maximum size";
    default: return "unknown";
    }
}

std::string reasonName(ErrorReason reason) {
    switch (reason) {
    case ErrorReason::format: return "format";
    case ErrorReason::out_of_range: return "out of range";
    case ErrorReason::invalid_range: return "invalid range";
    default: return "unknown";
    }
}

void checkSuccessResult(TestRunner& runner, const std::string& name,
    const SearchFile_return& actual, const FileSearch& expected) {
    const bool passed = actual.effective
        && actual.resulst.keyword == expected.keyword
        && actual.resulst.search_extname == expected.search_extname
        && actual.resulst.min_size == expected.min_size
        && actual.resulst.max_size == expected.max_size;

    std::string detail;
    if (!actual.effective) {
        detail = "expected success, got error: " + actual.errror_message;
    }
    else if (!passed) {
        detail = "one or more parsed conditions differ from the expected value";
    }
    runner.check(name, passed, detail);
}

void checkSuccess(TestRunner& runner, const std::string& name,
    const std::string& keyword, const std::string& extension,
    const std::string& minimum, const std::string& maximum,
    const FileSearch& expected) {
    checkSuccessResult(runner, name,
        SearchRequiedInput(keyword, extension, minimum, maximum), expected);
}

void checkFailureResult(TestRunner& runner, const std::string& name,
    const SearchFile_return& actual, ErrorField expected_field,
    ErrorReason expected_reason) {
    const ErrorField actual_field = detectField(actual.errror_message);
    const ErrorReason actual_reason = detectReason(actual.errror_message);
    const bool passed = !actual.effective
        && !actual.errror_message.empty()
        && actual_field == expected_field
        && actual_reason == expected_reason;

    std::string detail;
    if (actual.effective) {
        detail = "expected failure, but parsing succeeded";
    }
    else if (actual.errror_message.empty()) {
        detail = "failure did not include an error message";
    }
    else if (!passed) {
        detail = "expected " + fieldName(expected_field) + "/" + reasonName(expected_reason)
            + ", detected " + fieldName(actual_field) + "/" + reasonName(actual_reason)
            + "; message: " + actual.errror_message;
    }
    runner.check(name, passed, detail);
}

void checkFailure(TestRunner& runner, const std::string& name,
    const std::string& keyword, const std::string& extension,
    const std::string& minimum, const std::string& maximum,
    ErrorField expected_field, ErrorReason expected_reason) {
    checkFailureResult(runner, name,
        SearchRequiedInput(keyword, extension, minimum, maximum),
        expected_field, expected_reason);
}

FileSearch keywordOnly(const std::string& value) {
    FileSearch expected;
    expected.keyword = value;
    return expected;
}

FileSearch extensionOnly(const std::string& value) {
    FileSearch expected;
    expected.search_extname = value;
    return expected;
}

FileSearch minimumOnly(std::uintmax_t value) {
    FileSearch expected;
    expected.min_size = value;
    return expected;
}

FileSearch maximumOnly(std::uintmax_t value) {
    FileSearch expected;
    expected.max_size = value;
    return expected;
}

bool sameFile(const FileInfo& left, const FileInfo& right) {
    return left.file_name == right.file_name
        && left.file_path == right.file_path
        && left.file_size == right.file_size;
}

bool sameFiles(const std::vector<FileInfo>& left, const std::vector<FileInfo>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.size(); ++index) {
        if (!sameFile(left[index], right[index])) {
            return false;
        }
    }
    return true;
}

bool sameCriteria(const FileSearch& left, const FileSearch& right) {
    return left.keyword == right.keyword
        && left.search_extname == right.search_extname
        && left.min_size == right.min_size
        && left.max_size == right.max_size;
}

std::size_t occurrenceCount(const std::string& text, const std::string& part) {
    if (part.empty()) {
        return 0;
    }

    std::size_t count = 0;
    std::size_t position = 0;
    while ((position = text.find(part, position)) != std::string::npos) {
        ++count;
        position += part.size();
    }
    return count;
}

bool appearsBefore(const std::string& text, const std::string& first,
    const std::string& second) {
    const std::size_t first_position = text.find(first);
    const std::size_t second_position = text.find(second);
    return first_position != std::string::npos
        && second_position != std::string::npos
        && first_position < second_position;
}

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

std::string runInteraction(const std::vector<std::string>& lines) {
    std::ostringstream input_text;
    for (const auto& line : lines) {
        input_text << line << '\n';
    }

    std::istringstream input(input_text.str());
    std::ostringstream output;
    std::streambuf* const original_input = std::cin.rdbuf(input.rdbuf());
    std::streambuf* const original_output = std::cout.rdbuf(output.rdbuf());
    const std::ios::iostate original_input_state = std::cin.rdstate();
    std::cin.clear();

    try {
        runFileSearchProgram();
    }
    catch (...) {
        std::cin.rdbuf(original_input);
        std::cout.rdbuf(original_output);
        std::cin.clear(original_input_state);
        throw;
    }

    std::cin.rdbuf(original_input);
    std::cout.rdbuf(original_output);
    std::cin.clear(original_input_state);
    return output.str();
}

void runT4InteractionTests(TestRunner& runner) {
    RealFileFixture directory;
    const std::string root = directory.text();

    auto checkOutput = [&](const std::string& name, const std::vector<std::string>& input,
        const auto& predicate, const std::string& expectation) {
        const std::string output = runInteraction(input);
        runner.check("T4 interaction: " + name, predicate(output),
            "expected " + expectation + "; output: " + output);
    };

    checkOutput("valid query executes once",
        { root, "csapp", ".pdf", "100", "300", "exit" },
        [](const std::string& output) {
            return occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf")
                && occurrenceCount(output, "keyword: ") == 2
                && !contains(output, "again:")
                && !contains(output, "Error:");
        },
        "one completed search followed by a clean exit");

    checkOutput("validation starts after all four initial fields",
        { root, "", "pdf", "abc", "-1", "csapp", ".pdf", "100", "300", "exit" },
        [](const std::string& output) {
            return appearsBefore(output, "max_size: ", "input keyword again: ")
                && appearsBefore(output, "input keyword again: ", "intput search_extname again: ")
                && appearsBefore(output, "intput search_extname again: ", "input min_size again: ")
                && appearsBefore(output, "input min_size again: ", "input max_size again: ")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf");
        },
        "all initial prompts, then retries in keyword/extension/minimum/maximum order");

    checkOutput("keyword error retries only keyword",
        { root, "", ".pdf", "100", "300", "csapp", "exit" },
        [](const std::string& output) {
            return contains(output, "Error:keyword")
                && occurrenceCount(output, "input keyword again: ") == 1
                && !contains(output, "intput search_extname again:")
                && !contains(output, "input min_size again:")
                && !contains(output, "input max_size again:")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf");
        },
        "an error reason and only the keyword retry prompt");

    checkOutput("extension error retries only extension",
        { root, "csapp", "pdf", "100", "300", ".pdf", "exit" },
        [](const std::string& output) {
            return contains(output, "Error:search_extname")
                && occurrenceCount(output, "intput search_extname again: ") == 1
                && !contains(output, "input keyword again:")
                && !contains(output, "input min_size again:")
                && !contains(output, "input max_size again:")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf");
        },
        "an error reason and only the extension retry prompt");

    checkOutput("minimum error retries only minimum",
        { root, "csapp", ".pdf", "abc", "300", "100", "exit" },
        [](const std::string& output) {
            return contains(output, "Error:min_size")
                && occurrenceCount(output, "input min_size again: ") == 1
                && !contains(output, "input keyword again:")
                && !contains(output, "intput search_extname again:")
                && !contains(output, "input max_size again:")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf");
        },
        "an error reason and only the minimum-size retry prompt");

    checkOutput("maximum error retries only maximum",
        { root, "csapp", ".pdf", "100", "abc", "300", "exit" },
        [](const std::string& output) {
            return contains(output, "Error:max_size")
                && occurrenceCount(output, "input max_size again: ") == 1
                && !contains(output, "input keyword again:")
                && !contains(output, "intput search_extname again:")
                && !contains(output, "input min_size again:")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "csapp.pdf");
        },
        "an error reason and only the maximum-size retry prompt");

    checkOutput("invalid range retries maximum",
        { root, "any", ".pdf", "300", "100", "300", "exit" },
        [](const std::string& output) {
            return contains(output, "Error:min_size input is upper than max_size")
                && occurrenceCount(output, "input max_size again: ") == 1
                && !contains(output, "input min_size again:")
                && occurrenceCount(output, "Find 1 files.") == 1
                && contains(output, "CSAPP_upper.pdf");
        },
        "the range reason and a maximum-size retry");

    checkOutput("real files unrestricted search",
        { root, "any", "any", "any", "any", "exit" },
        [](const std::string& output) {
            return contains(output, "Found 6 files.")
                && contains(output, "Find 6 files.")
                && contains(output, "csapp.pdf")
                && contains(output, "csapp_notes.txt")
                && contains(output, "CSAPP_upper.pdf")
                && contains(output, "archive.tar.gz")
                && contains(output, "empty.pdf")
                && contains(output, "any.txt");
        },
        "six real files to be scanned and returned");

    checkOutput("real files keyword filter",
        { root, "csapp", "any", "any", "any", "exit" },
        [](const std::string& output) {
            return contains(output, "Find 2 files.")
                && contains(output, "csapp.pdf")
                && contains(output, "csapp_notes.txt")
                && !contains(output, "CSAPP_upper.pdf");
        },
        "the two case-sensitive keyword matches");

    checkOutput("real files extension filter",
        { root, "any", ".pdf", "any", "any", "exit" },
        [](const std::string& output) {
            return contains(output, "Find 3 files.")
                && contains(output, "csapp.pdf")
                && contains(output, "CSAPP_upper.pdf")
                && contains(output, "empty.pdf")
                && !contains(output, "csapp_notes.txt")
                && !contains(output, "archive.tar.gz");
        },
        "the three real PDF files");

    checkOutput("real files inclusive size range",
        { root, "any", "any", "100", "250", "exit" },
        [](const std::string& output) {
            return contains(output, "Find 3 files.")
                && contains(output, "csapp.pdf")
                && contains(output, "csapp_notes.txt")
                && contains(output, "any.txt")
                && !contains(output, "CSAPP_upper.pdf")
                && !contains(output, "empty.pdf")
                && !contains(output, "archive.tar.gz");
        },
        "the real files whose byte sizes are between 100 and 250 inclusive");

    checkOutput("real files combined filters",
        { root, "csapp", ".pdf", "100", "100", "exit" },
        [](const std::string& output) {
            return contains(output, "Find 1 files.")
                && contains(output, "csapp.pdf")
                && !contains(output, "csapp_notes.txt")
                && !contains(output, "CSAPP_upper.pdf")
                && !contains(output, "empty.pdf");
        },
        "one real file satisfying all four conditions");

    checkOutput("real nested file search",
        { root, "archive", ".gz", "400", "400", "exit" },
        [](const std::string& output) {
            return contains(output, "Find 1 files.")
                && contains(output, "archive.tar.gz")
                && contains(output, "nested");
        },
        "the matching file from a real subdirectory");

    checkOutput("real files no match",
        { root, "missing", ".pdf", "any", "any", "exit" },
        [](const std::string& output) {
            return occurrenceCount(output, "No matches found.") == 1
                && !contains(output, "Find 1 files.");
        },
        "a completed real search with no matching file");

    checkOutput("exit at initial keyword ends immediately", { root, "exit" },
        [](const std::string& output) {
            return contains(output, "keyword: ")
                && !contains(output, "search_extname: ")
                && !contains(output, "No matches found.");
        },
        "no extension prompt and no search result");

    checkOutput("exit at keyword retry ends immediately",
        { root, "", ".pdf", "100", "300", "exit" },
        [](const std::string& output) {
            return contains(output, "input keyword again: ")
                && !contains(output, "No matches found.");
        },
        "the keyword retry prompt and no search result");

    checkOutput("EOF at directory", {},
        [](const std::string& output) {
            return occurrenceCount(output, "Scan directory:") == 1
                && !contains(output, "Found ")
                && !contains(output, "Search:");
        },
        "an immediate stop at the first read");

    const std::string regular_file = std::filesystem::absolute(__FILE__).string();
    checkOutput("EOF while retrying directory", { regular_file },
        [](const std::string& output) {
            return occurrenceCount(output, "Scan directory:") == 2
                && contains(output, "is not a directory or no exit")
                && !contains(output, "Found ");
        },
        "one directory retry and then an immediate stop");

    checkOutput("EOF at initial keyword", { root },
        [](const std::string& output) {
            return contains(output, "keyword: ")
                && !contains(output, "search_extname: ")
                && !contains(output, "No matches found.");
        },
        "a stop before reading the extension");

    checkOutput("EOF at initial extension", { root, "csapp" },
        [](const std::string& output) {
            return contains(output, "search_extname: ")
                && !contains(output, "min_size: ")
                && !contains(output, "No matches found.");
        },
        "a stop before reading the minimum size");

    checkOutput("EOF at initial minimum", { root, "csapp", ".pdf" },
        [](const std::string& output) {
            return contains(output, "min_size: ")
                && !contains(output, "max_size: ")
                && !contains(output, "No matches found.");
        },
        "a stop before reading the maximum size");

    checkOutput("EOF at initial maximum", { root, "csapp", ".pdf", "100" },
        [](const std::string& output) {
            return contains(output, "max_size: ")
                && !contains(output, "Error:")
                && !contains(output, "again:")
                && !contains(output, "No matches found.");
        },
        "a stop before validation or search");

    checkOutput("EOF at keyword retry", { root, "", ".pdf", "100", "300" },
        [](const std::string& output) {
            return contains(output, "input keyword again: ")
                && !contains(output, "No matches found.");
        },
        "a stop at the keyword retry");

    checkOutput("EOF at extension retry", { root, "csapp", "pdf", "100", "300" },
        [](const std::string& output) {
            return contains(output, "intput search_extname again: ")
                && !contains(output, "No matches found.");
        },
        "a stop at the extension retry");

    checkOutput("EOF at minimum retry", { root, "csapp", ".pdf", "abc", "300" },
        [](const std::string& output) {
            return contains(output, "input min_size again: ")
                && !contains(output, "No matches found.");
        },
        "a stop at the minimum-size retry");

    checkOutput("EOF at maximum retry", { root, "csapp", ".pdf", "100", "abc" },
        [](const std::string& output) {
            return contains(output, "input max_size again: ")
                && !contains(output, "No matches found.");
        },
        "a stop at the maximum-size retry");
}

void runT2RegressionTests(TestRunner& runner) {
    const std::vector<FileInfo> fixtures = {
        { "csapp.pdf",       "/books/csapp.pdf",       100 },
        { "csapp_notes.txt", "/books/csapp_notes.txt", 200 },
        { "CSAPP.pdf",       "/books/CSAPP.pdf",       300 },
        { "archive.tar.gz",  "/books/archive.tar.gz",  400 },
        { "empty.pdf",       "/books/empty.pdf",         0 },
        { "README",          "/books/csapp/README",    150 },
        { "any.txt",         "/books/any.txt",         250 }
    };

    auto checkSearch = [&](const std::string& name, std::vector<FileInfo> input,
        FileSearch criteria, std::initializer_list<char> expected_ids) {
        const std::vector<FileInfo> input_before = input;
        const FileSearch criteria_before = criteria;
        std::vector<FileInfo> expected;
        for (char id : expected_ids) {
            expected.push_back(fixtures.at(static_cast<std::size_t>(id - 'A')));
        }

        const std::vector<FileInfo> actual = SearchFiles(input, criteria);
        const bool passed = sameFiles(actual, expected)
            && sameFiles(input, input_before)
            && sameCriteria(criteria, criteria_before);
        runner.check("T2 regression: " + name, passed,
            passed ? "" : "result, input data, or criteria did not match the expected state");
    };

    FileSearch criteria;
    checkSearch("all unrestricted", fixtures, criteria, { 'A', 'B', 'C', 'D', 'E', 'F', 'G' });

    criteria.keyword = "csapp";
    checkSearch("filename substring", fixtures, criteria, { 'A', 'B' });

    criteria.keyword = "CSAPP";
    checkSearch("keyword case sensitivity", fixtures, criteria, { 'C' });

    criteria = FileSearch{};
    criteria.search_extname = ".pdf";
    checkSearch("extension filter", fixtures, criteria, { 'A', 'C', 'E' });

    criteria.search_extname = ".PDF";
    checkSearch("extension case sensitivity", fixtures, criteria, {});

    criteria.search_extname = ".gz";
    checkSearch("last extension", fixtures, criteria, { 'D' });

    criteria = FileSearch{};
    criteria.min_size = 200;
    checkSearch("inclusive minimum", fixtures, criteria, { 'B', 'C', 'D', 'G' });

    criteria = FileSearch{};
    criteria.max_size = 100;
    checkSearch("inclusive maximum", fixtures, criteria, { 'A', 'E' });

    criteria.min_size = 0;
    criteria.max_size = 0;
    checkSearch("zero-byte range", fixtures, criteria, { 'E' });

    criteria = FileSearch{};
    criteria.keyword = "csapp";
    criteria.search_extname = ".pdf";
    criteria.min_size = 100;
    criteria.max_size = 100;
    checkSearch("four combined conditions", fixtures, criteria, { 'A' });

    criteria = FileSearch{};
    criteria.keyword = "csapp";
    criteria.search_extname = ".gz";
    checkSearch("all conditions must match", fixtures, criteria, {});

    criteria = FileSearch{};
    criteria.keyword = "any";
    checkSearch("literal any keyword", fixtures, criteria, { 'G' });

    criteria = FileSearch{};
    checkSearch("empty input", {}, criteria, {});

    criteria.search_extname = ".pd";
    checkSearch("exact extension equality", fixtures, criteria, {});
}

}

int main() {
    TestRunner runner;

    checkSuccess(runner, "all fields unrestricted", "any", "any", "any", "any", FileSearch{});

    for (const auto& [input, expected] : std::vector<std::pair<std::string, std::string>>{
            { "csapp", "csapp" }, { "ANY", "ANY" }, { "exit", "exit" },
            { "\"csapp\"", "csapp" }, { "\"any\"", "any" }, { "\"exit\"", "exit" },
            { "'any'", "'any'" }, { "it's", "it's" },
            { " csapp ", " csapp " }, { "   ", "   " }, { "\" csapp \"", " csapp " }
        }) {
        checkSuccess(runner, "keyword: " + input, input, "any", "any", "any", keywordOnly(expected));
    }

    for (const auto& input : std::vector<std::string>{ "", "\"\"", "\"csapp", "csapp\"", "\"" }) {
        checkFailure(runner, "invalid keyword: " + input, input, "any", "any", "any",
            ErrorField::keyword, ErrorReason::format);
    }

    for (const auto& [input, expected] : std::vector<std::pair<std::string, std::string>>{
            { ".pdf", ".pdf" }, { "\".pdf\"", ".pdf" }, { ".PDF", ".PDF" }, { ".pdf ", ".pdf " }
        }) {
        checkSuccess(runner, "extension: " + input, "any", input, "any", "any", extensionOnly(expected));
    }

    for (const auto& input : std::vector<std::string>{
            "pdf", ".", "\".\"", "", "\"\"", "\"any\"", "'.pdf'",
            "\".pdf", ".pdf\"", ".pdf/notes", ".pdf\\notes", " .pdf"
        }) {
        checkFailure(runner, "invalid extension: " + input, "any", input, "any", "any",
            ErrorField::extension, ErrorReason::format);
    }

    const std::string maximum_text =
        std::to_string(std::numeric_limits<std::uintmax_t>::max());
    const std::string maximum_with_leading_zeros = "000" + maximum_text;
    const std::string overflow_text = maximum_text + "0";

    for (const auto& [input, expected] : std::vector<std::pair<std::string, std::uintmax_t>>{
            { "0", 0 }, { "000", 0 }, { "100", 100 }, { "00100", 100 },
            { maximum_text, std::numeric_limits<std::uintmax_t>::max() },
            { maximum_with_leading_zeros, std::numeric_limits<std::uintmax_t>::max() }
        }) {
        checkSuccess(runner, "minimum size: " + input, "any", "any", input, "any", minimumOnly(expected));
        checkSuccess(runner, "maximum size: " + input, "any", "any", "any", input, maximumOnly(expected));
    }

    checkSuccess(runner, "minimum size unrestricted", "any", "any", "any", "any", FileSearch{});
    checkSuccess(runner, "maximum size unrestricted", "any", "any", "any", "any", FileSearch{});

    const std::vector<std::string> invalid_sizes = {
        "", "-1", "+1", "1.5", "100KB", "123abc", " 100", "100 ", "1 00",
        "\"100\"", "\"any\"", "'100'"
    };
    for (const auto& input : invalid_sizes) {
        checkFailure(runner, "invalid minimum size: " + input, "any", "any", input, "any",
            ErrorField::minimum_size, ErrorReason::format);
        checkFailure(runner, "invalid maximum size: " + input, "any", "any", "any", input,
            ErrorField::maximum_size, ErrorReason::format);
    }

    checkFailure(runner, "minimum size overflow", "any", "any", overflow_text, "any",
        ErrorField::minimum_size, ErrorReason::out_of_range);
    checkFailure(runner, "maximum size overflow", "any", "any", "any", overflow_text,
        ErrorField::maximum_size, ErrorReason::out_of_range);

    FileSearch all_enabled;
    all_enabled.keyword = "csapp";
    all_enabled.search_extname = ".pdf";
    all_enabled.min_size = 100;
    all_enabled.max_size = 1000;
    checkSuccess(runner, "all four fields enabled", "csapp", ".pdf", "100", "1000", all_enabled);

    FileSearch equal_100;
    equal_100.min_size = 100;
    equal_100.max_size = 100;
    checkSuccess(runner, "equal size bounds: 100", "any", "any", "100", "100", equal_100);

    FileSearch equal_zero;
    equal_zero.min_size = 0;
    equal_zero.max_size = 0;
    checkSuccess(runner, "equal size bounds: 0", "any", "any", "0", "0", equal_zero);

    checkFailure(runner, "minimum greater than maximum", "any", "any", "200", "100",
        ErrorField::maximum_size, ErrorReason::invalid_range);
    checkSuccess(runner, "only minimum enabled", "any", "any", "100", "any", minimumOnly(100));
    checkSuccess(runner, "only maximum enabled", "any", "any", "any", "100", maximumOnly(100));

    checkFailure(runner, "first error is keyword", "", "pdf", "abc", "-1",
        ErrorField::keyword, ErrorReason::format);
    checkFailure(runner, "first error is extension", "csapp", "pdf", "abc", "-1",
        ErrorField::extension, ErrorReason::format);
    checkFailure(runner, "first error is minimum size", "csapp", ".pdf", "abc", "-1",
        ErrorField::minimum_size, ErrorReason::format);
    checkFailure(runner, "first error is maximum size", "csapp", ".pdf", "100", "-1",
        ErrorField::maximum_size, ErrorReason::format);
    checkFailure(runner, "format errors precede range validation", "csapp", "pdf", "200", "100",
        ErrorField::extension, ErrorReason::format);

    const SearchFile_return sequence_success_1 = SearchRequiedInput("csapp", ".pdf", "100", "1000");
    const SearchFile_return sequence_failure = SearchRequiedInput("", "any", "any", "any");
    const SearchFile_return sequence_success_2 = SearchRequiedInput("any", "any", "any", "any");
    checkSuccessResult(runner, "sequence success before failure", sequence_success_1, all_enabled);
    checkFailureResult(runner, "sequence failure", sequence_failure,
        ErrorField::keyword, ErrorReason::format);
    checkSuccessResult(runner, "sequence success after failure", sequence_success_2, FileSearch{});

    runT2RegressionTests(runner);
    runT4InteractionTests(runner);

    std::cout << '\n' << runner.total - runner.failed << '/' << runner.total << " tests passed.\n";
    return runner.failed == 0 ? 0 : 1;
}
