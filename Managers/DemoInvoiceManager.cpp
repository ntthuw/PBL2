
#include "InvoiceManager.h"
#include "TemplateUtils.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

int passed = 0;
int failed = 0;

void test(bool condition, const string& name) {
    if (condition) {
        cout << "[PASS] " << name << endl;
        passed++;
    }
    else {
        cout << "[FAIL] " << name << endl;
        failed++;
    }
}

template <typename Function>
bool throwsException(Function function) {
    try {
        function();
        return false;
    }
    catch (const exception&) {
        return true;
    }
}

int main() {
    InvoiceManager manager;

    test(
        manager.getSize() == 0 &&
        manager.getCapacity() == 0,
        "Constructor mac dinh"
    );

    manager.addInvoice(
        Invoice("HD001", "KH001", "2026-11-09",
                25000, "2026-11")
    );

    manager += Invoice(
        "HD002", "KH002", "2026-11-09",
        30000, "2026-11"
    );

    test(
        manager.getSize() == 2 &&
        manager.getCapacity() == 2,
        "Them hoa don"
    );

    manager.addInvoice(
        Invoice("HD003", "KH003", "2026-11-09",
                165000, "2026-11")
    );

    test(
        manager.getCapacity() == 4,
        "Mo rong mang dong"
    );

    test(
        throwsException([&]() {
            manager.addInvoice(
                Invoice("HD001", "KH004",
                        "2026-11-09", 25000, "2026-11")
            );
        }),
        "Trung ma hoa don"
    );

    test(
        throwsException([&]() {
            manager.addInvoice(
                Invoice("HD004", "KH001",
                        "2026-11-09", 25000, "2026-11")
            );
        }),
        "Trung khach hang va thang"
    );

    test(
        throwsException([&]() {
            Invoice empty;
            manager.addInvoice(empty);
        }),
        "Hoa don rong"
    );

    test(
        throwsException([&]() {
            manager.addInvoice(
                Invoice("HD|005", "KH005",
                        "2026-11-09", 25000, "2026-11")
            );
        }),
        "Ky tu phan cach khong hop le"
    );

    test(
        manager.findInvoiceById("HD002") != nullptr &&
        manager.findInvoiceById("NONE") == nullptr,
        "Tim kiem hoa don"
    );

    test(
        manager.hasInvoicesForCustomer("KH001") &&
        manager.hasInvoiceForCustomerInMonth(
            "KH001", "2026-11"
        ) &&
        !manager.hasInvoiceForCustomerInMonth(
            "KH001", "2026-12"
        ),
        "Kiem tra hoa don theo khach hang"
    );

    test(
        throwsException([&]() {
            (void)manager[-1];
        }) &&
        throwsException([&]() {
            (void)manager[3];
        }),
        "Truy cap ngoai mang"
    );

    test(
        manager.markAsPaid("HD001") &&
        !manager.markAsPaid("HD001") &&
        !manager.markAsPaid("NONE"),
        "Thanh toan hoa don"
    );

    test(
        manager.calculateTotalInvoiced() == 220000 &&
        manager.calculateCollectedRevenue() == 25000,
        "Thong ke doanh thu"
    );

    InvoiceManager copied(manager);
    copied.deleteInvoice("HD001");

    test(
        manager.getSize() == 3 &&
        copied.getSize() == 2,
        "Copy Constructor"
    );

    InvoiceManager assigned;
    assigned = manager;
    assigned = assigned;

    test(
        assigned.getSize() == 3,
        "Toan tu gan va tu gan"
    );

    test(
        !manager.deleteInvoice("NONE") &&
        manager.deleteInvoice("HD002") &&
        manager.getSize() == 2,
        "Xoa hoa don"
    );

    // Kiem tra doc ghi file
    const string filePath = "invoice_test.txt";

    manager.saveToFile(filePath);

    InvoiceManager loaded;
    loaded.loadFromFile(filePath);

    const Invoice* hd = loaded.findInvoiceById("HD001");

    test(
        loaded.getSize() == 2 &&
        hd != nullptr &&
        hd->getInvoice_status() == "Paid",
        "Luu va doc lai TXT"
    );

    // Kiem tra file sai dinh dang
    {
        ofstream file("invoice_invalid.txt");
        file << "HD100|KH100|2026-11-09|Unpaid|25000|2026-11|\n";
    }

    test(
        throwsException([&]() {
            loaded.loadFromFile("invoice_invalid.txt");
        }) &&
        loaded.getSize() == 2,
        "File sai dinh dang"
    );

    // Kiem tra file co hoa don trung thang
    {
        ofstream file("invoice_duplicate.txt");

        file << "HD100|KH100|2026-11-09|Unpaid|25000|2026-11\n";
        file << "HD101|KH100|2026-11-09|Unpaid|25000|2026-11\n";
    }

    test(
        throwsException([&]() {
            loaded.loadFromFile("invoice_duplicate.txt");
        }) &&
        loaded.getSize() == 2,
        "File co hoa don trung"
    );

    // Kiem tra TemplateUtils voi mang con tro
    struct TestCustomer {
        string id;
    };

    TestCustomer a{"A"};
    TestCustomer b{"B"};

    TestCustomer* customers[] = {&a, &b};

    auto result = findIf(
        customers, 2,
        [](TestCustomer* customer) {
            return customer != nullptr &&
                   customer->id == "B";
        }
    );

    test(
        result != nullptr && *result == &b,
        "TemplateUtils voi mang con tro"
    );

    cout << "\n============================" << endl;
    cout << "PASSED: " << passed << endl;
    cout << "FAILED: " << failed << endl;
    cout << "============================" << endl;

    return failed == 0 ? 0 : 1;
}

