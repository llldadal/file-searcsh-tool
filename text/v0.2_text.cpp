#include "../src/search.h"
#include <cassert>
#include <iostream>

int main() {
    FileSearch criteria;

    // 默认情况下，所有条件都未启用
    assert(!criteria.keyword.has_value());
    assert(!criteria.search_extname.has_value());
    assert(!criteria.min_size.has_value());
    assert(!criteria.max_size.has_value());

    // 0 是有效的大小条件
    criteria.min_size = 0;
    assert(criteria.min_size.has_value());
    assert(*criteria.min_size == 0);

    // any 可以作为普通关键字保存
    criteria.keyword = "any";
    assert(*criteria.keyword == "any");

    // 重置一个条件，不影响其他条件
    criteria.min_size.reset();
    assert(!criteria.min_size.has_value());
    assert(criteria.keyword.has_value());
    assert(*criteria.keyword == "any");

    std::cout << "All tests passed.\n";
}