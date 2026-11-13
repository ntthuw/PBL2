
#include "InvoiceManager.h"
#include "TemplateUtils.h"
#include "../FileIO/FileManager.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <memory>
#include <limits>
#include <utility>

using namespace std;

// ==================== HAM HO TRO ====================

namespace {

    bool hasSeparator(const string& value) {
        return value.find_first_of("|\r\n") != string::npos;
    }

    void validateInvoice(const Invoice& invoice) {
        Invoice temp;

        temp.setInvoice_id(invoice.getInvoice_id());
        temp.setCustomer_id(invoice.getCustomer_id());
        temp.setInvoice_date(invoice.getInvoice_date());
        temp.setInvoice_status(invoice.getInvoice_status());
        temp.setTotal_amount(invoice.getTotal_amount());
        temp.setBilling_month(invoice.getBilling_month());

        if (hasSeparator(invoice.getInvoice_id()) ||
            hasSeparator(invoice.getCustomer_id()) ||
            hasSeparator(invoice.getInvoice_date()) ||
            hasSeparator(invoice.getInvoice_status()) ||
            hasSeparator(invoice.getBilling_month())) {
            throw invalid_argument(
                "Du lieu hoa don chua ky tu phan cach"
            );
        }
    }

    string invoiceToLine(const Invoice& invoice) {
        return invoice.getInvoice_id() + "|" +
               invoice.getCustomer_id() + "|" +
               invoice.getInvoice_date() + "|" +
               invoice.getInvoice_status() + "|" +
               to_string(invoice.getTotal_amount()) + "|" +
               invoice.getBilling_month();
    }

    Invoice lineToInvoice(const string& line) {
        if (count(line.begin(), line.end(), '|') != 5) {
            throw invalid_argument(
                "Dong hoa don phai co dung 6 truong"
            );
        }

        string fields[6];
        stringstream ss(line);

        for (int i = 0; i < 6; i++) {
            if (!getline(ss, fields[i], '|')) {
                throw invalid_argument(
                    "Dong hoa don thieu truong"
                );
            }
        }

        if (fields[4].empty()) {
            throw invalid_argument("So tien rong");
        }

        size_t pos = 0;
        long long amount = stoll(fields[4], &pos);

        if (pos != fields[4].size()) {
            throw invalid_argument(
                "So tien khong hop le"
            );
        }

        return Invoice(
            fields[0],
            fields[1],
            fields[2],
            amount,
            fields[5],
            fields[3]
        );
    }
}

// ==================== CONSTRUCTOR ====================

InvoiceManager::InvoiceManager()
    : arr(nullptr), size(0), capacity(0)
{
}

// ==================== COPY CONSTRUCTOR ====================

InvoiceManager::InvoiceManager(const InvoiceManager& other)
    : arr(nullptr), size(0), capacity(0)
{
    if (other.capacity > 0) {
        unique_ptr<Invoice[]> temp(
            new Invoice[other.capacity]
        );

        for (int i = 0; i < other.size; i++) {
            temp[i] = other.arr[i];
        }

        arr = temp.release();
    }

    size = other.size;
    capacity = other.capacity;
}

// ==================== TOAN TU GAN ====================

InvoiceManager& InvoiceManager::operator=(
    const InvoiceManager& other)
{
    if (this != &other) {
        InvoiceManager temp(other);

        swap(arr, temp.arr);
        swap(size, temp.size);
        swap(capacity, temp.capacity);
    }

    return *this;
}

// ==================== DESTRUCTOR ====================

InvoiceManager::~InvoiceManager() {
    delete[] arr;
}

// ==================== GETTER ====================

int InvoiceManager::getSize() const {
    return size;
}

int InvoiceManager::getCapacity() const {
    return capacity;
}

const Invoice* InvoiceManager::getArr() const {
    return arr;
}

// ==================== NAP CHONG TOAN TU ====================

InvoiceManager& InvoiceManager::operator+=(
    const Invoice& invoice)
{
    addInvoice(invoice);
    return *this;
}

InvoiceManager& InvoiceManager::operator-=(
    const string& invoiceId)
{
    deleteInvoice(invoiceId);
    return *this;
}

const Invoice& InvoiceManager::operator[](int index) const {
    if (index < 0 || index >= size) {
        throw out_of_range(
            "Chi so hoa don khong hop le"
        );
    }

    return arr[index];
}

// ==================== THEM HOA DON ====================

void InvoiceManager::addInvoice(const Invoice& invoice) {
    // Kiem tra du lieu
    validateInvoice(invoice);

    // Kiem tra trung ma
    if (findInvoiceById(invoice.getInvoice_id()) != nullptr) {
        throw invalid_argument(
            "Ma hoa don da ton tai"
        );
    }

    // Kiem tra trung khach hang va thang
    if (hasInvoiceForCustomerInMonth(
            invoice.getCustomer_id(),
            invoice.getBilling_month())) {
        throw invalid_argument(
            "Khach hang da co hoa don trong thang nay"
        );
    }

    // Mo rong mang neu can
    if (size == capacity) {
        if (capacity > numeric_limits<int>::max() / 2) {
            throw length_error(
                "Dung luong mang qua lon"
            );
        }

        int newCapacity = (capacity == 0)
                          ? 2 : capacity * 2;

        unique_ptr<Invoice[]> temp(
            new Invoice[newCapacity]
        );

        // Sao chep du lieu cu
        for (int i = 0; i < size; i++) {
            temp[i] = arr[i];
        }

        // Them hoa don moi
        temp[size] = invoice;

        // Thay mang cu
        delete[] arr;
        arr = temp.release();
        capacity = newCapacity;
    }
    else {
        arr[size] = invoice;
    }

    size++;
}

// ==================== THANH TOAN ====================

bool InvoiceManager::markAsPaid(
    const string& invoiceId)
{
    for (int i = 0; i < size; i++) {
        if (arr[i].getInvoice_id() == invoiceId) {
            if (arr[i].getInvoice_status() != "Unpaid") {
                return false;
            }

            arr[i].setInvoice_status("Paid");
            return true;
        }
    }

    return false;
}

// ==================== XOA HOA DON ====================

bool InvoiceManager::deleteInvoice(
    const string& invoiceId)
{
    int index = -1;

    for (int i = 0; i < size; i++) {
        if (arr[i].getInvoice_id() == invoiceId) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return false;
    }

    // Tao mang moi de tranh mat du lieu neu co loi
    unique_ptr<Invoice[]> temp(
        new Invoice[capacity]
    );

    int j = 0;

    for (int i = 0; i < size; i++) {
        if (i != index) {
            temp[j] = arr[i];
            j++;
        }
    }

    delete[] arr;
    arr = temp.release();
    size--;

    return true;
}

// ==================== TIM KIEM ====================

const Invoice* InvoiceManager::findInvoiceById(
    const string& invoiceId) const
{
    return findIf(
        arr,
        size,
        [&](const Invoice& invoice) {
            return invoice.getInvoice_id() == invoiceId;
        }
    );
}

// ==================== KIEM TRA ====================

bool InvoiceManager::hasInvoicesForCustomer(
    const string& customerId) const
{
    return findIf(
        arr,
        size,
        [&](const Invoice& invoice) {
            return invoice.getCustomer_id() == customerId;
        }
    ) != nullptr;
}

bool InvoiceManager::hasInvoiceForCustomerInMonth(
    const string& customerId,
    const string& billingMonth) const
{
    return findIf(
        arr,
        size,
        [&](const Invoice& invoice) {
            return invoice.getCustomer_id() == customerId &&
                   invoice.getBilling_month() == billingMonth;
        }
    ) != nullptr;
}

// ==================== HIEN THI ====================

void InvoiceManager::displayAll() const {
    if (size == 0) {
        cout << "Danh sach hoa don rong!" << endl;
        return;
    }

    cout << "\n===== DANH SACH HOA DON =====" << endl;

    for (int i = 0; i < size; i++) {
        arr[i].Show();
    }
}

void InvoiceManager::displayUnpaidInvoices() const {
    bool found = false;

    cout << "\n===== HOA DON CHUA THANH TOAN =====" << endl;

    for (int i = 0; i < size; i++) {
        if (arr[i].getInvoice_status() == "Unpaid") {
            arr[i].Show();
            found = true;
        }
    }

    if (!found) {
        cout << "Khong co hoa don chua thanh toan!" << endl;
    }
}

// ==================== THONG KE ====================

long long InvoiceManager::calculateTotalInvoiced() const {
    long long total = 0;

    for (int i = 0; i < size; i++) {
        long long amount = arr[i].getTotal_amount();

        if (amount >
            numeric_limits<long long>::max() - total) {
            throw overflow_error(
                "Tong tien vuot gioi han"
            );
        }

        total += amount;
    }

    return total;
}

long long InvoiceManager::calculateCollectedRevenue() const {
    long long total = 0;

    for (int i = 0; i < size; i++) {
        if (arr[i].getInvoice_status() == "Paid") {
            long long amount = arr[i].getTotal_amount();

            if (amount >
                numeric_limits<long long>::max() - total) {
                throw overflow_error(
                    "Doanh thu vuot gioi han"
                );
            }

            total += amount;
        }
    }

    return total;
}

// ==================== DOC FILE ====================

void InvoiceManager::loadFromFile(
    const string& filePath)
{
    FileManager& fm = FileManager::getInstance();

    if (!fm.fileExists(filePath)) {
        InvoiceManager empty;
        *this = empty;
        return;
    }

    vector<string> lines = fm.readFile(filePath);

    InvoiceManager temp;

    for (size_t i = 0; i < lines.size(); i++) {
        try {
            if (lines[i].empty()) {
                throw invalid_argument(
                    "Dong du lieu rong"
                );
            }

            Invoice invoice = lineToInvoice(lines[i]);
            temp.addInvoice(invoice);
        }
        catch (const exception& e) {
            throw runtime_error(
                "Loi du lieu hoa don dong " +
                to_string(i + 1) + ": " + e.what()
            );
        }
    }

    *this = temp;
}

// ==================== GHI FILE ====================

void InvoiceManager::saveToFile(
    const string& filePath) const
{
    vector<string> lines;

    for (int i = 0; i < size; i++) {
        validateInvoice(arr[i]);
        lines.push_back(invoiceToLine(arr[i]));
    }

    FileManager::getInstance().writeFile(
        filePath, lines
    );
}
