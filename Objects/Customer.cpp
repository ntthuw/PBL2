

#include "Customer.h"
#include <iomanip>
#include <string>
#include <stdexcept>
#include <cctype>


// Constructor
Customer::Customer()
    : customer_id(""),
    customer_type(""),
    customer_name(""),
    customer_phone(""),
    customer_address(""),
    service_area("") {


}


Customer::Customer(
    string customer_id,
    string customer_type,
    string customer_name,
    string customer_phone,
    string customer_address,
    string service_area
) : customer_id(""),
    customer_type(""),
    customer_name(""),
    customer_phone(""),
    customer_address(""),
    service_area("") {


    setCustomer_id(customer_id);
    setCustomer_type(customer_type);
    setCustomer_name(customer_name);
    setCustomer_phone(customer_phone);
    setCustomer_address(customer_address);
    setService_area(service_area);


    validate();
}


// Ham get
string Customer::getCustomer_id() const {
    return customer_id;
}


string Customer::getCustomer_type() const {
    return customer_type;
}


string Customer::getCustomer_name() const {
    return customer_name;
}


string Customer::getCustomer_phone() const {
    return customer_phone;
}


string Customer::getCustomer_address() const {
    return customer_address;
}


string Customer::getService_area() const {
    return service_area;
}


// Ham set
void Customer::setCustomer_id(const string& customer_id) {
    if (customer_id.empty()) {
        throw invalid_argument("Customer ID cannot be empty!");
    }


    this->customer_id = customer_id;
}


void Customer::setCustomer_type(const string& customer_type) {
    if (customer_type != "Household" &&
        customer_type != "Business") {
        throw invalid_argument("Invalid customer type!");
    }


    this->customer_type = customer_type;
}


void Customer::setCustomer_name(const string& customer_name) {
    if (customer_name.empty()) {
        throw invalid_argument("Customer name cannot be empty!");
    }


    this->customer_name = customer_name;
}


void Customer::setCustomer_phone(const string& customer_phone) {
    if (customer_phone.length() < 10 ||
        customer_phone.length() > 11) {
        throw invalid_argument("Phone number must have 10-11 digits!");
    }


    for (int i = 0; i < customer_phone.length(); i++) {
        if (!isdigit(static_cast<unsigned char>(customer_phone[i]))) {
            throw invalid_argument("Phone number must contain digits only!");
        }
    }


    this->customer_phone = customer_phone;
}


void Customer::setCustomer_address(const string& customer_address) {
    this->customer_address = customer_address;
}


void Customer::setService_area(const string& service_area) {
    if (service_area != "Area1" &&
        service_area != "Area2" &&
        service_area != "Type1" &&
        service_area != "Type2") {
        throw invalid_argument("Invalid service area!");
    }


    this->service_area = service_area;
}


// Ham kiem tra du lieu
void Customer::validate() const {
    if (customer_id.empty()) {
        throw invalid_argument("Customer ID cannot be empty!");
    }


    if (customer_name.empty()) {
        throw invalid_argument("Customer name cannot be empty!");
    }


    if (customer_type == "Household") {
        if (service_area != "Area1" &&
            service_area != "Area2") {
            throw invalid_argument(
                "Household must use Area1 or Area2!"
            );
        }
    }
    else if (customer_type == "Business") {
        if (service_area != "Type1" &&
            service_area != "Type2") {
            throw invalid_argument(
                "Business must use Type1 or Type2!"
            );
        }
    }
    else {
        throw invalid_argument("Invalid customer type!");
    }


    if (customer_phone.length() < 10 ||
        customer_phone.length() > 11) {
        throw invalid_argument("Phone number must have 10-11 digits!");
    }


    for (int i = 0; i < customer_phone.length(); i++) {
        if (!isdigit(static_cast<unsigned char>(customer_phone[i]))) {
            throw invalid_argument("Phone number must contain digits only!");
        }
    }
}


// Ham show
void Customer::Show() const {
    cout << left
         << setw(12) << customer_id
         << setw(15) << customer_type
         << setw(22) << customer_name
         << setw(15) << customer_phone
         << setw(25) << customer_address
         << setw(12) << service_area
         << '\n';
}



