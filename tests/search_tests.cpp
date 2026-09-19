#include"test_support.h"

TestRunner RT2;

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

int main() {
    runT2RegressionTests(RT2);
    std::cout << '\n' << RT2.total - RT2.failed << '/' << RT2.total << " tests passed.\n";
    return RT2.failed;
}