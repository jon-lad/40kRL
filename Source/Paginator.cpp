#include "Paginator.h"

int Paginator::totalPages() const {
    if (totalItems <= 0) return 1;
    return (totalItems + pageSize - 1) / pageSize;
}

int Paginator::startIndex() const {
    return currentPage * pageSize;
}

int Paginator::endIndex() const {
    return std::min(startIndex() + pageSize, totalItems);
}

int Paginator::displayCount() const {
    return endIndex() - startIndex();
}

bool Paginator::canAdvance() const {
    return currentPage < totalPages() - 1;
}

bool Paginator::canRetreat() const {
    return currentPage > 0;
}

void Paginator::nextPage() {
    if (canAdvance()) {
        ++currentPage;
    }
}

void Paginator::prevPage() {
    if (canRetreat()) {
        --currentPage;
    }
}

std::string Paginator::indicator() const {
    return "Page " + std::to_string(currentPage + 1) + "/" + std::to_string(totalPages());
}
