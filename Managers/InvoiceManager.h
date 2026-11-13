
#pragma once

#include "../Objects/Invoice.h"
#include <string>

using namespace std;

class InvoiceManager {
private:
    Invoice* arr;
    int size;
    int capacity;

public:
    // Constructor va Destructor
    InvoiceManager();
    InvoiceManager(const InvoiceManager& other);
    InvoiceManager& operator=(const InvoiceManager& other);
    ~InvoiceManager();

    // Getter
    int getSize() const;
    int getCapacity() const;
    const Invoice* getArr() const;

    // Nap chong toan tu
    InvoiceManager& operator+=(const Invoice& invoice);
    InvoiceManager& operator-=(const string& invoiceId);
    const Invoice& operator[](int index) const;

    // Quan ly hoa don
    void addInvoice(const Invoice& invoice);
    bool markAsPaid(const string& invoiceId);
    bool deleteInvoice(const string& invoiceId);

    // Tim kiem
    const Invoice* findInvoiceById(
        const string& invoiceId
    ) const;

    // Kiem tra
    bool hasInvoicesForCustomer(
        const string& customerId
    ) const;

    bool hasInvoiceForCustomerInMonth(
        const string& customerId,
        const string& billingMonth
    ) const;

    // Hien thi
    void displayAll() const;
    void displayUnpaidInvoices() const;

    // Thong ke
    long long calculateTotalInvoiced() const;
    long long calculateCollectedRevenue() const;

    // Doc ghi file
    void loadFromFile(const string& filePath);
    void saveToFile(const string& filePath) const;
};
