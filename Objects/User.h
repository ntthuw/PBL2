
#pragma once

#include <string>

enum class UserRole {
    Manager,
    Employee,
    Customer
};

class User {
private:
    std::string user_id;
    std::string username;
    std::string password; // Chi dung cho demo noi bo
    UserRole role;
    std::string linked_id;
    bool active;

public:
    User(const std::string& userId,
         const std::string& username,
         const std::string& password,
         UserRole role,
         const std::string& linkedId,
         bool active = true);

    const std::string& getUserId() const;
    const std::string& getUsername() const;
    UserRole getRole() const;
    const std::string& getLinkedId() const;
    bool isActive() const;

    void setPassword(const std::string& newPassword);
    void setActive(bool newStatus);
    bool matchesPassword(const std::string& candidate) const;

    void validate() const;

    static std::string roleToString(UserRole role);
    static UserRole roleFromString(const std::string& text);
};
