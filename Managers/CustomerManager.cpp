

#include "CustomerManager.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>


using namespace std;


//constructor
CustomerManager::CustomerManager() {
    size = 0;
    capacity = 5;
    arr = new Customer[capacity];
}


//copy constructor
CustomerManager::CustomerManager(const CustomerManager& other) {
    size = other.size;
    capacity = other.capacity;


    arr = new Customer[capacity];


    for (int i = 0; i < size; i++) {
        arr[i] = other.arr[i];
    }
}


//toan tu gan
CustomerManager& CustomerManager::operator=(const CustomerManager& other) {
    if (this != &other) {
        Customer* newArr = new Customer[other.capacity];


        for (int i = 0; i < other.size; i++) {
            newArr[i] = other.arr[i];
        }


        delete[] arr;


        arr = newArr;
        size = other.size;
        capacity = other.capacity;
    }


    return *this;
}


//destructor
CustomerManager::~CustomerManager() {
    delete[] arr;
    arr = nullptr;
    size = 0;
    capacity = 0;
}


//Getter
int CustomerManager::getSize() const {
    return size;
}


int CustomerManager::getCapacity() const {
    return capacity;
}


Customer* CustomerManager::getArr() {
    return arr;
}


const Customer* CustomerManager::getArr() const {
    return arr;
}


//Nap chong toan tu
CustomerManager& CustomerManager::operator+=(const Customer& customer) {
    addCustomer(customer);
    return *this;
}


CustomerManager& CustomerManager::operator-=(const string& id) {
    deleteCustomer(id);
    return *this;
}


Customer& CustomerManager::operator[](int index) {
    if (index < 0 || index >= size) {
        throw out_of_range("Customer index out of range!");
    }


    return arr[index];
}


const Customer& CustomerManager::operator[](int index) const {
    if (index < 0 || index >= size) {
        throw out_of_range("Customer index out of range!");
    }


    return arr[index];
}


//Ham tim kiem
Customer* CustomerManager::findCustomerById(const string& id) {
    for (int i = 0; i < size; i++) {
        if (arr[i].getCustomer_id() == id) {
            return &arr[i];
        }
    }


    return nullptr;
}


const Customer* CustomerManager::findCustomerById(const string& id) const {
    for (int i = 0; i < size; i++) {
        if (arr[i].getCustomer_id() == id) {
            return &arr[i];
        }
    }


    return nullptr;
}


//Ham quan ly khach hang


//1. Ham add
void CustomerManager::addCustomer(const Customer& customer) {
    //Kiem tra du lieu khach hang
    customer.validate();


    //Kiem tra ID trung
    if (findCustomerById(customer.getCustomer_id()) != nullptr) {
        throw invalid_argument("Customer ID already exists!");
    }


    if (size == capacity) {
        int newCapacity = capacity * 2;
        Customer* newArr = new Customer[newCapacity];


        for (int i = 0; i < size; i++) {
            newArr[i] = arr[i];
        }


        delete[] arr;
        arr = newArr;
        capacity = newCapacity;
    }


    arr[size] = customer;
    size++;
}


//2. Ham edit
bool CustomerManager::editCustomer(
    const string& id,
    const Customer& updatedCustomer
) {
    //Kiem tra du lieu khach hang
    updatedCustomer.validate();


    Customer* customer = findCustomerById(id);


    if (customer == nullptr) {
        return false;
    }


    //khong cho phep thay doi ID
    if (id != updatedCustomer.getCustomer_id()) {
        return false;
    }


    *customer = updatedCustomer;


    return true;
}


//3. Ham delete
bool CustomerManager::deleteCustomer(const string& id) {
    int index = -1;


    for (int i = 0; i < size; i++) {
        if (arr[i].getCustomer_id() == id) {
            index = i;
            break;
        }
    }


    if (index == -1) {
        return false;
    }


    for (int i = index; i < size - 1; i++) {
        arr[i] = arr[i + 1];
    }


    size--;


    return true;
}


//Ham hien thi
void CustomerManager::displayAll() const {
    if (size == 0) {
        cout << "\nDanh sach khach hang rong!\n";
        return;
    }


    cout << "\n";
    cout << left
         << setw(12) << "Ma KH"
         << setw(15) << "Loai KH"
         << setw(22) << "Ho ten"
         << setw(15) << "So dien thoai"
         << setw(25) << "Dia chi"
         << setw(12) << "Khu vuc"
         << '\n';


    cout << string(101, '-') << '\n';


    for (int i = 0; i < size; i++) {
        arr[i].Show();
    }
}



