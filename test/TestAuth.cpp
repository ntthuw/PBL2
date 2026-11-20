
#include "../Managers/AuthManager.h"
#include "../FileIO/FileManager.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdio>

using namespace std;

int main() {
    const string path = "data/_test_auth_users.txt";

    try {
        vector<string> data = {
            "user_id|username|password|role|linked_id|status",
            "U001|admin|Admin@1234|Manager|NV007|Active",
            "U002|driver01|Driver@1234|Employee|NV001|Active",
            "U003|customer01|Customer@1234|Customer|KH001|Active"
        };

        FileManager::getInstance().writeFile(path, data);

        AuthManager auth;
        auth.loadFromFile(path);

        if (auth.getUserCount() != 3)
            throw runtime_error("Loi nap tai khoan");

        // Test Manager
        if (!auth.login("admin", "Admin@1234"))
            throw runtime_error("Manager login failed");

        if (!auth.hasPermission(Permission::ManageExpenses))
            throw runtime_error("Manager permission failed");

        if (auth.hasPermission(Permission::CheckInSelf))
            throw runtime_error("Manager khong duoc check-in ho");

        auth.logout();

        // Test Employee
        if (!auth.login("driver01", "Driver@1234"))
            throw runtime_error("Employee login failed");

        if (!auth.canAccess(Permission::CheckInSelf, "NV001"))
            throw runtime_error("Employee check-in failed");

        if (auth.canAccess(Permission::CheckInSelf, "NV002"))
            throw runtime_error("Employee duoc check-in ho");

        auth.logout();

        // Test Customer
        if (!auth.login("customer01", "Customer@1234"))
            throw runtime_error("Customer login failed");

        if (!auth.canAccess(Permission::ViewOwnInvoices, "KH001"))
            throw runtime_error("Customer invoice permission failed");

        if (auth.hasPermission(Permission::CheckOutSelf))
            throw runtime_error("Customer khong duoc check-out");

        auth.logout();

        remove(path.c_str());

        cout << "PASS: AuthManager test thanh cong!\n";
        return 0;

    } catch (const exception& e) {
        cerr << "TEST FAILED: " << e.what() << '\n';
        remove(path.c_str());
        return 1;
    }
}
