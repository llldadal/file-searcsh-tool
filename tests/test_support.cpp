#include"test_support.h"

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

std::string runInteraction(const std::vector<std::string>& lines) {
    // 用字符串流代替真实的键盘和控制台，测试时就不需要手动输入。
    std::ostringstream input_text;
    for (const auto& line : lines) {
        input_text << line << '\n';
    }

    std::istringstream input(input_text.str());
    std::ostringstream output;

    // 最新的交互接口已经支持传入 input、output 和 errorout，
    // 因此直接测试 RunInteraction，不再包含 main.cpp，也不再替换 std::cin/std::cout 的缓冲区。
    // 主程序会把普通输出和错误输出都传给 std::cout，此处使用同一个 output 保持行为一致。
    RunInteraction(input, output, output);

    return output.str();
}
