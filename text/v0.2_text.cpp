#include "../src/search.h"

#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

// 比较文件的全部字段
bool SameFile(const FileInfo& a, const FileInfo& b) {
    return a.file_name == b.file_name
        && a.file_path == b.file_path
        && a.file_size == b.file_size;
}

// 同时比较文件内容和排列顺序
bool SameFiles(
    const std::vector<FileInfo>& a,
    const std::vector<FileInfo>& b
) {
    if (a.size() != b.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!SameFile(a[i], b[i])) {
            return false;
        }
    }

    return true;
}

bool SameCriteria(const FileSearch& a, const FileSearch& b) {
    return a.keyword == b.keyword
        && a.search_extname == b.search_extname
        && a.min_size == b.min_size
        && a.max_size == b.max_size;
}

int main() {
    // 路径仅作为字符串数据使用，不需要在磁盘上存在。
    const std::vector<FileInfo> fixtures = {
        {"csapp.pdf",       "/books/csapp.pdf",       100}, // A
        {"csapp_notes.txt", "/books/csapp_notes.txt", 200}, // B
        {"CSAPP.pdf",       "/books/CSAPP.pdf",       300}, // C
        {"archive.tar.gz",  "/books/archive.tar.gz",  400}, // D
        {"empty.pdf",       "/books/empty.pdf",         0}, // E
        {"README",          "/books/csapp/README",    150}, // F
        {"any.txt",         "/books/any.txt",         250}  // G
    };

    int total = 0;
    int failed = 0;

    // expected_ids 使用 A～G 指定期望结果及顺序。
    auto check = [&](
        const std::string& name,
        std::vector<FileInfo> input,
        FileSearch criteria,
        std::initializer_list<char> expected_ids
        ) {
            ++total;

            const auto input_before = input;
            const auto criteria_before = criteria;

            std::vector<FileInfo> expected;
            for (char id : expected_ids) {
                expected.push_back(fixtures.at(id - 'A'));
            }

            const auto actual = SearchFiles(input, criteria);

            const bool result_ok = SameFiles(actual, expected);
            const bool input_ok = SameFiles(input, input_before);
            const bool criteria_ok = SameCriteria(criteria, criteria_before);

            if (result_ok && input_ok && criteria_ok) {
                std::cout << "[PASS] " << name << '\n';
            }
            else {
                ++failed;
                std::cout << "[FAIL] " << name << '\n';

                if (!result_ok) {
                    std::cout << "  Wrong result contents or order.\n";
                    std::cout << "  Expected:";
                    for (const auto& file : expected) {
                        std::cout << ' ' << file.file_name;
                    }

                    std::cout << "\n  Actual:";
                    for (const auto& file : actual) {
                        std::cout << ' ' << file.file_name;
                    }
                    std::cout << '\n';
                }

                if (!input_ok) {
                    std::cout << "  Input files were modified.\n";
                }

                if (!criteria_ok) {
                    std::cout << "  Search criteria were modified.\n";
                }
            }
        };

    FileSearch criteria;

    // 1. 全部不限制
    check("All unrestricted", fixtures, criteria,
        { 'A', 'B', 'C', 'D', 'E', 'F', 'G' });

    // 2. 文件名搜索，不能匹配 F 的父目录名
    criteria.keyword = "csapp";
    check("Filename substring", fixtures, criteria, { 'A', 'B' });

    // 3. 关键字区分大小写
    criteria.keyword = "CSAPP";
    check("Keyword case sensitivity", fixtures, criteria, { 'C' });

    // 4. 扩展名筛选
    criteria = FileSearch{};
    criteria.search_extname = ".pdf";
    check("Extension filter", fixtures, criteria, { 'A', 'C', 'E' });

    // 5. 扩展名区分大小写
    criteria.search_extname = ".PDF";
    check("Extension case sensitivity", fixtures, criteria, {});

    // 6. 只取最后一个扩展名
    criteria.search_extname = ".gz";
    check("Last extension", fixtures, criteria, { 'D' });

    // 7. 下限包含边界
    criteria = FileSearch{};
    criteria.min_size = 200;
    check("Inclusive minimum", fixtures, criteria, { 'B', 'C', 'D', 'G' });

    // 8. 上限包含边界
    criteria = FileSearch{};
    criteria.max_size = 100;
    check("Inclusive maximum", fixtures, criteria, { 'A', 'E' });

    // 9. 0 是启用的条件值，不代表不限制
    criteria.min_size = 0;
    criteria.max_size = 0;
    check("Zero-byte range", fixtures, criteria, { 'E' });

    // 10. 四个条件同时生效
    criteria = FileSearch{};
    criteria.keyword = "csapp";
    criteria.search_extname = ".pdf";
    criteria.min_size = 100;
    criteria.max_size = 100;
    check("Four combined conditions", fixtures, criteria, { 'A' });

    // 11. 必须全部满足，不能使用 OR
    criteria = FileSearch{};
    criteria.keyword = "csapp";
    criteria.search_extname = ".gz";
    check("All conditions must match", fixtures, criteria, {});

    // 12. 内部条件中的 any 是普通字符串
    criteria = FileSearch{};
    criteria.keyword = "any";
    check("Literal any keyword", fixtures, criteria, { 'G' });

    // 13. 空文件列表
    criteria = FileSearch{};
    check("Empty input", {}, criteria, {});

    // 14. 回归：.pd 不能匹配 .pdf
    criteria.search_extname = ".pd";
    check("Exact extension equality", fixtures, criteria, {});

    std::cout << '\n'
        << total - failed << '/' << total << " tests passed.\n";

    return failed == 0 ? 0 : 1;
}