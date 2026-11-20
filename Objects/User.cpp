
#include "User.h"

#include <cctype>
#include <stdexcept>

using namespace std;

namespace {

bool validIdentifier(const string& text) {
    if (text.empty() || text.size() > 64)
        return false;

    for (unsigned char c : text) {
        if (!(isalnum(c) || c == '_' ||
              c == '-' || c == '.')) {
            return false;
        }
    }

    return true;
}

bool validUsername(const string& text) {
    return text.size() >= 3 &&
           text.size() <= 32 &&
           validIdentifier(text);
}

bool validDemoPassword(const string& text) {
    if (text.size() < 8 || text.size() > 128)
        return false;

    for (unsigned char c : text) {
        if (c < 33 || c > 126 || c == '|')
            return false;
    }

    return true;
}

} // namespace


// ==================== CONSTRUCTOR ====================

User::User(const string& userId,
           const string& login,
           const string& secret,
           UserRole accountRole,
           const string& linkedId,
           bool enabled)
    : user_id(userId),
      username(login),
      password(secret),
      role(accountRole),
      linked_id(linkedId),
      active(enabled) {

    validate();
}


// ==================== GETTERS ====================

const string& User::getUserId() const {
    return user_id;
}

const string& User::getUsername() const {
    return username;
}

UserRole User::getRole() const {
    return role;
}

const string& User::getLinkedId() const {
    return linked_id;
}

bool User::isActive() const {
    return active;
}


// ==================== SETTERS ====================

void User::setPassword(const string& newPassword) {
    if (!validDemoPassword(newPassword)) {
        throw invalid_argument(
            "Mat khau demo phai co 8-128 ky tu ASCII, "
            "khong co | hoac khoang trang"
        );
    }

    password = newPassword;
}

void User::setActive(bool newStatus) {
    active = newStatus;
}


// ==================== AUTHENTICATION ====================

bool User::matchesPassword(
    const string& candidate
) const {
    return password == candidate;
}


// ==================== VALIDATION ====================

void User::validate() const {
    if (!validIdentifier(user_id)) {
        throw invalid_argument(
            "User ID khong hop le"
        );
    }

    if (!validUsername(username)) {
        throw invalid_argument(
            "Username phai dai 3-32 ky tu "
            "va chi gom chu/so/._-"
        );
    }

    if (!validIdentifier(linked_id)) {
        throw invalid_argument(
            "Linked ID khong hop le"
        );
    }

    if (!validDemoPassword(password)) {
        throw invalid_argument(
            "Mat khau demo khong hop le"
        );
    }

    roleToString(role);
}


// ==================== ROLE CONVERSION ====================

string User::roleToString(UserRole value) {
    switch (value) {
        case UserRole::Manager:
            return "Manager";

        case UserRole::Employee:
            return "Employee";

        case UserRole::Customer:
            return "Customer";
    }

    throw invalid_argument(
        "Role khong hop le"
    );
}

UserRole User::roleFromString(
    const string& text
) {
    if (text == "Manager")
        return UserRole::Manager;

    if (text == "Employee")
        return UserRole::Employee;

    if (text == "Customer")
        return UserRole::Customer;

    throw invalid_argument(
        "Role phai la Manager, Employee hoac Customer"
    );
}
