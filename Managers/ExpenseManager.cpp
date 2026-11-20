#include "ExpenseManager.h"
#include "../FileIO/FileManager.h"

#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

namespace {

bool validMonth(const string& month) {
    if (month.size() != 7 || month[4] != '-') return false;

    for (size_t i = 0; i < month.size(); ++i) {
        if (i != 4 && (month[i] < '0' || month[i] > '9'))
            return false;
    }

    int year = stoi(month.substr(0, 4));
    int m = stoi(month.substr(5, 2));

    return year >= 1 && m >= 1 && m <= 12;
}

void safeAdd(long long& total, long long value) {
    if (value < 0 || value > numeric_limits<long long>::max() - total) {
        throw overflow_error("Tong chi phi vuot gioi han long long");
    }
    total += value;
}

string expenseToLine(const Expense& expense) {
    expense.validate();

    return expense.getExpense_id() + "|" +
           expense.getCategory() + "|" +
           expense.getDate() + "|" +
           to_string(expense.getAmount()) + "|" +
           expense.getDescription();
}

Expense lineToExpense(const string& line) {
    string fields[5];
    size_t begin = 0;

    for (int i = 0; i < 4; ++i) {
        const size_t pos = line.find('|', begin);

        if (pos == string::npos) {
            throw invalid_argument("Dong chi phi phai co dung 5 truong");
        }

        fields[i] = line.substr(begin, pos - begin);
        begin = pos + 1;
    }

    if (line.find('|', begin) != string::npos) {
        throw invalid_argument("Dong chi phi phai co dung 5 truong");
    }

    fields[4] = line.substr(begin);

    if (fields[3].empty())
        throw invalid_argument("So tien chi phi rong");

    for (size_t i = 0; i < fields[3].size(); ++i) {
        if (fields[3][i] < '0' || fields[3][i] > '9') {
            throw invalid_argument("So tien chi phi phai la so nguyen duong");
        }
    }

    long long amount = 0;

    try {
        amount = stoll(fields[3]);
    } catch (const out_of_range&) {
        throw invalid_argument("So tien chi phi vuot gioi han long long");
    }

    return Expense(fields[0], fields[1], amount, fields[2], fields[4]);
}

bool isHeader(const string& line) {
    return line == "expense_id|expense_type|expense_date|amount|description" ||
           line == "expense_id|category|date|amount|description";
}

} // namespace


// ==================== CONSTRUCTORS / DESTRUCTOR ====================

ExpenseManager::ExpenseManager()
    : arr(nullptr), size(0), capacity(0) {}

ExpenseManager::ExpenseManager(const ExpenseManager& other)
    : arr(nullptr), size(0), capacity(0) {

    if (other.capacity > 0) {
        unique_ptr<Expense[]> copied(new Expense[other.capacity]);

        for (int i = 0; i < other.size; ++i)
            copied[i] = other.arr[i];

        arr = copied.release();
    }

    size = other.size;
    capacity = other.capacity;
}

ExpenseManager& ExpenseManager::operator=(const ExpenseManager& other) {
    if (this != &other) {
        ExpenseManager temp(other);

        swap(arr, temp.arr);
        swap(size, temp.size);
        swap(capacity, temp.capacity);
    }

    return *this;
}

ExpenseManager::~ExpenseManager() {
    delete[] arr;
}


// ==================== GETTERS ====================

int ExpenseManager::getSize() const {
    return size;
}

int ExpenseManager::getCapacity() const {
    return capacity;
}

Expense* ExpenseManager::getArr() {
    return arr;
}

const Expense* ExpenseManager::getArr() const {
    return arr;
}


// ==================== OPERATORS ====================

ExpenseManager& ExpenseManager::operator+=(const Expense& expense) {
    addExpense(expense);
    return *this;
}

ExpenseManager& ExpenseManager::operator-=(const string& expenseId) {
    deleteExpense(expenseId);
    return *this;
}

Expense& ExpenseManager::operator[](int index) {
    if (index < 0 || index >= size) {
        throw out_of_range("Chi so chi phi khong hop le");
    }

    return arr[index];
}

const Expense& ExpenseManager::operator[](int index) const {
    if (index < 0 || index >= size) {
        throw out_of_range("Chi so chi phi khong hop le");
    }

    return arr[index];
}


// ==================== ADD ====================

void ExpenseManager::addExpense(const Expense& expense) {
    expense.validate();

    if (findExpenseById(expense.getExpense_id()) != nullptr) {
        throw invalid_argument("Ma chi phi da ton tai");
    }

    if (size == capacity) {
        if (capacity > numeric_limits<int>::max() / 2) {
            throw length_error("Dung luong mang chi phi qua lon");
        }

        const int newCapacity = capacity == 0 ? 2 : capacity * 2;

        unique_ptr<Expense[]> expanded(new Expense[newCapacity]);

        for (int i = 0; i < size; ++i)
            expanded[i] = arr[i];

        expanded[size] = expense;

        delete[] arr;
        arr = expanded.release();
        capacity = newCapacity;
    } else {
        arr[size] = expense;
    }

    ++size;
}


// ==================== EDIT ====================

bool ExpenseManager::editExpense(
    const string& expenseId,
    const Expense& updatedExpense
) {
    updatedExpense.validate();

    if (updatedExpense.getExpense_id() != expenseId)
        return false;

    Expense* existing = findExpenseById(expenseId);

    if (existing == nullptr)
        return false;

    *existing = updatedExpense;
    return true;
}


// ==================== DELETE ====================

bool ExpenseManager::deleteExpense(const string& expenseId) {
    int removed = -1;

    for (int i = 0; i < size; ++i) {
        if (arr[i].getExpense_id() == expenseId) {
            removed = i;
            break;
        }
    }

    if (removed < 0)
        return false;

    unique_ptr<Expense[]> remaining(new Expense[capacity]);

    int dest = 0;

    for (int i = 0; i < size; ++i) {
        if (i != removed)
            remaining[dest++] = arr[i];
    }

    delete[] arr;
    arr = remaining.release();

    --size;
    return true;
}


// ==================== SEARCH ====================

Expense* ExpenseManager::findExpenseById(const string& expenseId) {
    for (int i = 0; i < size; ++i) {
        if (arr[i].getExpense_id() == expenseId)
            return &arr[i];
    }

    return nullptr;
}

const Expense* ExpenseManager::findExpenseById(
    const string& expenseId
) const {
    for (int i = 0; i < size; ++i) {
        if (arr[i].getExpense_id() == expenseId)
            return &arr[i];
    }

    return nullptr;
}


// ==================== DISPLAY ====================

void ExpenseManager::displayAll() const {
    if (size == 0) {
        cout << "Danh sach chi phi rong!\n";
        return;
    }

    cout << "\n===== DANH SACH CHI PHI =====\n";

    for (int i = 0; i < size; ++i)
        arr[i].Show();
}


// ==================== STATISTICS ====================

long long ExpenseManager::calculateTotalExpenses() const {
    long long total = 0;

    for (int i = 0; i < size; ++i) {
        arr[i].validate();
        safeAdd(total, arr[i].getAmount());
    }

    return total;
}

long long ExpenseManager::calculateExpensesByMonth(
    const string& month
) const {
    if (!validMonth(month)) {
        throw invalid_argument("Thang khong hop le (YYYY-MM)");
    }

    long long total = 0;

    for (int i = 0; i < size; ++i) {
        arr[i].validate();

        if (arr[i].getDate().compare(0, 7, month) == 0) {
            safeAdd(total, arr[i].getAmount());
        }
    }

    return total;
}

long long ExpenseManager::calculateExpensesByCategory(
    const string& category
) const {
    if (category.empty())
        throw invalid_argument("Loai chi phi rong");

    long long total = 0;

    for (int i = 0; i < size; ++i) {
        arr[i].validate();

        if (arr[i].getCategory() == category)
            safeAdd(total, arr[i].getAmount());
    }

    return total;
}


// ==================== LOAD FILE ====================

void ExpenseManager::loadFromFile(const string& filePath) {
    FileManager& fm = FileManager::getInstance();

    // Neu sai duong dan, giu nguyen danh sach hien co
    // thay vi xoa het.
    const vector<string> lines = fm.readFile(filePath);

    ExpenseManager loaded;

    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0 && isHeader(lines[i]))
            continue;

        if (lines[i].empty())
            continue;

        try {
            loaded.addExpense(lineToExpense(lines[i]));
        } catch (const exception& e) {
            throw runtime_error(
                "Loi du lieu expenses.txt dong " +
                to_string(i + 1) + ": " + e.what()
            );
        }
    }

    // Chi thay du lieu khi da doc xong toan bo file.
    *this = loaded;
}


// ==================== SAVE FILE ====================

void ExpenseManager::saveToFile(const string& filePath) const {
    if (filePath.empty())
        throw invalid_argument("Duong dan file chi phi rong");

    vector<string> lines;
    ExpenseManager checked;

    for (int i = 0; i < size; ++i) {
        // Bat loi trung ID do sua truc tiep getArr/operator[].
        checked.addExpense(arr[i]);
        lines.push_back(expenseToLine(arr[i]));
    }

    FileManager& fm = FileManager::getInstance();

    const string temporaryPath = filePath + ".tmp";
    const string backupPath = filePath + ".bak";

    if (fm.fileExists(temporaryPath) || fm.fileExists(backupPath)) {
        throw runtime_error(
            "Ton tai file .tmp/.bak tu lan luu truoc; "
            "hay kiem tra truoc khi tiep tuc"
        );
    }

    fm.writeFile(temporaryPath, lines);

    const bool existed = fm.fileExists(filePath);

    if (existed && rename(filePath.c_str(), backupPath.c_str()) != 0) {
        remove(temporaryPath.c_str());
        throw runtime_error("Khong the doi ten file cu sang .bak");
    }

    if (rename(temporaryPath.c_str(), filePath.c_str()) != 0) {
        if (existed)
            rename(backupPath.c_str(), filePath.c_str());

        remove(temporaryPath.c_str());

        throw runtime_error(
            "Khong the thay the file chi phi; kiem tra .bak"
        );
    }

    if (existed)
        remove(backupPath.c_str());
}