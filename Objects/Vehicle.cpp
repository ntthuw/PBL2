#include "Vehicle.h"
#include <iomanip>
#include <string>


// Constructor
Vehicle::Vehicle()
    : vehicle_id(""),
    vehicle_status("Available"),
    vehicle_capacity(0.0),
    vehicle_km(0.0){


}


Vehicle::Vehicle (
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


// Ham get
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


//Hamset
void Vehicle::setVehicle_id (const string& vehicle_id){
    if(vehicle_id.empty()){
        throw invalid_argument ("Vehicle ID is empty!");
    }
    this->vehicle_id = vehicle_id;
}


void Vehicle::setVehicle_status(const string& vehicle_status){
    if (vehicle_status != "Available"   &&  vehicle_status != "InUse"  &&   vehicle_status!="Maintenance")
    {
        throw invalid_argument("Invaild vehicle status!");
    }
    this->vehicle_status= vehicle_status;
}


void Vehicle::setVehicle_capacity(double vehicle_capacity){
    if (vehicle_capacity <=0){
        throw invalid_argument("Vehicle capacity must be greater than 0!");


    }
    this->vehicle_capacity= vehicle_capacity;
}


void Vehicle::setVehicle_km(double vehicle_km){
    if(vehicle_km <0){
        throw invalid_argument("Vehicle kilometer cannot be negative");


    }
    this->vehicle_km= vehicle_km;
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











