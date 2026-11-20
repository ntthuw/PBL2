
#include "Managers/ExpenseManager.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace {

const string DATA_PATH = "data/demo_expenses.txt";

string ask(const string& label) {
    cout << label;

    string value;

    if (!getline(cin, value)) {
        throw runtime_error("Ket thuc nhap du lieu");
    }

    return value;
}

long long parseAmount(const string& raw) {
    if (raw.empty()) {
        throw invalid_argument("So tien khong duoc rong");
    }

    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] < '0' || raw[i] > '9') {
            throw invalid_argument(
                "So tien phai la so nguyen duong"
            );
        }
    }

    size_t parsed = 0;
    long long value = stoll(raw, &parsed);

    if (parsed != raw.size() || value <= 0) {
        throw invalid_argument("So tien phai lon hon 0");
    }

    return value;
}

int readChoice() {
    const string raw = ask("Chon: ");
    stringstream ss(raw);

    int choice = -1;
    char extra;

    if (!(ss >> choice) || (ss >> extra)) {
        return -1;
    }

    return choice;
}


// ==================== ADD ====================

void addExpense(ExpenseManager& manager) {
    Expense newExpense;

    const string id = ask("Ma chi phi: ");

    // Kiem tra ID ngay khi nhap.
    newExpense.setExpense_id(id);

    if (manager.findExpenseById(id) != nullptr) {
        throw invalid_argument("Ma chi phi da ton tai");
    }

    newExpense.setCategory(
        ask("Loai (Fuel/Maintenance/Other/...): ")
    );

    newExpense.setDate(
        ask("Ngay (YYYY-MM-DD): ")
    );

    newExpense.setAmount(
        parseAmount(ask("So tien (VND): "))
    );

    newExpense.setDescription(
        ask("Mo ta: ")
    );

    manager.addExpense(newExpense);

    cout << "Da them chi phi vao RAM.\n";
}


// ==================== EDIT ====================

void editExpense(ExpenseManager& manager) {
    const string id = ask("Nhap ma chi phi can sua: ");

    const Expense* current = manager.findExpenseById(id);

    if (current == nullptr) {
        cout << "Khong tim thay chi phi.\n";
        return;
    }

    current->Show();

    // Sua tren ban sao, chi cap nhat khi da nhap hop le.
    Expense updated = *current;

    string value = ask("Loai moi (Enter de giu): ");

    if (!value.empty()) {
        updated.setCategory(value);
    }

    value = ask("Ngay moi (Enter de giu): ");

    if (!value.empty()) {
        updated.setDate(value);
    }

    value = ask("So tien moi (Enter de giu): ");

    if (!value.empty()) {
        updated.setAmount(parseAmount(value));
    }

    value = ask("Mo ta moi (Enter de giu): ");

    if (!value.empty()) {
        updated.setDescription(value);
    }

    if (manager.editExpense(id, updated)) {
        cout << "Da sua chi phi trong RAM.\n";
    }
}


// ==================== SEARCH ====================

void searchExpense(const ExpenseManager& manager) {
    const string id = ask("Ma chi phi can tim: ");

    const Expense* found = manager.findExpenseById(id);

    if (found != nullptr) {
        found->Show();
    } else {
        cout << "Khong tim thay.\n";
    }
}


// ==================== STATISTICS ====================

void showStats(const ExpenseManager& manager) {
    cout << "Tong chi phi: "
         << manager.calculateTotalExpenses()
         << " VND\n";

    const string month = ask(
        "Nhap thang (YYYY-MM, Enter de bo qua): "
    );

    if (!month.empty()) {
        cout << "Chi phi thang " << month << ": "
             << manager.calculateExpensesByMonth(month)
             << " VND\n";
    }

    const string category = ask(
        "Nhap loai (Enter de bo qua): "
    );

    if (!category.empty()) {
        cout << "Chi phi loai " << category << ": "
             << manager.calculateExpensesByCategory(category)
             << " VND\n";
    }
}


// ==================== MENU ====================

void printMenu() {
    cout << "\n========== DEMO QUAN LY CHI PHI ==========\n"
         << "1. Danh sach chi phi\n"
         << "2. Them chi phi\n"
         << "3. Sua chi phi\n"
         << "4. Xoa chi phi\n"
         << "5. Tim kiem theo ID\n"
         << "6. Thong ke chi phi\n"
         << "7. Nap lai du lieu goc (bo thay doi trong RAM)\n"
         << "0. Thoat (khong luu thay doi)\n";
}

} // namespace


// ==================== MAIN ====================

int main() {
    cout << "DEMO CHI DOC: " << DATA_PATH << "\n"
         << "Moi thay doi chi nam trong RAM, thoat se mat.\n"
         << "Chay tu thu muc goc PBL2 - CODE de duong dan dung.\n";

    ExpenseManager expenses;

    try {
        expenses.loadFromFile(DATA_PATH);

        cout << "Da nap "
             << expenses.getSize()
             << " khoan chi.\n";

    } catch (const exception& e) {
        cerr << "Khong mo duoc du lieu demo: "
             << e.what() << '\n';

        cerr << "Vui long kiem tra data/demo_expenses.txt.\n";
        return 1;
    }

    while (true) {
        try {
            printMenu();

            const int choice = readChoice();

            switch (choice) {
                case 0:
                    cout << "Ket thuc demo.\n";
                    return 0;

                case 1:
                    expenses.displayAll();
                    break;

                case 2:
                    addExpense(expenses);
                    break;

                case 3:
                    editExpense(expenses);
                    break;

                case 4: {
                    const string id = ask("Ma can xoa: ");

                    cout << (
                        expenses.deleteExpense(id)
                        ? "Da xoa.\n"
                        : "Khong tim thay.\n"
                    );

                    break;
                }

                case 5:
                    searchExpense(expenses);
                    break;

                case 6:
                    showStats(expenses);
                    break;

                case 7:
                    expenses.loadFromFile(DATA_PATH);

                    cout << "Da phuc hoi "
                         << expenses.getSize()
                         << " khoan chi tu TXT.\n";
                    break;

                default:
                    cout << "Lua chon khong hop le.\n";
            }

        } catch (const exception& e) {
            cerr << "Loi: " << e.what() << '\n';

            if (cin.eof()) {
                return 0;
            }
        }
    }
}
