#pragma once

#include <string>
#include <unordered_map>

class FeePolicy {
private:
    std::unordered_map<std::string, long long> feeTable;

    static std::string makeKey(const std::string& customerType,
                               const std::string& serviceArea);

public:
    // Khởi tạo: đọc bảng giá từ file, ném std::runtime_error nếu có lỗi
    explicit FeePolicy(const std::string& feeFilePath);

    // Trả về mức phí (VND) cho cặp (customerType, serviceArea)
    // Ném std::runtime_error nếu không tồn tại
    long long getFee(const std::string& customerType,
                     const std::string& serviceArea) const;
};
