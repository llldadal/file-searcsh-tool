#include"test_support.h"

TestRunner RT4;

void runT4InteractionTests(TestRunner& runner) {
    RealFileFixture directory;
    const std::string root = directory.text();

    auto checkOutput = [&](const std::string& name, const std::vector<std::string>& input,
        const auto& predicate, const std::string& expectation) {
            const std::string output = runInteraction(input);

            // 写一个断言时，依次填入：测试名、布尔判断、失败说明。
            // 例：runner.check("output contains prompt",
            //                    contains(output, "keyword: "),
            //                    "expected the keyword prompt; output: " + output);
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

int main() {
    runT4InteractionTests(RT4);
    std::cout << '\n' << RT4.total - RT4.failed << '/' << RT4.total << " tests passed.\n";
    return RT4.failed;
}