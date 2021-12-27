#pragma once

#include <utility>

namespace utils {

class ReferenceCounter {
    int* _ref_count = nullptr;
public:
    explicit ReferenceCounter(int& ref_count) : _ref_count(&ref_count) {
        if (_ref_count) (*_ref_count)++;
    }
    ~ReferenceCounter() {
        if (_ref_count) (*_ref_count)--;
    }
    ReferenceCounter& operator=(const ReferenceCounter& other) {
        if (this != &other) {
            if (_ref_count) (*_ref_count)--;
            _ref_count = other._ref_count;
            if (_ref_count) (*_ref_count)++;
        }
        return *this;
    }
    ReferenceCounter& operator=(ReferenceCounter&& other) {
        if (this != &other) {
            if (_ref_count) (*_ref_count)--;
            _ref_count = std::exchange(other._ref_count, nullptr);
        }
        return *this;
    }
    ReferenceCounter(const ReferenceCounter& other) { *this = other; }
    ReferenceCounter(ReferenceCounter&& other) { *this = std::move(other); }
};

} // namespace utils
