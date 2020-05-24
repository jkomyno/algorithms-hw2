#pragma once

#include <vector>

namespace utils {
    template <typename T>
    void two_opt(const std::vector<T>& path, std::vector<T>& new_path, size_t x,
                 size_t y) noexcept {
        size_t i = 0;
        size_t j = 0;

        // copy the first x items
        for (; i < x; i++, j++) {
            new_path[j] = path[i];
        }

        // copy the reversed version of the next x items
        for (i = y - 1; i >= x; i--, j++) {
            new_path[j] = path[i];
        }

        // copy the last part
        for (i = y; i < path.size(); i++, j++) {
            new_path[j] = path[i];
        }
    }

    template <typename T>
    void translate(const std::vector<T>& path, std::vector<T>& new_path, size_t x,
                   size_t y) noexcept {
        size_t i = 0;
        size_t j = 0;

        // copy the first x items
        for (; i < x; i++, j++) {
            new_path[j] = path[i];
        }

        // copy the item at position y-1
        new_path[j++] = path[y - 1];

        // copy the next y-x-1 items
        for (i = x; i < y - 1; i++, j++) {
            new_path[j] = path[i];
        }

        // copy the last part
        for (i = y; i < path.size(); i++, j++) {
            new_path[j] = path[i];
        }
    }

    template <typename T>
    void switching(const std::vector<T>& path, std::vector<T>& new_path, size_t x, size_t y) {
        size_t i = 0;
        size_t j = 0;

        // copy the first x items
        for (; i < x; i++, j++) {
            new_path[j] = path[i];
        }

        // copy the item at position y-1
        new_path[j++] = path[y - 1];

        // skip the x-th item, copy the next y-x-2 items
        for (i = x + 1; i < y - 1; i++, j++) {
            new_path[j] = path[i];
        }

        // copy the x-th item
        new_path[j++] = path[x];

        // copy the last part
        for (i = y; i < path.size(); i++, j++) {
            new_path[j] = path[i];
        }
    }
}  // namespace utils
