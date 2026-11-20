
#include "Expense.h"

#include <cctype>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace {

bool isBlank(const string& value) {
    for (size_t i = 0; i < value.size(); ++i) {
        if (!isspace(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }
    return true;
}

// Chuan hoa chu ASCII va loai bo khoang trang dau/cuoi.
string normalizedCategory(const string& value) {
    size_t begin = 0;
    size_t end = value.size();

    while (begin < end &&
           isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }

    while (end > begin &&
           isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    string result = value.substr(begin, end - begin);

    for (size_t i = 0; i < result.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(result[i]);

        if (ch >= 'A' && ch <= 'Z') {
            result[i] = static_cast<char>(ch - 'A' + 'a');
        }
    }

    return result;
}

void validateText(const string& value, const string& field) {
    if (isBlank(value)) {
        throw invalid_argument(field + " khong duoc rong");
    }

    if (value.find_first_of("|\r\n") != string::npos) {
        throw invalid_argument(
            field + " chua ky tu phan cach hoac xuong dong"
        );
    }
}

void validateCategory(const string& value) {
    validateText(value, "Loai chi phi");

    const string type = normalizedCategory(value);

    // Luong do Payroll phu trach, khong ghi trung vao Expense.
    if (type == "salary" ||
        type == "payroll" ||
        type == "wage" ||
        type == "wages" ||
        type == "luong" ||
        type == "tien luong" ||
        type == "lương" ||
        type == "tiền lương") {

        throw invalid_argument(
            "Khong nhap luong vao Expense; "
            "luong duoc tinh o Payroll"
        );
    }
}

bool isValidDate(const string& value) {
    if (value.size() != 10 ||
        value[4] != '-' ||
        value[7] != '-') {
        return false;
    }

    for (size_t i = 0; i < value.size(); ++i) {
        if (i != 4 && i != 7 &&
            (value[i] < '0' || value[i] > '9')) {
            return false;
        }
    }

    const int year = stoi(value.substr(0, 4));
    const int month = stoi(value.substr(5, 2));
    const int day = stoi(value.substr(8, 2));

    if (year < 1 || month < 1 || month > 12) {
        return false;
    }

    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    const bool leapYear =
        (year % 400 == 0) ||
        (year % 4 == 0 && year % 100 != 0);

    if (leapYear) {
        daysInMonth[1] = 29;
    }

    return day >= 1 && day <= daysInMonth[month - 1];
}

} // namespace


// ==================== CONSTRUCTOR ====================

Expense::Expense()
    : expense_id(""),
      category(""),
      amount(0),
      date(""),
      description("") {}

Expense::Expense(string expense_id,
                 string category,
                 long long amount,
                 string date,
                 string description)
    : Expense() {

    setExpense_id(expense_id);
    setCategory(category);
    setAmount(amount);
    setDate(date);
    setDescription(description);
}


// ==================== GETTERS / SETTERS ====================

string Expense::getExpense_id() const {
    return expense_id;
}

void Expense::setExpense_id(const string& value) {
    validateText(value, "Ma chi phi");
    expense_id = value;
}

string Expense::getCategory() const {
    return category;
}

void Expense::setCategory(const string& value) {
    validateCategory(value);
    category = value;
}

long long Expense::getAmount() const {
    return amount;
}

void Expense::setAmount(long long value) {
    if (value <= 0) {
        throw invalid_argument(
            "So tien chi phi phai lon hon 0"
        );
    }

    amount = value;
}

string Expense::getDate() const {
    return date;
}

void Expense::setDate(const string& value) {
    if (!isValidDate(value)) {
        throw invalid_argument(
            "Ngay chi phi khong hop le (YYYY-MM-DD)"
        );
    }

    date = value;
}

string Expense::getDescription() const {
    return description;
}

void Expense::setDescription(const string& value) {
    validateText(value, "Mo ta chi phi");
    description = value;
}


// ==================== VALIDATE ====================

void Expense::validate() const {
    validateText(expense_id, "Ma chi phi");
    validateCategory(category);

    if (amount <= 0) {
        throw invalid_argument(
            "So tien chi phi phai lon hon 0"
        );
    }

    if (!isValidDate(date)) {
        throw invalid_argument(
            "Ngay chi phi khong hop le (YYYY-MM-DD)"
        );
    }

    validateText(description, "Mo ta chi phi");
}


// ==================== DISPLAY ====================

void Expense::Show() const {
    cout << "\n===== THONG TIN CHI PHI =====\n";
    cout << "Ma chi phi: " << expense_id << '\n';
    cout << "Loai chi phi: " << category << '\n';
    cout << "So tien: " << amount << " VND\n";
    cout << "Ngay phat sinh: " << date << '\n';
    cout << "Mo ta: " << description << '\n';
}
