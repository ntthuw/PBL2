#pragma once

#include "Vehicle.h"
#include <string>

using namespace std;

class VehicleManager {
private:
    Vehicle* arr;
    int size;
    int capacity;

public:
    VehicleManager();
    VehicleManager(const VehicleManager& other);
    VehicleManager& operator=(const VehicleManager& other);
    ~VehicleManager();

    int getSize() const;
    int getCapacity() const;

    Vehicle* getArr();
    const Vehicle* getArr() const;

    // Nap chong toan tu
    VehicleManager& operator+=(const Vehicle& vehicle);     // Them phuong tien
    VehicleManager& operator-=(const string& id);           // Xoa phuong tien theo ID
    Vehicle& operator[](int index);                        // Truy xuat theo index
    const Vehicle& operator[](int index) const;

    // Quan ly phuong tien
    void addVehicle(const Vehicle& vehicle);
    bool editVehicle(const string& id, const Vehicle& updatedVehicle);
    bool deleteVehicle(const string& id);

    // Tim kiem va hien thi
    Vehicle* findVehicleById(const string& id);
    const Vehicle* findVehicleById(const string& id) const;
    void displayAll() const;

    // Loc va hien thi theo trang thai (San sang / Dang hoat dong / Bao duong)
    void displayByStatus(const string& status) const;
};
