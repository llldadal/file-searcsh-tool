#include"test_support.h"

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


    std::cout << '\n' << runner.total - runner.failed << '/' << runner.total << " tests passed.\n";
    return runner.failed;
}