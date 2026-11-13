
#include "Managers/CustomerManager.h"
#include "Managers/InvoiceManager.h"
#include "Policies/FeePolicy.h"
#include "FileIO/FileManager.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>
#include <sstream>

using namespace std;

namespace {

const string CUSTOMER_FILE = "data/demo_customer.txt";
const string INVOICE_FILE = "data/demo_invoice.txt";
const string FEE_FILE = "data/fee.txt";

string ask(const string& label) {
    cout << label;
    string result;
    if (!getline(cin, result)) {
        throw runtime_error("Da dong dau vao");
    }
    return result;
}

int askMenu() {
    string value = ask("Chon chuc nang: ");
    stringstream ss(value);
    int option = -1;
    char extra;

    if (!(ss >> option) || (ss >> extra)) {
        return -1;
    }

    return option;
}

unique_ptr<Customer> newCustomer(
    const FeePolicy& fee,
    const string& id,
    const string& currentName = "",
    const string& currentPhone = "",
    const string& currentAddress = "",
    const string& currentType = "",
    const string& currentArea = "")
{
    string type = ask(
        "Loai (Household/Business) [" + currentType + "]: "
    );
    if (type.empty()) type = currentType;

    string area = ask(
        "Khu vuc (Household: Area1/Area2; "
        "Business: Type1/Type2) [" + currentArea + "]: "
    );
    if (area.empty()) area = currentArea;

    string name = ask("Ten khach hang [" + currentName + "]: ");
    if (name.empty()) name = currentName;

    string phone = ask("So dien thoai [" + currentPhone + "]: ");
    if (phone.empty()) phone = currentPhone;

    string address = ask("Dia chi [" + currentAddress + "]: ");
    if (address.empty()) address = currentAddress;

    long long monthlyFee = fee.getFee(type, area);

    return unique_ptr<Customer>(
        new Customer(
            id, type, name, phone,
            address, area, monthlyFee
        )
    );
}

void checkRelations(
    const CustomerManager& customers,
    const InvoiceManager& invoices)
{
    for (int i = 0; i < invoices.getSize(); ++i) {
        const Invoice& invoice = invoices[i];

        if (customers.findCustomerById(
                invoice.getCustomer_id()) == nullptr) {
            throw runtime_error(
                "Hoa don " + invoice.getInvoice_id() +
                " tham chieu KH khong ton tai: " +
                invoice.getCustomer_id()
            );
        }
    }
}

void report(
    const CustomerManager& customers,
    const InvoiceManager& invoices)
{
    cout << "\n===== BAO CAO TONG HOP =====\n";

    cout << "So khach hang: "
         << customers.getSize() << '\n';

    cout << "So hoa don: "
         << invoices.getSize() << '\n';

    cout << "Du kien phi thang: "
         << customers.calculateTotalExpectedRevenue()
         << " VND\n";

    cout << "Tong gia tri hoa don: "
         << invoices.calculateTotalInvoiced()
         << " VND\n";

    cout << "Da thu: "
         << invoices.calculateCollectedRevenue()
         << " VND\n";

    cout << "Chua thu: "
         << invoices.calculateTotalInvoiced()
            - invoices.calculateCollectedRevenue()
         << " VND\n";
}

void printMenu() {
    cout << "\n========== PBL2 ==========\n";
    cout << "QUAN LY KHACH HANG + HOA DON\n";
    cout << "===========================\n";

    cout << "1. Danh sach khach hang\n";
    cout << "2. Tim khach hang theo ID\n";
    cout << "3. Them khach hang\n";
    cout << "4. Sua thong tin khach hang\n";
    cout << "5. Xoa khach hang\n";

    cout << "---------------------------\n";

    cout << "6. Danh sach hoa don\n";
    cout << "7. Tim hoa don theo ID\n";
    cout << "8. Tra cuu hoa don theo KH\n";
    cout << "9. Tao hoa don moi\n";
    cout << "10. Ghi nhan thanh toan\n";
    cout << "11. Xoa hoa don\n";
    cout << "12. Hoa don chua thanh toan\n";

    cout << "---------------------------\n";

    cout << "13. Bao cao thu phi\n";
    cout << "0. Thoat\n";
}

void runMenu(
    CustomerManager& customers,
    InvoiceManager& invoices,
    const FeePolicy& fee)
{
    while (true) {
        printMenu();

        int choice = askMenu();

        if (choice == 0) return;

        try {
            switch (choice) {

            case 1:
                customers.displayAll();
                break;

            case 2: {
                string id = ask("Nhap ma khach hang: ");
                Customer* c = customers.findCustomerById(id);

                if (c) c->Show();
                else cout << "Khong tim thay!\n";
                break;
            }

            case 3: {
                string id = ask("Ma khach hang moi: ");

                unique_ptr<Customer> c = newCustomer(fee, id);

                customers.addCustomer(c.get());
                c.release();

                customers.saveToFile(CUSTOMER_FILE);

                cout << "Da them khach hang!\n";
                break;
            }

            case 4: {
                string id = ask("Ma khach hang can sua: ");

                Customer* old = customers.findCustomerById(id);

                if (!old) {
                    cout << "Khong tim thay khach hang!\n";
                    break;
                }

                unique_ptr<Customer> c = newCustomer(
                    fee,
                    id,
                    old->getCustomer_name(),
                    old->getCustomer_phone(),
                    old->getCustomer_address(),
                    old->getCustomer_type(),
                    old->getService_area()
                );

                customers.editCustomer(id, c.get());
                c.release();

                customers.saveToFile(CUSTOMER_FILE);

                cout << "Da cap nhat khach hang!\n";
                break;
            }

            case 5: {
                string id = ask("Ma khach hang can xoa: ");

                if (invoices.hasInvoicesForCustomer(id)) {
                    cout << "Khong the xoa KH dang co hoa don!\n";
                }
                else if (customers.deleteCustomer(id)) {
                    customers.saveToFile(CUSTOMER_FILE);
                    cout << "Da xoa khach hang!\n";
                }
                else {
                    cout << "Khong tim thay khach hang!\n";
                }

                break;
            }

            case 6:
                invoices.displayAll();
                break;

            case 7: {
                string id = ask("Ma hoa don can tim: ");

                const Invoice* invoice =
                    invoices.findInvoiceById(id);

                if (invoice) invoice->Show();
                else cout << "Khong tim thay hoa don!\n";
                break;
            }

            case 8: {
                string id = ask("Ma khach hang: ");
                bool found = false;

                for (int i = 0; i < invoices.getSize(); ++i) {
                    if (invoices[i].getCustomer_id() == id) {
                        invoices[i].Show();
                        found = true;
                    }
                }

                if (!found) {
                    cout << "KH chua co hoa don!\n";
                }
                break;
            }

            case 9: {
                string customerId = ask("Ma khach hang: ");

                Customer* c =
                    customers.findCustomerById(customerId);

                if (!c) {
                    cout << "Khach hang khong ton tai!\n";
                    break;
                }

                string invoiceId = ask("Ma hoa don moi: ");
                string invoiceDate = ask("Ngay lap (YYYY-MM-DD): ");
                string month = ask("Thang thu phi (YYYY-MM): ");

                Invoice invoice(
                    invoiceId,
                    customerId,
                    invoiceDate,
                    c->calculate_fee(),
                    month,
                    "Unpaid"
                );

                invoices.addInvoice(invoice);
                invoices.saveToFile(INVOICE_FILE);

                cout << "Da tao hoa don: "
                     << invoiceId << '\n';

                cout << "So tien: "
                     << c->calculate_fee() << " VND\n";

                break;
            }

            case 10: {
                string id = ask("Ma hoa don can thanh toan: ");

                if (invoices.markAsPaid(id)) {
                    invoices.saveToFile(INVOICE_FILE);
                    cout << "Thanh toan thanh cong!\n";
                }
                else {
                    cout << "Khong tim thay hoa don Unpaid!\n";
                }
                break;
            }

            case 11: {
                string id = ask("Ma hoa don can xoa: ");

                if (invoices.deleteInvoice(id)) {
                    invoices.saveToFile(INVOICE_FILE);
                    cout << "Da xoa hoa don!\n";
                }
                else {
                    cout << "Khong tim thay hoa don!\n";
                }
                break;
            }

            case 12:
                invoices.displayUnpaidInvoices();
                break;

            case 13:
                report(customers, invoices);
                break;

            default:
                cout << "Lua chon khong hop le!\n";
                break;
            }
        }
        catch (const exception& e) {
            cerr << "[LOI] " << e.what() << '\n';
        }
    }
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        FileManager& files = FileManager::getInstance();

        if (!files.fileExists(CUSTOMER_FILE) ||
            !files.fileExists(INVOICE_FILE)) {
            throw runtime_error(
                "Thieu file TXT demo trong thu muc data!"
            );
        }

        FeePolicy fee(FEE_FILE);

        CustomerManager customers;
        InvoiceManager invoices;

        customers.loadFromFile(CUSTOMER_FILE, fee);
        invoices.loadFromFile(INVOICE_FILE);

        checkRelations(customers, invoices);

        cout << "Da doc " << customers.getSize()
             << " khach hang va " << invoices.getSize()
             << " hoa don tu TXT.\n";

        if (argc > 1 && string(argv[1]) == "--check") {
            report(customers, invoices);

            cout << "[PASS] Du lieu hop le!\n";
            return 0;
        }

        runMenu(customers, invoices, fee);
        return 0;
    }
    catch (const exception& e) {
        cerr << "[LOI KHOI DONG] "
             << e.what() << '\n';
        return 1;
    }
}
