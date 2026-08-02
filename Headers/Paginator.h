#pragma once
#include <string>
#include <algorithm>

// Paginator — reusable pagination state machine for tabbed menus.
// Encapsulates page arithmetic for scrollable lists.
struct Paginator {
    int totalItems = 0;
    int pageSize = 20;       // items per page
    int currentPage = 0;

    int totalPages() const;       // ceil(totalItems / pageSize), minimum 1
    int startIndex() const;       // currentPage * pageSize
    int endIndex() const;         // min(startIndex + pageSize, totalItems)
    int displayCount() const;     // endIndex - startIndex
    bool canAdvance() const;      // currentPage < totalPages - 1
    bool canRetreat() const;      // currentPage > 0
    void nextPage();              // ++currentPage (clamped)
    void prevPage();              // --currentPage (clamped to 0)
    std::string indicator() const; // "Page 2/5" format string
};
