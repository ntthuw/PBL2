#include <iostream>
#include "Policies/FeePolicy.h"

int main() {
    try {
        FeePolicy policy("data/fee.txt");
        std::cout << "Household Area1 => " << policy.getFee("Household", "Area1") << "\n";
        std::cout << "Household Area2 => " << policy.getFee("Household", "Area2") << "\n";
        std::cout << "Business Type1 => " << policy.getFee("Business", "Type1") << "\n";
        std::cout << "Business Type2 => " << policy.getFee("Business", "Type2") << "\n";
        // test non‑existent key
        try {
            policy.getFee("Household", "AreaX");
        } catch (const std::exception& e) {
            std::cerr << "Expected error: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
