
#pragma once

#include "../Objects/User.h"

#include <cstddef>
#include <string>
#include <vector>

enum class Permission {
    ViewDashboard,
    ManageAccounts,
    ManageEmployees,
    ManageAssignments,
    ManageVehicles,
    ManageCustomers,
    ManageInvoices,
    ManageExpenses,
    ViewReports,
    ViewOwnWork,
    ViewOwnInvoices,

    // Quyen cham cong
    ViewOwnAttendance,
    CheckInSelf,
    CheckOutSelf,
    ViewAllAttendance,
    CorrectAttendance
};

struct UserSummary {
    std::string user_id;
    std::string username;
    UserRole role;
    std::string linked_id;
    bool active;
};

class AuthManager {
private:
    std::vector<User> users;
    int current_index;

    int findIndexById(
        const std::string& id
    ) const;

    int findIndexByUsername(
        const std::string& username
    ) const;

    void requireAccountAdmin() const;

public:
    AuthManager();

    // Nap tai khoan tu TXT.
    // Khong tu dong ghi file khi thoat.
    void loadFromFile(const std::string& path);

    // Dang nhap / dang xuat
    bool login(
        const std::string& username,
        const std::string& password
    );

    void logout();

    bool isLoggedIn() const;

    const User* getCurrentUser() const;

    // Kiem tra loai quyen.
    bool hasPermission(
        Permission permission
    ) const;

    // Kiem tra quyen va ID nguoi so huu.
    bool canAccess(
        Permission permission,
        const std::string& ownerId
    ) const;

    // Quan ly tai khoan (Manager).
    void addUser(const User& user);

    bool disableUser(
        const std::string& userId
    );

    bool resetPassword(
        const std::string& userId,
        const std::string& newPassword
    );

    bool changeOwnPassword(
        const std::string& oldPassword,
        const std::string& newPassword
    );

    std::vector<UserSummary> listUsers() const;

    std::size_t getUserCount() const;
};
