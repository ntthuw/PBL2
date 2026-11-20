
#pragma once

#include <string>

using namespace std;

class Expense {
private:
    string expense_id;
    string category;         // Chi phi phat sinh, KHONG bao gom luong
    long long amount;        // Don vi: VND
    string date;             // Dinh dang: YYYY-MM-DD
    string description;

public:
    Expense();
    Expense(string expense_id, string category, long long amount,
            string date, string description);

    string getExpense_id() const;
    void setExpense_id(const string& expense_id);

    string getCategory() const;
    void setCategory(const string& category);

    long long getAmount() const;
    void setAmount(long long amount);

    string getDate() const;
    void setDate(const string& date);

    string getDescription() const;
    void setDescription(const string& description);

    // Kiem tra tinh hop le cua toan bo doi tuong.
    void validate() const;

    void Show() const;
};
