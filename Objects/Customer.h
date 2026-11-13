#pragma once


#include <iostream>
#include <string>


using namespace std;


class Customer {
private:
    string customer_id;
    string customer_type;
    string customer_name;
    string customer_phone;
    string customer_address;
    string service_area;


public:
    // Constructor
    Customer();


    Customer(
        string customer_id,
        string customer_type,
        string customer_name,
        string customer_phone,
        string customer_address,
        string service_area
    );


    // Getter
    string getCustomer_id() const;
    string getCustomer_type() const;
    string getCustomer_name() const;
    string getCustomer_phone() const;
    string getCustomer_address() const;
    string getService_area() const;


    // Setter
    void setCustomer_id(const string& customer_id);
    void setCustomer_type(const string& customer_type);
    void setCustomer_name(const string& customer_name);
    void setCustomer_phone(const string& customer_phone);
    void setCustomer_address(const string& customer_address);
    void setService_area(const string& service_area);


    // Ham kiem tra du lieu
    void validate() const;


    // Ham show
    void Show() const;
};



