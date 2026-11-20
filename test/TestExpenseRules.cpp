
#include "../Managers/ExpenseManager.h"
#include "../FileIO/FileManager.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int main() {
    int checks = 0;

    const string banned[] = {
        "Salary", "sAlArY", "  Salary  ",
        "Payroll", "Wages", "Luong",
        "Lương", "Tiền lương"
    };

    // 1. Khong cho tao Expense thuoc loai luong.
    for (const string& category : banned) {
        bool rejected = false;

        try {
            Expense expense(
                "CP01", category, 1000,
                "2026-11-18", "test"
            );
        } catch (const invalid_argument&) {
            rejected = true;
        }

        assert(rejected);
        ++checks;
    }

    // 2. Khong cho sua loai Fuel thanh Salary.
    Expense valid(
        "CP01", "Fuel", 1000,
        "2026-11-18", "test"
    );

    bool rejected = false;

    try {
        valid.setCategory("salary");
    } catch (const invalid_argument&) {
        rejected = true;
    }

    assert(rejected);
    assert(valid.getCategory() == "Fuel");
    ++checks;

    // 3. Nap file chua Salary phai bao loi.
    // Danh sach RAM cu phai duoc giu nguyen.
    ExpenseManager expenses;
    expenses.addExpense(valid);

    const string path = "data/_salary_reject_test.txt";

    FileManager::getInstance().writeFile(path, {
        "expense_id|expense_type|expense_date|amount|description",
        "CP02|Salary|2026-11-18|1000|test"
    });

    rejected = false;

    try {
        expenses.loadFromFile(path);
    } catch (const runtime_error&) {
        rejected = true;
    }

    assert(rejected);
    assert(expenses.getSize() == 1);
    assert(expenses[0].getCategory() == "Fuel");
    ++checks;

    remove(path.c_str());

    cout << "PASS " << checks
         << " salary exclusion & import safety checks\n";

    return 0;
}
