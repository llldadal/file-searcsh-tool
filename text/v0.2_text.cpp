#include "../src/search.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
            "pdf", ".", "", "\"\"", "\"any\"", "'.pdf'",
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

    std::cout << '\n' << runner.total - runner.failed << '/' << runner.total << " tests passed.\n";
    return runner.failed == 0 ? 0 : 1;
}
