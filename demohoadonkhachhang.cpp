
#include "Managers/CustomerManager.h"
#include "Managers/InvoiceManager.h"
#include "FileIO/FileManager.h"
#include "Policies/FeePolicy.h"

#include <algorithm>
#include <cstdio>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

const string CUSTOMER_PATH = "data/demo_customer.txt";
const string INVOICE_PATH = "data/demo_invoice.txt";
const string SOURCE_CUSTOMER_PATH = "data/customer.txt";
const string SOURCE_INVOICE_PATH = "data/invoice.txt";
const string FEE_PATH = "data/fee.txt";

struct DemoState {
    FeePolicy fees;
    unique_ptr<CustomerManager> customers;
    unique_ptr<InvoiceManager> invoices;

    DemoState() : fees(FEE_PATH),
                  customers(new CustomerManager()),
                  invoices(new InvoiceManager()) {}
};

bool blank(const string& text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (!isspace(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }
    return true;
}

void checkField(const string& value, const string& name) {
    if (blank(value) ||
        value.find_first_of("|\r\n") != string::npos) {
        throw invalid_argument(
            name + " rong hoac chua ky tu cam (|, xuong dong)"
        );
    }
}

string input(const string& prompt) {
    cout << prompt;
    string value;
    if (!getline(cin, value)) {
        throw runtime_error("Khong con du lieu nhap");
    }
    return value;
}

string required(const string& prompt, const string& name) {
    string value = input(prompt);
    checkField(value, name);
    return value;
}

string keepOrChange(const string& name, const string& oldValue) {
    string value = input(
        name + " [" + oldValue + "] (Enter de giu): "
    );
    if (value.empty()) return oldValue;
    checkField(value, name);
    return value;
}

int menuChoice() {
    string raw = input("Nhap lua chon: ");
    stringstream ss(raw);
    int choice = -1;
    char extra;

    if (!(ss >> choice) || (ss >> extra)) return -1;
    return choice;
}

long long parsePositiveAmount(const string& raw) {
    checkField(raw, "So tien");

    for (size_t i = 0; i < raw.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(raw[i]))) {
            throw invalid_argument(
                "So tien chi duoc la so nguyen duong"
            );
        }
    }

    size_t used = 0;
    long long amount = stoll(raw, &used);

    if (used != raw.size() || amount <= 0) {
        throw invalid_argument("So tien phai lon hon 0");
    }
    return amount;
}

// Tach dung 6 truong TXT.
vector<string> splitSix(const string& line) {
    if (count(line.begin(), line.end(), '|') != 5) {
        throw invalid_argument(
            "Can dung 6 truong, ngan cach bang 5 dau |"
        );
    }

    vector<string> fields;
    size_t start = 0;

    for (size_t pos = 0; pos < line.size(); ++pos) {
        if (line[pos] == '|') {
            fields.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }
    }

    fields.push_back(line.substr(start));
    return fields;
}

// Chuyen mot dong TXT thanh Customer.
Customer fromLine(const string& line, const FeePolicy& fees) {
    vector<string> fields = splitSix(line);

    const char* labels[6] = {
        "Ma KH", "Loai KH", "Ten KH",
        "SDT", "Dia chi", "Khu vuc"
    };

    for (int i = 0; i < 6; ++i) {
        checkField(fields[i], labels[i]);
    }

    Customer customer(
        fields[0], fields[1], fields[2],
        fields[3], fields[4], fields[5]
    );

    customer.validate();

    (void)fees.getFee(
        customer.getCustomer_type(),
        customer.getService_area()
    );

    return customer;
}

// Chuyen Customer thanh mot dong TXT.
string toLine(const Customer& c, const FeePolicy& fees) {
    c.validate();

    checkField(c.getCustomer_id(), "Ma KH");
    checkField(c.getCustomer_type(), "Loai KH");
    checkField(c.getCustomer_name(), "Ten KH");
    checkField(c.getCustomer_phone(), "SDT");
    checkField(c.getCustomer_address(), "Dia chi");
    checkField(c.getService_area(), "Khu vuc");

    (void)fees.getFee(
        c.getCustomer_type(), c.getService_area()
    );

    return c.getCustomer_id() + "|" +
           c.getCustomer_type() + "|" +
           c.getCustomer_name() + "|" +
           c.getCustomer_phone() + "|" +
           c.getCustomer_address() + "|" +
           c.getService_area();
}

// Doc danh sach khach hang tu TXT.
void loadCustomers(
    CustomerManager& manager,
    const string& path,
    const FeePolicy& fees
) {
    vector<string> lines =
        FileManager::getInstance().readFile(path);

    CustomerManager next;

    for (size_t i = 0; i < lines.size(); ++i) {
        try {
            next.addCustomer(fromLine(lines[i], fees));
        }
        catch (const exception& e) {
            throw runtime_error(
                path + ", dong " + to_string(i + 1) +
                ": " + e.what()
            );
        }
    }

    manager = next;
}

vector<string> customerLines(
    const CustomerManager& customers,
    const FeePolicy& fees
) {
    vector<string> lines;

    for (int i = 0; i < customers.getSize(); ++i) {
        lines.push_back(toLine(customers[i], fees));
    }

    return lines;
}

// Kiem tra quan he khach hang va hoa don.
void checkRelations(
    const CustomerManager& customers,
    const InvoiceManager& invoices
) {
    for (int i = 0; i < invoices.getSize(); ++i) {
        const Invoice& inv = invoices[i];

        if (!customers.findCustomerById(inv.getCustomer_id())) {
            throw runtime_error(
                "Hoa don " + inv.getInvoice_id() +
                " tham chieu KH khong ton tai: " +
                inv.getCustomer_id()
            );
        }
    }
}

// Tao du lieu demo tu du lieu goc neu chua co.
void prepareDemoFiles(const FeePolicy& fees) {
    FileManager& fm = FileManager::getInstance();

    bool hasCustomers = fm.fileExists(CUSTOMER_PATH);
    bool hasInvoices = fm.fileExists(INVOICE_PATH);

    if (hasCustomers && hasInvoices) return;

    if (hasCustomers != hasInvoices) {
        throw runtime_error(
            "Thieu mot file demo TXT. Hay kiem tra " +
            CUSTOMER_PATH + " va " + INVOICE_PATH
        );
    }

    if (!fm.fileExists(SOURCE_CUSTOMER_PATH) ||
        !fm.fileExists(SOURCE_INVOICE_PATH)) {
        throw runtime_error(
            "Khong co file demo va khong co file mau "
            "customer/invoice.txt"
        );
    }

    CustomerManager sampleCustomers;
    InvoiceManager sampleInvoices;

    loadCustomers(
        sampleCustomers, SOURCE_CUSTOMER_PATH, fees
    );
    sampleInvoices.loadFromFile(SOURCE_INVOICE_PATH);

    checkRelations(sampleCustomers, sampleInvoices);

    fm.writeFile(
        CUSTOMER_PATH,
        customerLines(sampleCustomers, fees)
    );

    sampleInvoices.saveToFile(INVOICE_PATH);

    cout << "Da tao 2 file demo TXT tu "
         << "customer.txt va invoice.txt goc.\n";
}

// Ghi file .tmp truoc, sao luu file cu thanh .bak.
void replaceDemoFile(const string& path) {
    FileManager& fm = FileManager::getInstance();

    const string temp = path + ".tmp";
    const string backup = path + ".bak";

    if (!fm.fileExists(path)) {
        throw runtime_error(
            "File demo khong ton tai: " + path
        );
    }

    if (fm.fileExists(backup)) {
        throw runtime_error(
            "Dang co file sao luu " + backup +
            ". Hay kiem tra truoc khi tiep tuc ghi."
        );
    }

    if (std::rename(path.c_str(), backup.c_str()) != 0) {
        throw runtime_error(
            "Khong the sao luu file cu: " + path
        );
    }

    if (std::rename(temp.c_str(), path.c_str()) != 0) {
        if (std::rename(backup.c_str(), path.c_str()) != 0) {
            throw runtime_error(
                "Khong the doi file moi; "
                "DU LIEU CU O: " + backup
            );
        }

        throw runtime_error(
            "Khong the cap nhat " + path +
            "; da khoi phuc file cu"
        );
    }

    if (std::remove(backup.c_str()) != 0) {
        cerr << "[CANH BAO] Khong xoa duoc backup: "
             << backup << '\n';
    }
}

void saveCustomers(
    const CustomerManager& customers,
    const FeePolicy& fees
) {
    FileManager::getInstance().writeFile(
        CUSTOMER_PATH + ".tmp",
        customerLines(customers, fees)
    );

    replaceDemoFile(CUSTOMER_PATH);
}

void saveInvoices(const InvoiceManager& invoices) {
    invoices.saveToFile(INVOICE_PATH + ".tmp");
    replaceDemoFile(INVOICE_PATH);
}

// Nhap thong tin khach hang.
Customer askCustomer(
    const string& id,
    const Customer* old = nullptr
) {
    string type = old
        ? keepOrChange(
            "Loai KH (Household/Business)",
            old->getCustomer_type())
        : required(
            "Loai KH (Household/Business): ", "Loai KH");

    string area = old
        ? keepOrChange(
            "Khu vuc (Area1/Area2/Type1/Type2)",
            old->getService_area())
        : required(
            "Khu vuc (Area1/Area2/Type1/Type2): ",
            "Khu vuc");

    string name = old
        ? keepOrChange("Ten KH", old->getCustomer_name())
        : required("Ten KH: ", "Ten KH");

    string phone = old
        ? keepOrChange(
            "So dien thoai", old->getCustomer_phone())
        : required("So dien thoai: ", "SDT");

    string address = old
        ? keepOrChange("Dia chi", old->getCustomer_address())
        : required("Dia chi: ", "Dia chi");

    return Customer(id, type, name, phone, address, area);
}

// ==================== QUAN LY KHACH HANG ====================

void showCustomer(DemoState& state) {
    string id = required("Ma khach hang: ", "Ma KH");

    const Customer* c =
        state.customers->findCustomerById(id);

    if (!c) {
        cout << "Khong tim thay khach hang.\n";
        return;
    }

    c->Show();

    cout << "Phi thang hien tai: "
         << state.fees.getFee(
            c->getCustomer_type(),
            c->getService_area())
         << " VND\n";
}

void addCustomer(DemoState& state) {
    string id = required("Ma KH moi: ", "Ma KH");
    Customer customer = askCustomer(id);

    (void)state.fees.getFee(
        customer.getCustomer_type(),
        customer.getService_area()
    );

    unique_ptr<CustomerManager> next(
        new CustomerManager(*state.customers)
    );

    next->addCustomer(customer);

    saveCustomers(*next, state.fees);
    state.customers.swap(next);

    cout << "Them khach hang va luu TXT thanh cong.\n";
}

void editCustomer(DemoState& state) {
    string id = required("Ma KH can sua: ", "Ma KH");

    const Customer* old =
        state.customers->findCustomerById(id);

    if (!old) {
        cout << "Khong tim thay khach hang.\n";
        return;
    }

    Customer changed = askCustomer(id, old);

    (void)state.fees.getFee(
        changed.getCustomer_type(),
        changed.getService_area()
    );

    unique_ptr<CustomerManager> next(
        new CustomerManager(*state.customers)
    );

    if (!next->editCustomer(id, changed)) {
        throw runtime_error("Khong the sua KH");
    }

    saveCustomers(*next, state.fees);
    state.customers.swap(next);

    cout << "Sua khach hang thanh cong; "
         << "hoa don cu khong thay doi.\n";
}

void deleteCustomer(DemoState& state) {
    string id = required("Ma KH can xoa: ", "Ma KH");

    if (state.invoices->hasInvoicesForCustomer(id)) {
        cout << "Khong the xoa KH da co hoa don.\n";
        return;
    }

    unique_ptr<CustomerManager> next(
        new CustomerManager(*state.customers)
    );

    if (!next->deleteCustomer(id)) {
        cout << "Khong tim thay KH.\n";
        return;
    }

    saveCustomers(*next, state.fees);
    state.customers.swap(next);

    cout << "Da xoa KH va luu TXT.\n";
}

// ==================== QUAN LY HOA DON ====================

void showInvoice(DemoState& state) {
    string id = required("Ma hoa don: ", "Ma HD");

    const Invoice* inv =
        state.invoices->findInvoiceById(id);

    if (inv) inv->Show();
    else cout << "Khong tim thay hoa don.\n";
}

void invoicesOfCustomer(DemoState& state) {
    string id = required("Ma KH: ", "Ma KH");

    if (!state.customers->findCustomerById(id)) {
        cout << "Khach hang khong ton tai.\n";
        return;
    }

    bool found = false;

    for (int i = 0; i < state.invoices->getSize(); ++i) {
        if ((*state.invoices)[i].getCustomer_id() == id) {
            (*state.invoices)[i].Show();
            found = true;
        }
    }

    if (!found) {
        cout << "Khach hang nay chua co hoa don.\n";
    }
}

void createInvoice(DemoState& state) {
    string id = required("Ma hoa don moi: ", "Ma HD");
    string customerId = required("Ma KH: ", "Ma KH");

    const Customer* c =
        state.customers->findCustomerById(customerId);

    if (!c) {
        cout << "Khach hang khong ton tai.\n";
        return;
    }

    string date = required(
        "Ngay lap (YYYY-MM-DD): ", "Ngay lap"
    );

    string month = required(
        "Thang thu phi (YYYY-MM): ", "Thang thu phi"
    );

    long long fee = state.fees.getFee(
        c->getCustomer_type(),
        c->getService_area()
    );

    Invoice inv(
        id, customerId, date, fee, month, "Unpaid"
    );

    unique_ptr<InvoiceManager> next(
        new InvoiceManager(*state.invoices)
    );

    next->addInvoice(inv);

    saveInvoices(*next);
    state.invoices.swap(next);

    cout << "Da tao hoa don " << id
         << ", so tien " << fee << " VND.\n";
}

void editInvoice(DemoState& state) {
    string id = required("Ma hoa don can sua: ", "Ma HD");

    const Invoice* old =
        state.invoices->findInvoiceById(id);

    if (!old) {
        cout << "Khong tim thay hoa don.\n";
        return;
    }

    string customerId = keepOrChange(
        "Ma KH", old->getCustomer_id()
    );

    const Customer* c =
        state.customers->findCustomerById(customerId);

    if (!c) {
        throw invalid_argument(
            "Khach hang moi khong ton tai"
        );
    }

    string date = keepOrChange(
        "Ngay lap", old->getInvoice_date()
    );

    string month = keepOrChange(
        "Thang thu phi", old->getBilling_month()
    );

    string status = keepOrChange(
        "Trang thai (Paid/Unpaid)",
        old->getInvoice_status()
    );

    long long amount = old->getTotal_amount();

    if (customerId != old->getCustomer_id()) {
        amount = state.fees.getFee(
            c->getCustomer_type(),
            c->getService_area()
        );

        cout << "Doi KH: so tien mac dinh theo bang gia moi: "
             << amount << " VND\n";
    }

    string changedAmount = input(
        "So tien moi (Enter de giu so tien tren): "
    );

    if (!changedAmount.empty()) {
        amount = parsePositiveAmount(changedAmount);
    }

    Invoice changed(
        id, customerId, date, amount, month, status
    );

    unique_ptr<InvoiceManager> next(
        new InvoiceManager(*state.invoices)
    );

    if (!next->deleteInvoice(id)) {
        throw runtime_error(
            "Khong xoa duoc HD tren ban sao"
        );
    }

    next->addInvoice(changed);

    saveInvoices(*next);
    state.invoices.swap(next);

    cout << "Da sua hoa don va luu TXT.\n";
}

void deleteInvoice(DemoState& state) {
    string id = required("Ma HD can xoa: ", "Ma HD");

    unique_ptr<InvoiceManager> next(
        new InvoiceManager(*state.invoices)
    );

    if (!next->deleteInvoice(id)) {
        cout << "Khong tim thay hoa don.\n";
        return;
    }

    saveInvoices(*next);
    state.invoices.swap(next);

    cout << "Da xoa hoa don va luu TXT.\n";
}

void payInvoice(DemoState& state) {
    string id = required(
        "Ma HD can thanh toan: ", "Ma HD"
    );

    unique_ptr<InvoiceManager> next(
        new InvoiceManager(*state.invoices)
    );

    if (!next->markAsPaid(id)) {
        cout << "HD khong ton tai hoac da thanh toan.\n";
        return;
    }

    saveInvoices(*next);
    state.invoices.swap(next);

    cout << "Da ghi nhan thanh toan va luu TXT.\n";
}

// ==================== BAO CAO THONG KE ====================

void report(const DemoState& state) {
    long long expected = 0;

    for (int i = 0; i < state.customers->getSize(); ++i) {
        const Customer& c = (*state.customers)[i];

        long long fee = state.fees.getFee(
            c.getCustomer_type(),
            c.getService_area()
        );

        if (expected >
            numeric_limits<long long>::max() - fee) {
            throw overflow_error(
                "Tong phi du kien qua lon"
            );
        }

        expected += fee;
    }

    long long total =
        state.invoices->calculateTotalInvoiced();

    long long paid =
        state.invoices->calculateCollectedRevenue();

    cout << "\n===== THONG KE =====\n"
         << "So khach hang: "
         << state.customers->getSize() << '\n'
         << "So hoa don: "
         << state.invoices->getSize() << '\n'
         << "Tong phi du kien/thang: "
         << expected << " VND\n"
         << "Tong gia tri hoa don: "
         << total << " VND\n"
         << "Da thu: " << paid << " VND\n"
         << "Chua thu: "
         << (total - paid) << " VND\n";
}

// ==================== MENU ====================

void printMenu() {
    cout << "\n======= DEMO CUSTOMER + INVOICE =======\n"
         << " 1. Danh sach KH       2. Tim/Xem KH\n"
         << " 3. Them KH            4. Sua KH\n"
         << " 5. Xoa KH             6. Danh sach HD\n"
         << " 7. Tim/Xem HD         8. HD cua mot KH\n"
         << " 9. Tao HD            10. Sua HD\n"
         << "11. Xoa HD            12. Thanh toan HD\n"
         << "13. HD chua thanh toan\n"
         << "14. Bao cao tong hop\n"
         << " 0. Thoat\n"
         << "=======================================\n";
}

void runMenu(DemoState& state) {
    while (true) {
        printMenu();

        int choice = menuChoice();
        if (choice == 0) return;

        try {
            switch (choice) {
            case 1:
                state.customers->displayAll();
                break;
            case 2:
                showCustomer(state);
                break;
            case 3:
                addCustomer(state);
                break;
            case 4:
                editCustomer(state);
                break;
            case 5:
                deleteCustomer(state);
                break;
            case 6:
                state.invoices->displayAll();
                break;
            case 7:
                showInvoice(state);
                break;
            case 8:
                invoicesOfCustomer(state);
                break;
            case 9:
                createInvoice(state);
                break;
            case 10:
                editInvoice(state);
                break;
            case 11:
                deleteInvoice(state);
                break;
            case 12:
                payInvoice(state);
                break;
            case 13:
                state.invoices->displayUnpaidInvoices();
                break;
            case 14:
                report(state);
                break;
            default:
                cout << "Lua chon khong hop le.\n";
            }
        }
        catch (const exception& e) {
            cerr << "[LOI] " << e.what() << '\n';
        }
    }
}

} // namespace

// ==================== MAIN DEMO ====================

int main(int argc, char* argv[]) {
    try {
        DemoState state;

        prepareDemoFiles(state.fees);

        loadCustomers(
            *state.customers,
            CUSTOMER_PATH,
            state.fees
        );

        state.invoices->loadFromFile(INVOICE_PATH);

        checkRelations(
            *state.customers,
            *state.invoices
        );

        cout << "Da doc "
             << state.customers->getSize() << " KH, "
             << state.invoices->getSize()
             << " HD tu file TXT demo.\n";

        if (argc > 1 && string(argv[1]) == "--check") {
            report(state);
            cout << "[PASS] Doc TXT va kiem tra "
                 << "lien ket thanh cong.\n";
            return 0;
        }

        runMenu(state);
        return 0;
    }
    catch (const exception& e) {
        cerr << "[LOI KHOI DONG] "
             << e.what() << '\n';
        return 1;
    }
}
