#include "FeePolicy.h"
#include "../FileIO/FileManager.h"
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <unordered_set>

// Helper: tạo key "type|area"
std::string FeePolicy::makeKey(const std::string& customerType,
                               const std::string& serviceArea)
{
    return customerType + "|" + serviceArea;
}

// Constructor: đọc file bảng giá, kiểm tra lỗi
FeePolicy::FeePolicy(const std::string& feeFilePath)
{
    // Kiểm tra file tồn tại
    if (!FileManager::getInstance().fileExists(feeFilePath)) {
        throw std::runtime_error("FeePolicy: fee file not found: " + feeFilePath);
    }

    // Đọc toàn bộ dòng
    std::vector<std::string> lines = FileManager::getInstance().readFile(feeFilePath);
    if (lines.empty()) {
        throw std::runtime_error("FeePolicy: fee file is empty: " + feeFilePath);
    }

    for (size_t lineNo = 0; lineNo < lines.size(); ++lineNo) {
        const std::string& raw = lines[lineNo];
        if (raw.empty()) continue; // bỏ dòng trống
        // Tách bằng '|'
        std::vector<std::string> parts;
        std::stringstream ss(raw);
        std::string item;
        while (std::getline(ss, item, '|')) {
            parts.push_back(item);
        }
        if (parts.size() != 3) {
            throw std::runtime_error("FeePolicy: invalid format at line " + std::to_string(lineNo + 1) + " – expected 3 fields");
        }
        const std::string& type = parts[0];
        const std::string& area = parts[1];
        const std::string& priceStr = parts[2];

        // Kiểm tra không rỗng và không chứa ký tự '|'
        if (type.empty() || area.empty()) {
            throw std::runtime_error("FeePolicy: empty customer_type or service_area at line " + std::to_string(lineNo + 1));
        }
        if (type.find('|') != std::string::npos || area.find('|') != std::string::npos) {
            throw std::runtime_error("FeePolicy: illegal '|' character in type or area at line " + std::to_string(lineNo + 1));
        }
        // Kiểm tra price là số nguyên dương
        if (priceStr.empty()) {
            throw std::runtime_error("FeePolicy: empty price at line " + std::to_string(lineNo + 1));
        }
        for (char c : priceStr) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                throw std::runtime_error("FeePolicy: non‑digit character in price at line " + std::to_string(lineNo + 1));
            }
        }
        long long price = 0;
        try {
            price = std::stoll(priceStr);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("FeePolicy: invalid number format for price at line " + std::to_string(lineNo + 1));
        } catch (const std::out_of_range&) {
            throw std::runtime_error("FeePolicy: price out of range at line " + std::to_string(lineNo + 1));
        }
        if (price <= 0) {
            throw std::runtime_error("FeePolicy: price must be > 0 at line " + std::to_string(lineNo + 1));
        }
        // Kiểm tra trùng key
        std::string key = makeKey(type, area);
        if (feeTable.find(key) != feeTable.end()) {
            throw std::runtime_error("FeePolicy: duplicate fee entry for " + key + " at line " + std::to_string(lineNo + 1));
        }
        feeTable.emplace(std::move(key), price);
    }
    // Kiểm tra ít nhất một mục
    if (feeTable.empty()) {
        throw std::runtime_error("FeePolicy: no valid fee entries loaded from " + feeFilePath);
    }
}

long long FeePolicy::getFee(const std::string& customerType,
                           const std::string& serviceArea) const
{
    std::string key = makeKey(customerType, serviceArea);
    auto it = feeTable.find(key);
    if (it == feeTable.end()) {
        throw std::runtime_error("FeePolicy: fee not defined for " + key);
    }
    return it->second;
}
