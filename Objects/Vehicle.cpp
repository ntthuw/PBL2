
#include "Vehicle.h"
#include <iomanip>
#include <string>
#include <stdexcept>
#include <cmath>
#include <cctype>


// Ham kiem tra chuoi rong hoac chi co khoang trang
bool isBlank(const string& str) {
    for (size_t i = 0; i < str.length(); i++) {
        if (!isspace(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }
    return true;
}

// Constructor
Vehicle::Vehicle()
    : vehicle_id(""),
      vehicle_status("Available"),
      vehicle_capacity(0.0),
      vehicle_km(0.0) {

}

Vehicle::Vehicle(
    string vehicle_id,
    string vehicle_status,
    double vehicle_capacity,
    double vehicle_km
) : vehicle_id(""),
    vehicle_status("Available"),
    vehicle_capacity(0.0),
    vehicle_km(0.0) {

    setVehicle_id(vehicle_id);
    setVehicle_status(vehicle_status);
    setVehicle_capacity(vehicle_capacity);
    setVehicle_km(vehicle_km);
}

// Getter
string Vehicle::getVehicle_id() const {
    return vehicle_id;
}

string Vehicle::getVehicle_status() const {
    return vehicle_status;
}

double Vehicle::getVehicle_capacity() const {
    return vehicle_capacity;
}

double Vehicle::getVehicle_km() const {
    return vehicle_km;
}

// Setter
void Vehicle::setVehicle_id(const string& vehicle_id) {
    if (isBlank(vehicle_id)) {
        throw invalid_argument("Vehicle ID cannot be empty!");
    }

    this->vehicle_id = vehicle_id;
}

void Vehicle::setVehicle_status(const string& vehicle_status) {
    if (vehicle_status != "Available" &&
        vehicle_status != "InUse" &&
        vehicle_status != "Maintenance") {

        throw invalid_argument("Invalid vehicle status!");
    }

    this->vehicle_status = vehicle_status;
}

void Vehicle::setVehicle_capacity(double vehicle_capacity) {
    if (!isfinite(vehicle_capacity) || vehicle_capacity <= 0) {
        throw invalid_argument(
            "Vehicle capacity must be a finite number greater than 0!"
        );
    }

    this->vehicle_capacity = vehicle_capacity;
}

void Vehicle::setVehicle_km(double vehicle_km) {
    if (!isfinite(vehicle_km) || vehicle_km < 0) {
        throw invalid_argument(
            "Vehicle kilometer must be a finite non-negative number!"
        );
    }

    this->vehicle_km = vehicle_km;
}

// Ham kiem tra du lieu
void Vehicle::validate() const {
    if (isBlank(vehicle_id)) {
        throw invalid_argument("Vehicle ID cannot be empty!");
    }

    if (vehicle_status != "Available" &&
        vehicle_status != "InUse" &&
        vehicle_status != "Maintenance") {

        throw invalid_argument("Invalid vehicle status!");
    }

    if (!isfinite(vehicle_capacity) || vehicle_capacity <= 0) {
        throw invalid_argument("Invalid vehicle capacity!");
    }

    if (!isfinite(vehicle_km) || vehicle_km < 0) {
        throw invalid_argument("Invalid vehicle kilometer!");
    }
}

// Ham show
void Vehicle::Show() const {
    cout << left
         << setw(15) << vehicle_id
         << setw(18) << vehicle_status
         << setw(18) << fixed << setprecision(2) << vehicle_capacity
         << setw(15) << fixed << setprecision(2) << vehicle_km
         << '\n';
}


