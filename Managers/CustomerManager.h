

#pragma once


#include "Customer.h"
#include <string>


using namespace std;


class CustomerManager {
private:
    Customer* arr;
    int size;
    int capacity;


public:
    // Constructor
    CustomerManager();


    // Copy constructor
    CustomerManager(const CustomerManager& other);


    // Toan tu gan
    CustomerManager& operator=(const CustomerManager& other);


    // Destructor
    ~CustomerManager();


    // Getter
    int getSize() const;
    int getCapacity() const;


    Customer* getArr();
    const Customer* getArr() const;


    // Nap chong toan tu
    CustomerManager& operator+=(const Customer& customer);
    CustomerManager& operator-=(const string& id);


    Customer& operator[](int index);
    const Customer& operator[](int index) const;


    // Ham tim kiem
    Customer* findCustomerById(const string& id);
    const Customer* findCustomerById(const string& id) const;


    // Ham quan ly khach hang
    void addCustomer(const Customer& customer);
    bool editCustomer(const string& id, const Customer& updatedCustomer);
    bool deleteCustomer(const string& id);


    // Ham hien thi
    void displayAll() const;
};



