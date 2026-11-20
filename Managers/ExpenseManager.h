#pragma once

#include "../Objects/Expense.h"
#include <string>

using namespace std;

class ExpenseManager {
private:
    Expense* arr;
    int size;
    int capacity;

public:
    ExpenseManager();
    ExpenseManager(const ExpenseManager& other);
    ExpenseManager& operator=(const ExpenseManager& other);
    ~ExpenseManager();

    int getSize() const;
    int getCapacity() const;
    Expense* getArr();
    const Expense* getArr() const;

    ExpenseManager& operator+=(const Expense& expense);
    ExpenseManager& operator-=(const string& expenseId);
    Expense& operator[](int index);
    const Expense& operator[](int index) const;

    void addExpense(const Expense& expense);

    // Khong cho doi ma chi phi. Tra ve false neu khong tim thay/doi ID.
    bool editExpense(const string& expenseId, const Expense& updatedExpense);

    bool deleteExpense(const string& expenseId);

    Expense* findExpenseById(const string& expenseId);
    const Expense* findExpenseById(const string& expenseId) const;

    void displayAll() const;

    long long calculateTotalExpenses() const;
    long long calculateExpensesByMonth(const string& month) const;
    long long calculateExpensesByCategory(const string& category) const;

    // Dung FileManager san co.
    // File TXT khong bat buoc co header.
    // loadFromFile nem loi neu file khong ton tai,
    // KHONG xoa du lieu trong RAM.
    void loadFromFile(const string& filePath);

    // Luu qua tep .tmp, doi ten tep cu sang .bak
    // de giam nguy co mat du lieu.
    void saveToFile(const string& filePath) const;
};