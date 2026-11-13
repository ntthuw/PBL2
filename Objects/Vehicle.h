#pragma once


#include <iostream>
#include <string>
using namespace std;


class Vehicle {
private:
    string vehicle_id;
    string vehicle_status;
    double vehicle_capacity; // Sửa capaciy thành capacity
    double vehicle_km;


public:
    Vehicle();


    Vehicle(
        string vehicle_id,
        string vehicle_status,
        double vehicle_capacity,
        double vehicle_km
    );


    string getVehicle_id() const;
    void setVehicle_id(const string& vehicle_id);


    string getVehicle_status() const;
    void setVehicle_status(const string& vehicle_status);


    double getVehicle_capacity() const;
    void setVehicle_capacity(double vehicle_capacity);


    double getVehicle_km() const;
    void setVehicle_km(double vehicle_km);


    void Show() const;
};

