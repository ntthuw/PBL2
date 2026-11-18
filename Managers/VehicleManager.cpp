
#include "VehicleManager.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>

using namespace std;

// Constructor
VehicleManager::VehicleManager() {
    size = 0;
    capacity = 5;
    arr = new Vehicle[capacity];
}

// Copy constructor
VehicleManager::VehicleManager(const VehicleManager& other) {
    size = other.size;
    capacity = other.capacity;
    arr = new Vehicle[capacity];

    try {
        for (int i = 0; i < size; i++) {
            arr[i] = other.arr[i];
        }
    }
    catch (...) {
        delete[] arr;
        throw;
    }
}

// Toan tu gan
VehicleManager& VehicleManager::operator=(const VehicleManager& other) {
    if (this == &other) {
        return *this;
    }

    Vehicle* newArr = new Vehicle[other.capacity];

    try {
        for (int i = 0; i < other.size; i++) {
            newArr[i] = other.arr[i];
        }
    }
    catch (...) {
        delete[] newArr;
        throw;
    }

    delete[] arr;

    arr = newArr;
    size = other.size;
    capacity = other.capacity;

    return *this;
}

// Destructor
VehicleManager::~VehicleManager() {
    delete[] arr;
    arr = nullptr;
    size = 0;
    capacity = 0;
}

// Getter
int VehicleManager::getSize() const {
    return size;
}

int VehicleManager::getCapacity() const {
    return capacity;
}

Vehicle* VehicleManager::getArr() {
    return arr;
}

const Vehicle* VehicleManager::getArr() const {
    return arr;
}

// Nap chong toan tu
VehicleManager& VehicleManager::operator+=(const Vehicle& vehicle) {
    addVehicle(vehicle);
    return *this;
}

VehicleManager& VehicleManager::operator-=(const string& id) {
    deleteVehicle(id);
    return *this;
}

Vehicle& VehicleManager::operator[](int index) {
    if (index < 0 || index >= size) {
        throw out_of_range("Vehicle index is out of range!");
    }

    return arr[index];
}

const Vehicle& VehicleManager::operator[](int index) const {
    if (index < 0 || index >= size) {
        throw out_of_range("Vehicle index is out of range!");
    }

    return arr[index];
}

// Ham tim kiem
Vehicle* VehicleManager::findVehicleById(const string& id) {
    for (int i = 0; i < size; i++) {
        if (arr[i].getVehicle_id() == id) {
            return &arr[i];
        }
    }

    return nullptr;
}

const Vehicle* VehicleManager::findVehicleById(const string& id) const {
    for (int i = 0; i < size; i++) {
        if (arr[i].getVehicle_id() == id) {
            return &arr[i];
        }
    }

    return nullptr;
}

// Ham quan ly phuong tien

// 1. Ham add
void VehicleManager::addVehicle(const Vehicle& vehicle) {
    // Kiem tra toan bo du lieu
    vehicle.validate();

    // Kiem tra ID trung
    if (findVehicleById(vehicle.getVehicle_id()) != nullptr) {
        throw invalid_argument("Vehicle ID already exists!");
    }

    // Mo rong mang khi day
    if (size == capacity) {
        int newCapacity = capacity * 2;
        Vehicle* newArr = new Vehicle[newCapacity];

        try {
            for (int i = 0; i < size; i++) {
                newArr[i] = arr[i];
            }
        }
        catch (...) {
            delete[] newArr;
            throw;
        }

        delete[] arr;
        arr = newArr;
        capacity = newCapacity;
    }

    arr[size] = vehicle;
    size++;
}

// 2. Edit
bool VehicleManager::editVehicle(
    const string& id,
    const Vehicle& updatedVehicle
) {
    Vehicle* vehicle = findVehicleById(id);

    if (vehicle == nullptr) {
        return false;
    }

    // Khong cho phep thay doi ID
    if (updatedVehicle.getVehicle_id() != id) {
        return false;
    }

    // Kiem tra toan bo du lieu moi truoc khi cap nhat
    updatedVehicle.validate();

    // Cap nhat thong tin
    *vehicle = updatedVehicle;

    return true;
}

// 3. Delete
bool VehicleManager::deleteVehicle(const string& id) {
    int index = -1;

    for (int i = 0; i < size; i++) {
        if (arr[i].getVehicle_id() == id) {
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

// Ham display
void VehicleManager::displayAll() const {
    if (size == 0) {
        cout << "\nVehicle list is empty!" << endl;
        return;
    }

    cout << "\n========== VEHICLE LIST ==========" << endl;

    cout << left
         << setw(15) << "Vehicle ID"
         << setw(18) << "Status"
         << setw(18) << "Capacity"
         << setw(15) << "Kilometer"
         << '\n';

    cout << string(66, '-') << '\n';

    for (int i = 0; i < size; i++) {
        arr[i].Show();
    }

    cout << string(66, '-') << '\n';
    cout << "Total vehicles: " << size << '\n';
}

void VehicleManager::displayByStatus(const string& status) const {
    // Kiem tra trang thai dau vao
    if (status != "Available" &&
        status != "InUse" &&
        status != "Maintenance") {

        throw invalid_argument("Invalid vehicle status!");
    }

    bool found = false;

    cout << "\nVehicles with status: " << status << endl;

    cout << left
         << setw(15) << "Vehicle ID"
         << setw(18) << "Status"
         << setw(18) << "Capacity"
         << setw(15) << "Kilometer"
         << '\n';

    cout << string(66, '-') << '\n';

    for (int i = 0; i < size; i++) {
        if (arr[i].getVehicle_status() == status) {
            arr[i].Show();
            found = true;
        }
    }

    if (!found) {
        cout << "No vehicle found with this status!" << endl;
    }

    cout << string(66, '-') << '\n';
}
