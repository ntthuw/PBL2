
#include "AuthManager.h"
#include "../FileIO/FileManager.h"

#include <cctype>
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

string lowerAscii(string value) {
    for (char& ch : value) {
        unsigned char c = static_cast<unsigned char>(ch);

        if (c >= 'A' && c <= 'Z') {
            ch = static_cast<char>(c - 'A' + 'a');
        }
    }

    return value;
}

vector<string> splitUserRow(const string& line) {
    vector<string> fields;
    size_t start = 0;

    while (true) {
        const size_t sep = line.find('|', start);

        if (sep == string::npos) {
            fields.push_back(line.substr(start));
            break;
        }

        fields.push_back(line.substr(start, sep - start));
        start = sep + 1;
    }

    if (fields.size() != 6) {
        throw invalid_argument(
            "Du lieu user phai gom dung 6 cot"
        );
    }

    return fields;
}

User parseUser(const string& line) {
    vector<string> f = splitUserRow(line);

    if (f[5] != "Active" && f[5] != "Disabled") {
        throw invalid_argument(
            "Account status phai la Active hoac Disabled"
        );
    }

    return User(
        f[0],
        f[1],
        f[2],
        User::roleFromString(f[3]),
        f[4],
        f[5] == "Active"
    );
}

} // namespace


// ==================== CONSTRUCTOR ====================

AuthManager::AuthManager()
    : current_index(-1) {}


// ==================== SEARCH ====================

int AuthManager::findIndexById(const string& id) const {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i].getUserId() == id) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

int AuthManager::findIndexByUsername(
    const string& login
) const {
    const string key = lowerAscii(login);

    for (size_t i = 0; i < users.size(); ++i) {
        if (
            lowerAscii(users[i].getUsername()) == key
        ) {
            return static_cast<int>(i);
        }
    }

    return -1;
}


// ==================== LOAD FILE ====================

void AuthManager::loadFromFile(const string& path) {
    const vector<string> lines =
        FileManager::getInstance().readFile(path);

    vector<User> loaded;

    for (size_t i = 0; i < lines.size(); ++i) {

        if (
            i == 0 &&
            lines[i] ==
            "user_id|username|password|role|linked_id|status"
        ) {
            continue;
        }

        if (lines[i].empty()) {
            continue;
        }

        try {
            User account = parseUser(lines[i]);

            for (const User& previous : loaded) {
                if (
                    previous.getUserId() ==
                        account.getUserId() ||

                    lowerAscii(previous.getUsername()) ==
                        lowerAscii(account.getUsername())
                ) {
                    throw invalid_argument(
                        "User ID hoac username bi trung"
                    );
                }
            }

            loaded.push_back(account);

        } catch (const exception& e) {
            throw runtime_error(
                "Loi users.txt dong " +
                to_string(i + 1) +
                ": " + e.what()
            );
        }
    }

    bool activeManager = false;

    for (const User& u : loaded) {
        if (
            u.isActive() &&
            u.getRole() == UserRole::Manager
        ) {
            activeManager = true;
        }
    }

    if (!activeManager) {
        throw runtime_error(
            "users.txt phai co it nhat mot Manager Active"
        );
    }

    users.swap(loaded);
    logout();
}


// ==================== LOGIN ====================

bool AuthManager::login(
    const string& login,
    const string& secret
) {
    logout();

    const int found = findIndexByUsername(login);

    if (found < 0) {
        return false;
    }

    const User& account = users[found];

    if (
        !account.isActive() ||
        !account.matchesPassword(secret)
    ) {
        return false;
    }

    current_index = found;
    return true;
}


// ==================== LOGOUT ====================

void AuthManager::logout() {
    current_index = -1;
}

bool AuthManager::isLoggedIn() const {
    return current_index >= 0;
}

const User* AuthManager::getCurrentUser() const {
    if (!isLoggedIn()) {
        return nullptr;
    }

    return &users[
        static_cast<size_t>(current_index)
    ];
}


// ==================== PERMISSIONS ====================

bool AuthManager::hasPermission(
    Permission permission
) const {
    const User* current = getCurrentUser();

    if (
        current == nullptr ||
        !current->isActive()
    ) {
        return false;
    }

    // Chi Employee duoc tu Check-in / Check-out.
    // Manager khong duoc cham cong ho.
    if (
        permission == Permission::CheckInSelf ||
        permission == Permission::CheckOutSelf
    ) {
        return current->getRole() ==
               UserRole::Employee;
    }

    switch (current->getRole()) {

        // ===== MANAGER =====
        case UserRole::Manager:
            switch (permission) {
                case Permission::ViewDashboard:
                case Permission::ManageAccounts:
                case Permission::ManageEmployees:
                case Permission::ManageAssignments:
                case Permission::ManageVehicles:
                case Permission::ManageCustomers:
                case Permission::ManageInvoices:
                case Permission::ManageExpenses:
                case Permission::ViewReports:
                case Permission::ViewOwnWork:
                case Permission::ViewOwnInvoices:
                case Permission::ViewOwnAttendance:
                case Permission::ViewAllAttendance:
                case Permission::CorrectAttendance:
                    return true;

                default:
                    return false;
            }

        // ===== EMPLOYEE =====
        case UserRole::Employee:
            return
                permission == Permission::ViewDashboard ||
                permission == Permission::ViewOwnWork ||
                permission == Permission::ViewOwnAttendance;

        // ===== CUSTOMER =====
        case UserRole::Customer:
            return
                permission == Permission::ViewDashboard ||
                permission == Permission::ViewOwnInvoices;
    }

    return false;
}


// ==================== OWNERSHIP CHECK ====================

bool AuthManager::canAccess(
    Permission permission,
    const string& ownerId
) const {
    if (!hasPermission(permission)) {
        return false;
    }

    const User* current = getCurrentUser();

    // Manager duoc quan ly du lieu,
    // nhung khong duoc Check-in/Check-out ho
    // vi da bi chan o hasPermission().
    if (current->getRole() == UserRole::Manager) {
        return true;
    }

    if (permission == Permission::ViewDashboard) {
        return true;
    }

    // Employee chi thao tac voi Employee ID cua minh.
    // Customer chi xem du lieu thuoc Customer ID cua minh.
    return !ownerId.empty() &&
           ownerId == current->getLinkedId();
}


// ==================== CHECK ADMIN ====================

void AuthManager::requireAccountAdmin() const {
    if (
        !hasPermission(Permission::ManageAccounts)
    ) {
        throw runtime_error(
            "Khong co quyen quan ly tai khoan"
        );
    }
}


// ==================== ADD USER ====================

void AuthManager::addUser(const User& user) {
    requireAccountAdmin();
    user.validate();

    if (
        findIndexById(user.getUserId()) >= 0 ||
        findIndexByUsername(user.getUsername()) >= 0
    ) {
        throw invalid_argument(
            "User ID hoac username da ton tai"
        );
    }

    users.push_back(user);
}


// ==================== DISABLE USER ====================

bool AuthManager::disableUser(
    const string& userId
) {
    requireAccountAdmin();

    const int found = findIndexById(userId);

    if (found < 0) {
        return false;
    }

    if (found == current_index) {
        throw invalid_argument(
            "Khong tu vo hieu hoa tai khoan dang dang nhap"
        );
    }

    if (!users[found].isActive()) {
        return false;
    }

    if (
        users[found].getRole() ==
        UserRole::Manager
    ) {
        int activeManagers = 0;

        for (const User& u : users) {
            if (
                u.isActive() &&
                u.getRole() == UserRole::Manager
            ) {
                ++activeManagers;
            }
        }

        if (activeManagers <= 1) {
            throw invalid_argument(
                "Phai giu lai it nhat mot Manager Active"
            );
        }
    }

    users[found].setActive(false);
    return true;
}


// ==================== RESET PASSWORD ====================

bool AuthManager::resetPassword(
    const string& userId,
    const string& newPassword
) {
    requireAccountAdmin();

    const int found = findIndexById(userId);

    if (found < 0) {
        return false;
    }

    users[found].setPassword(newPassword);
    return true;
}


// ==================== CHANGE OWN PASSWORD ====================

bool AuthManager::changeOwnPassword(
    const string& oldPassword,
    const string& newPassword
) {
    if (!isLoggedIn()) {
        return false;
    }

    User& current = users[
        static_cast<size_t>(current_index)
    ];

    if (!current.matchesPassword(oldPassword)) {
        return false;
    }

    current.setPassword(newPassword);

    return true;
}


// ==================== LIST USERS ====================

vector<UserSummary> AuthManager::listUsers() const {
    requireAccountAdmin();

    vector<UserSummary> result;

    for (const User& user : users) {
        result.push_back({
            user.getUserId(),
            user.getUsername(),
            user.getRole(),
            user.getLinkedId(),
            user.isActive()
        });
    }

    return result;
}


// ==================== GET SIZE ====================

size_t AuthManager::getUserCount() const {
    return users.size();
}
