
#include "Managers/AuthManager.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

const string DATA_PATH = "data/demo_users.txt";

// ==================== GIAO DIEN CHUNG ====================

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    cout << "\033[2J\033[H";
#endif
}

string ask(const string& label) {
    cout << label;

    string value;
    if (!getline(cin, value)) {
        throw runtime_error("Da ket thuc luong nhap.");
    }

    return value;
}

int askChoice() {
    const string raw = ask("Lua chon: ");
    istringstream ss(raw);

    int choice = -1;
    char extra;

    if (!(ss >> choice) || (ss >> extra))
        return -1;

    return choice;
}

void pauseScreen() {
    ask("\nNhan Enter de tiep tuc...");
}

void title(const string& heading) {
    cout << "========================================\n";
    cout << "  PBL2 - HE THONG THU GOM RAC THAI\n";
    cout << "========================================\n";
    cout << heading << "\n";
    cout << "----------------------------------------\n";
}

void sessionHeader(
    const AuthManager& auth,
    const string& name
) {
    clearScreen();
    title(name);

    const User* current = auth.getCurrentUser();

    if (current) {
        cout << "Tai khoan: " << current->getUsername()
             << "  |  Vai tro: "
             << User::roleToString(current->getRole())
             << "\nLien ket ID: "
             << current->getLinkedId() << "\n";
    }

    cout << "----------------------------------------\n";
}


// ==================== CHUC NANG CHUA TICH HOP ====================

void showIntegrationNotice(
    const AuthManager& auth,
    Permission permission,
    const string& feature
) {
    if (!auth.hasPermission(permission)) {
        cout << "Khong co quyen truy cap chuc nang nay.\n";
        return;
    }

    cout << "\n[" << feature << "]\n";
    cout << "Module chua ket noi trong demo Auth.\n";
    cout << "Khi tich hop main/Raylib, Controller se goi "
         << "Manager/System tuong ung.\n";
}

void showOwnIntegrationNotice(
    const AuthManager& auth,
    Permission permission,
    const string& feature
) {
    const User* current = auth.getCurrentUser();

    if (!current ||
        !auth.canAccess(permission, current->getLinkedId())) {
        cout << "Khong co quyen truy cap du lieu nay.\n";
        return;
    }

    showIntegrationNotice(auth, permission, feature);

    cout << "Pham vi: chi du lieu cua "
         << current->getLinkedId() << ".\n";
}


// ==================== THONG TIN TAI KHOAN ====================

void printProfile(const AuthManager& auth) {
    const User* current = auth.getCurrentUser();

    if (!current)
        return;

    cout << "\nMa tai khoan : "
         << current->getUserId() << '\n';

    cout << "Ten dang nhap: "
         << current->getUsername() << '\n';

    cout << "Vai tro      : "
         << User::roleToString(current->getRole()) << '\n';

    cout << "Ma lien ket  : "
         << current->getLinkedId() << '\n';

    cout << "Trang thai   : "
         << (current->isActive() ? "Active" : "Disabled")
         << '\n';
}

void profileWorkspace(AuthManager& auth) {
    while (auth.isLoggedIn()) {
        sessionHeader(auth, "THONG TIN TAI KHOAN");

        cout << "1. Xem ho so\n";
        cout << "2. Doi mat khau\n";
        cout << "0. Quay lai Dashboard\n";

        try {
            int choice = askChoice();

            if (choice == 0)
                return;

            if (choice == 1) {
                printProfile(auth);
            }
            else if (choice == 2) {
                const string oldPassword =
                    ask("Mat khau cu: ");

                const string newPassword =
                    ask("Mat khau moi: ");

                cout << (
                    auth.changeOwnPassword(
                        oldPassword,
                        newPassword
                    )
                    ? "Da doi mat khau trong RAM.\n"
                    : "Mat khau cu khong dung.\n"
                );
            }
            else {
                cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            if (cin.eof())
                throw;

            cout << "Loi: " << e.what() << '\n';
        }

        pauseScreen();
    }
}


// ==================== QUAN LY TAI KHOAN ====================

string lowerAscii(string value) {
    for (char& ch : value) {
        unsigned char c =
            static_cast<unsigned char>(ch);

        if (c >= 'A' && c <= 'Z') {
            ch = static_cast<char>(c - 'A' + 'a');
        }
    }

    return value;
}

void addAccount(AuthManager& auth) {
    const vector<UserSummary> accounts = auth.listUsers();

    string id = ask("Ma User moi: ");

    if (id.empty()) {
        throw invalid_argument("Ma User khong duoc rong.");
    }

    for (const UserSummary& u : accounts) {
        if (u.user_id == id) {
            throw invalid_argument("Ma User da ton tai.");
        }
    }

    string username = ask("Ten dang nhap: ");

    if (username.size() < 3 || username.size() > 32) {
        throw invalid_argument(
            "Username phai co tu 3 den 32 ky tu."
        );
    }

    for (const UserSummary& u : accounts) {
        if (lowerAscii(u.username) == lowerAscii(username)) {
            throw invalid_argument("Username da ton tai.");
        }
    }

    const UserRole role = User::roleFromString(
        ask("Vai tro (Manager/Employee/Customer): ")
    );

    const string linkedId =
        ask("Ma Employee/Customer lien ket: ");

    const string password =
        ask("Mat khau demo (>= 8 ky tu): ");

    auth.addUser(
        User(id, username, password, role, linkedId)
    );

    cout << "Da them tai khoan vao RAM.\n";
}

void accountWorkspace(AuthManager& auth) {
    if (!auth.hasPermission(Permission::ManageAccounts))
        return;

    while (auth.isLoggedIn()) {
        sessionHeader(auth, "QUAN LY TAI KHOAN (MANAGER)");

        cout << "1. Danh sach tai khoan\n";
        cout << "2. Them tai khoan\n";
        cout << "3. Khoa tai khoan\n";
        cout << "4. Reset mat khau\n";
        cout << "0. Quay lai Dashboard\n";

        try {
            int choice = askChoice();

            if (choice == 0)
                return;

            switch (choice) {
                case 1: {
                    cout << "\nID | Username | Role | "
                         << "Linked ID | Status\n";

                    for (const UserSummary& u : auth.listUsers()) {
                        cout << u.user_id
                             << " | " << u.username
                             << " | " << User::roleToString(u.role)
                             << " | " << u.linked_id
                             << " | "
                             << (u.active ? "Active" : "Disabled")
                             << '\n';
                    }

                    break;
                }

                case 2:
                    addAccount(auth);
                    break;

                case 3: {
                    const string id =
                        ask("Ma User can khoa: ");

                    cout << (
                        auth.disableUser(id)
                        ? "Da khoa tai khoan trong RAM.\n"
                        : "Khong tim thay hoac tai khoan da khoa.\n"
                    );

                    break;
                }

                case 4: {
                    const string id =
                        ask("Ma User can reset: ");

                    const string newPassword =
                        ask("Mat khau moi: ");

                    cout << (
                        auth.resetPassword(id, newPassword)
                        ? "Da reset mat khau trong RAM.\n"
                        : "Khong tim thay tai khoan.\n"
                    );

                    break;
                }

                default:
                    cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            if (cin.eof())
                throw;

            cout << "Loi: " << e.what() << '\n';
        }

        pauseScreen();
    }
}


// ==================== WORKSPACE MANAGER ====================

void managerWorkspace(AuthManager& auth) {
    while (auth.isLoggedIn()) {
        sessionHeader(auth, "DASHBOARD - QUAN LY");

        cout << "1. Nhan su va cong viec\n";
        cout << "2. Phuong tien thu gom\n";
        cout << "3. Khach hang va hoa don\n";
        cout << "4. Quan ly chi phi\n";
        cout << "5. Bao cao va thong ke\n";
        cout << "6. Quan ly / chinh sua sai sot cham cong\n";
        cout << "7. Quan ly tai khoan\n";
        cout << "8. Ho so va doi mat khau\n";
        cout << "0. Dang xuat\n";

        try {
            int choice = askChoice();

            if (choice == 0) {
                auth.logout();
                return;
            }

            if (choice == 7) {
                accountWorkspace(auth);
                continue;
            }

            if (choice == 8) {
                profileWorkspace(auth);
                continue;
            }

            switch (choice) {
                case 1:
                    showIntegrationNotice(
                        auth,
                        Permission::ManageEmployees,
                        "Nhan su va cong viec"
                    );
                    break;

                case 2:
                    showIntegrationNotice(
                        auth,
                        Permission::ManageVehicles,
                        "Phuong tien"
                    );
                    break;

                case 3:
                    showIntegrationNotice(
                        auth,
                        Permission::ManageInvoices,
                        "Khach hang va hoa don"
                    );
                    break;

                case 4:
                    showIntegrationNotice(
                        auth,
                        Permission::ManageExpenses,
                        "Chi phi"
                    );
                    break;

                case 5:
                    showIntegrationNotice(
                        auth,
                        Permission::ViewReports,
                        "Bao cao"
                    );
                    break;

                case 6:
                    showIntegrationNotice(
                        auth,
                        Permission::CorrectAttendance,
                        "Chinh sua sai sot cham cong"
                    );
                    break;

                default:
                    cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            if (cin.eof())
                throw;

            cout << "Loi: " << e.what() << '\n';
        }

        pauseScreen();
    }
}


// ==================== WORKSPACE EMPLOYEE ====================

void employeeWorkspace(AuthManager& auth) {
    while (auth.isLoggedIn()) {
        sessionHeader(auth, "DASHBOARD - NHAN VIEN");

        cout << "1. Cong viec cua toi\n";
        cout << "2. Check-in cua toi\n";
        cout << "3. Check-out cua toi\n";
        cout << "4. Lich su cham cong cua toi\n";
        cout << "5. Ho so va doi mat khau\n";
        cout << "0. Dang xuat\n";

        try {
            int choice = askChoice();

            if (choice == 0) {
                auth.logout();
                return;
            }

            if (choice == 5) {
                profileWorkspace(auth);
                continue;
            }

            switch (choice) {
                case 1:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::ViewOwnWork,
                        "Lich cong viec"
                    );
                    break;

                case 2:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::CheckInSelf,
                        "Check-in"
                    );
                    break;

                case 3:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::CheckOutSelf,
                        "Check-out"
                    );
                    break;

                case 4:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::ViewOwnAttendance,
                        "Lich su cham cong"
                    );
                    break;

                default:
                    cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            if (cin.eof())
                throw;

            cout << "Loi: " << e.what() << '\n';
        }

        pauseScreen();
    }
}


// ==================== WORKSPACE CUSTOMER ====================

void customerWorkspace(AuthManager& auth) {
    while (auth.isLoggedIn()) {
        sessionHeader(auth, "DASHBOARD - KHACH HANG");

        cout << "1. Xem hoa don cua toi\n";
        cout << "2. Yeu cau xuat hoa don\n";
        cout << "3. Ho so va doi mat khau\n";
        cout << "0. Dang xuat\n";

        try {
            int choice = askChoice();

            if (choice == 0) {
                auth.logout();
                return;
            }

            if (choice == 3) {
                profileWorkspace(auth);
                continue;
            }

            switch (choice) {
                case 1:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::ViewOwnInvoices,
                        "Hoa don cua toi"
                    );
                    break;

                case 2:
                    showOwnIntegrationNotice(
                        auth,
                        Permission::ViewOwnInvoices,
                        "Yeu cau xuat hoa don - chua tich hop"
                    );
                    break;

                default:
                    cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            if (cin.eof())
                throw;

            cout << "Loi: " << e.what() << '\n';
        }

        pauseScreen();
    }
}


// ==================== DIEU HUONG THEO ROLE ====================

void openWorkspace(AuthManager& auth) {
    const User* current = auth.getCurrentUser();

    if (!current)
        return;

    switch (current->getRole()) {
        case UserRole::Manager:
            managerWorkspace(auth);
            break;

        case UserRole::Employee:
            employeeWorkspace(auth);
            break;

        case UserRole::Customer:
            customerWorkspace(auth);
            break;
    }
}

} // namespace


// ==================== MAIN ====================

int main() {
    AuthManager auth;

    try {
        auth.loadFromFile(DATA_PATH);
    }
    catch (const exception& e) {
        cerr << "Khong the nap tai khoan: "
             << e.what() << '\n';

        cerr << "Kiem tra file "
             << DATA_PATH << '\n';

        return 1;
    }

    try {
        while (true) {
            clearScreen();
            title("DANG NHAP HE THONG");

            cout << "1. Dang nhap\n";
            cout << "0. Thoat chuong trinh\n";

            int choice = askChoice();

            if (choice == 0) {
                cout << "Ket thuc.\n";
                return 0;
            }

            if (choice != 1) {
                cout << "Lua chon khong hop le.\n";
                pauseScreen();
                continue;
            }

            const string username =
                ask("Ten dang nhap: ");

            const string password =
                ask("Mat khau demo: ");

            if (!auth.login(username, password)) {
                cout << "Sai tai khoan, mat khau "
                     << "hoac tai khoan bi khoa.\n";

                pauseScreen();
                continue;
            }

            // Sau khi dang nhap thanh cong,
            // chuyen den Workspace phu hop voi Role.
            openWorkspace(auth);
        }
    }
    catch (const exception& e) {
        if (!cin.eof()) {
            cerr << "Loi: " << e.what() << '\n';
        }

        auth.logout();
        return cin.eof() ? 0 : 1;
    }
}
