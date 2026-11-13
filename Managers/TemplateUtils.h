
#pragma once

// Function Template tim phan tu thoa man dieu kien
// Dung duoc cho mang doi tuong va mang con tro

template <typename T, typename Predicate>
const T* findIf(const T* arr, int size, Predicate condition) {
    if (size <= 0 || arr == nullptr) {
        return nullptr;
    }

    for (int i = 0; i < size; i++) {
        if (condition(arr[i])) {
            return &arr[i];
        }
    }

    return nullptr;
}
