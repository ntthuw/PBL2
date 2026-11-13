
#include "Invoice.h"
#include <iostream>
#include <stdexcept>
using namespace std;

// Kiem tra ngay theo dinh dang YYYY-MM-DD
namespace {
    bool isValidDate(const string& date) {
        if (date.length() != 10 ||
            date[4] != '-' || date[7] != '-') {
            return false;
        }

        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7) continue;

            if (date[i] < '0' || date[i] > '9') {
                return false;
            }
        }

        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));

        if (year < 1 || month < 1 || month > 12) {
            return false;
        }

        int daysInMonth[] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        bool leapYear = (year % 400 == 0) ||
                        (year % 4 == 0 && year % 100 != 0);

        if (leapYear) {
            daysInMonth[1] = 29;
        }

        return day >= 1 && day <= daysInMonth[month - 1];
    }

    // Kiem tra thang theo dinh dang YYYY-MM
    bool isValidMonth(const string& month) {
        if (month.length() != 7 || month[4] != '-') {
            return false;
        }

        for (int i = 0; i < 7; i++) {
            if (i == 4) continue;

            if (month[i] < '0' || month[i] > '9') {
                return false;
            }
        }

        int year = stoi(month.substr(0, 4));
        int m = stoi(month.substr(5, 2));

        return year >= 1 && m >= 1 && m <= 12;
    }
}

// Constructor mac dinh
Invoice::Invoice()
    : invoice_id(""),
      customer_id(""),
      invoice_date(""),
      invoice_status("Unpaid"),
      total_amount(0),
      billing_month("")
{
}

// Constructor co tham so
Invoice::Invoice(const string& invoice_id,
                 const string& customer_id,
                 const string& invoice_date,
                 long long total_amount,
                 const string& billing_month,
                 const string& invoice_status)
    : Invoice()
{
    setInvoice_id(invoice_id);
    setCustomer_id(customer_id);
    setInvoice_date(invoice_date);
    setTotal_amount(total_amount);
    setBilling_month(billing_month);
    setInvoice_status(invoice_status);
}

// Getter va Setter cho invoice_id
string Invoice::getInvoice_id() const {
    return invoice_id;
}

void Invoice::setInvoice_id(const string& invoice_id) {
    if (invoice_id.empty()) {
        throw invalid_argument("Ma hoa don khong duoc rong");
    }

    this->invoice_id = invoice_id;
}

// Getter va Setter cho customer_id
string Invoice::getCustomer_id() const {
    return customer_id;
}

void Invoice::setCustomer_id(const string& customer_id) {
    if (customer_id.empty()) {
        throw invalid_argument("Ma khach hang khong duoc rong");
    }

    this->customer_id = customer_id;
}

// Getter va Setter cho invoice_date
string Invoice::getInvoice_date() const {
    return invoice_date;
}

void Invoice::setInvoice_date(const string& invoice_date) {
    if (!isValidDate(invoice_date)) {
        throw invalid_argument("Ngay lap hoa don khong hop le");
    }

    this->invoice_date = invoice_date;
}

// Getter va Setter cho invoice_status
string Invoice::getInvoice_status() const {
    return invoice_status;
}

void Invoice::setInvoice_status(const string& invoice_status) {
    if (invoice_status != "Paid" &&
        invoice_status != "Unpaid") {
        throw invalid_argument("Trang thai hoa don khong hop le");
    }

    this->invoice_status = invoice_status;
}

// Getter va Setter cho total_amount
long long Invoice::getTotal_amount() const {
    return total_amount;
}

void Invoice::setTotal_amount(long long total_amount) {
    if (total_amount < 0) {
        throw invalid_argument("Tong tien khong duoc am");
    }

    this->total_amount = total_amount;
}

// Getter va Setter cho billing_month
string Invoice::getBilling_month() const {
    return billing_month;
}

void Invoice::setBilling_month(const string& billing_month) {
    if (!isValidMonth(billing_month)) {
        throw invalid_argument("Thang thu phi khong hop le");
    }

    this->billing_month = billing_month;
}

// Hien thi thong tin hoa don
void Invoice::Show() const {
    cout << "\n===== THONG TIN HOA DON =====" << endl;
    cout << "Ma hoa don: " << invoice_id << endl;
    cout << "Ma khach hang: " << customer_id << endl;
    cout << "Ngay lap: " << invoice_date << endl;
    cout << "Trang thai: " << invoice_status << endl;
    cout << "Tong tien: " << total_amount << " VND" << endl;
    cout << "Thang thu phi: " << billing_month << endl;
}
