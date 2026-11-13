
#include <iostream>
#include <stdexcept>
#include "Invoice.h"
using namespace std;

int main() {
    cout << "===== DEMO INVOICE =====\n";

    // TEST 1: Constructor mac dinh
    cout << "\n[TEST 1] Constructor mac dinh\n";
    Invoice hd1;
    hd1.Show();

    // TEST 2: Constructor co tham so
    cout << "\n[TEST 2] Constructor co tham so\n";
    Invoice hd2("HD001", "KH001", "2026-11-06",
                25000, "2026-11");
    hd2.Show();

    // TEST 3: Kiem tra Getter
    cout << "\n[TEST 3] Getter\n";
    cout << "Ma hoa don: " << hd2.getInvoice_id() << endl;
    cout << "Ma khach hang: " << hd2.getCustomer_id() << endl;
    cout << "Ngay lap: " << hd2.getInvoice_date() << endl;
    cout << "Trang thai: " << hd2.getInvoice_status() << endl;
    cout << "Tong tien: " << hd2.getTotal_amount() << endl;
    cout << "Thang thu phi: " << hd2.getBilling_month() << endl;

    // TEST 4: Setter hop le
    cout << "\n[TEST 4] Setter hop le\n";
    hd2.setInvoice_status("Paid");
    hd2.setTotal_amount(30000);
    hd2.Show();

    // TEST 5: Trang thai khong hop le
    cout << "\n[TEST 5] Trang thai sai\n";
    try {
        hd2.setInvoice_status("Pending");
        cout << "FAIL: Khong phat hien trang thai sai\n";
    }
    catch (const invalid_argument& e) {
        cout << "PASS: " << e.what() << endl;
    }

    // TEST 6: So tien am
    cout << "\n[TEST 6] So tien am\n";
    try {
        hd2.setTotal_amount(-10000);
        cout << "FAIL: Khong phat hien so tien am\n";
    }
    catch (const invalid_argument& e) {
        cout << "PASS: " << e.what() << endl;
    }

    // TEST 7: Ngay khong hop le
    cout << "\n[TEST 7] Ngay khong hop le\n";
    try {
        hd2.setInvoice_date("2026-02-30");
        cout << "FAIL: Khong phat hien ngay sai\n";
    }
    catch (const invalid_argument& e) {
        cout << "PASS: " << e.what() << endl;
    }

    // TEST 8: Thang khong hop le
    cout << "\n[TEST 8] Thang khong hop le\n";
    try {
        hd2.setBilling_month("2026-13");
        cout << "FAIL: Khong phat hien thang sai\n";
    }
    catch (const invalid_argument& e) {
        cout << "PASS: " << e.what() << endl;
    }

    // TEST 9: Ma hoa don rong
    cout << "\n[TEST 9] Ma hoa don rong\n";
    try {
        hd2.setInvoice_id("");
        cout << "FAIL: Khong phat hien ma rong\n";
    }
    catch (const invalid_argument& e) {
        cout << "PASS: " << e.what() << endl;
    }

    // TEST 10: Ngay nhuan hop le
    cout << "\n[TEST 10] Ngay nhuan hop le\n";
    try {
        hd2.setInvoice_date("2024-02-29");
        cout << "PASS: Ngay nhuan hop le\n";
    }
    catch (const invalid_argument& e) {
        cout << "FAIL: " << e.what() << endl;
    }

    cout << "\n===== KET THUC DEMO =====\n";
    return 0;
}
