
#pragma once
#include <string>
using namespace std;

class Invoice {
private:
    string invoice_id;
    string customer_id;
    string invoice_date;
    string invoice_status;
    long long total_amount;
    string billing_month;

public:
    Invoice();

    Invoice(const string& invoice_id,
            const string& customer_id,
            const string& invoice_date,
            long long total_amount,
            const string& billing_month,
            const string& invoice_status = "Unpaid");

    string getInvoice_id() const;
    void setInvoice_id(const string& invoice_id);

    string getCustomer_id() const;
    void setCustomer_id(const string& customer_id);

    string getInvoice_date() const;
    void setInvoice_date(const string& invoice_date);

    string getInvoice_status() const;
    void setInvoice_status(const string& invoice_status);

    long long getTotal_amount() const;
    void setTotal_amount(long long total_amount);

    string getBilling_month() const;
    void setBilling_month(const string& billing_month);

    void Show() const;
};
  