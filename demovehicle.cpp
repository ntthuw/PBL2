#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>


#include "Vehicle.h"
#include "VehicleManager.h"
#include "FileManager.h"


using namespace std;


const string VEHICLE_FILE = "vehicle.txt";
const string VEHICLE_HEADER =
    "vehicle_id|vehicle_status|capacity|vehicle_km";
// ==================== STRING / DATA CONVERSION ====================


// Tach mot dong theo dau '|'.
vector<string> splitLine(const string& line, char delimiter = '|') {
    vector<string> fields;
    string field;
    stringstream ss(line);


    while (getline(ss, field, delimiter)) {
        fields.push_back(field);
    }


    // Bao dam truong cuoi cung rong van duoc ghi nhan.
    if (!line.empty() && line.back() == delimiter) {
        fields.push_back("");
    }


    return fields;
}


// Chuyen mot dong trong vehicle.txt thanh doi tuong Vehicle.
Vehicle parseVehicleLine(const string& line) {
    vector<string> fields = splitLine(line);


    if (fields.size() != 4) {
        throw invalid_argument("ERROR: VEHICLE CHUA DU YEU TO");
    }


    string id = fields[0];
    string status = fields[1];
    double capacity = stod(fields[2]);
    double km = stod(fields[3]);


    return Vehicle(id, status, capacity, km);
}
// Chuyen mot doi tuong Vehicle thanh mot dong theo dung cau truc TXT.
string vehicleToLine(const Vehicle& vehicle) {
    ostringstream line;


    line << vehicle.getVehicle_id() << '|'
         << vehicle.getVehicle_status() << '|'
         << setprecision(15) << vehicle.getVehicle_capacity() << '|'
         << setprecision(15) << vehicle.getVehicle_km();


    return line.str();
}
// ==================== LOAD / SAVE ====================


void loadVehicles(VehicleManager& manager, FileManager& fileManager) {
    if (!fileManager.fileExists(VEHICLE_FILE)) {
        cout << "ERROR: FILE BAN CHON CHUA DUOC TAO\n";
        fileManager.writeFile(VEHICLE_FILE, {VEHICLE_HEADER});
        return;
    }


    vector<string> lines = fileManager.readFile(VEHICLE_FILE);


    for (size_t i = 0; i < lines.size(); ++i) {
        // Bo qua dong trong va dong tieu de.
        if (lines[i].empty() || lines[i] == VEHICLE_HEADER) {
            continue;
        }


        try {
            Vehicle vehicle = parseVehicleLine(lines[i]);
            manager.addVehicle(vehicle);
        }
        catch (const exception& e) {
            cout << "BO QUA DOAN GHI KHONG HOP LE TAI DONG " << (i + 1)
                 << ": " << e.what() << '\n';
        }
    }


    cout << "Loaded " << manager.getSize() << " vehicle(s) from "
         << VEHICLE_FILE << ".\n";
}
void saveVehicles(const VehicleManager& manager, FileManager& fileManager) {
    vector<string> lines;
    lines.push_back(VEHICLE_HEADER);


    for (int i = 0; i < manager.getSize(); ++i) {
        lines.push_back(vehicleToLine(manager[i]));
    }


    fileManager.writeFile(VEHICLE_FILE, lines);
    cout << "DU LIEU PHUONG TIEN DA DUOC LUU VAO " << VEHICLE_FILE << ".\n";
}
// ==================== INPUT HELPERS ====================


string inputNonEmptyString(const string& message) {
    string value;


    do {
        cout << message;
        getline(cin, value);


        if (value.empty()) {
            cout << "DAU VAO BI RONG. XIN HAY THU LAI!\n";
        }
    } while (value.empty());


    return value;
}


double inputDouble(const string& message, double minimumValue) {
    double value;


    while (true) {
        cout << message;


        if (cin >> value && value >= minimumValue) {
            cin.ignore(10000, '\n');
            return value;
        }


        cout << "Lua chon khong hop le. Vui long chon lai! \n";
        cin.clear();
        cin.ignore(10000, '\n');
    }
}
string inputStatus() {
    cout << "CHON LOAI TRANG THAI:\n";
    cout << "1. Available\n";
    cout << "2. InUse\n";
    cout << "3. Maintenance\n";


    int choice;


    while (true) {
        cout << "LUA CHON CUA BAN: ";


        if (cin >> choice) {
            cin.ignore(10000, '\n');


            switch (choice) {
                case 1: return "Available";
                case 2: return "InUse";
                case 3: return "Maintenance";
                default:
                    cout << "Vui long chon lai 1,2 hoac 3.\n";
            }
        }
        else {
            cout << "Lua chon khong hop le. Vui long chon lai! \n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}
Vehicle inputNewVehicle(const string& fixedId = "") {
    string id = fixedId;


    if (id.empty()) {
        id = inputNonEmptyString("Nhap ID phuong tien: ");
    }


    string status = inputStatus();
    double capacity = inputDouble("Nhap suc chua toi da cua phuong tien ", 0.000001);
    double km = inputDouble("Nhap so km cua phuong tien ", 0.0);


    return Vehicle(id, status, capacity, km);
}
// ==================== DISPLAY / SEARCH ====================


void displayOneVehicle(const Vehicle& vehicle) {
    cout << left
         << setw(15) << vehicle.getVehicle_id()
         << setw(18) << vehicle.getVehicle_status()
         << setw(18) << fixed << setprecision(2)
         << vehicle.getVehicle_capacity()
         << setw(15) << fixed << setprecision(2)
         << vehicle.getVehicle_km()
         << '\n';
}


void searchVehicle(const VehicleManager& manager) {
    string id = inputNonEmptyString("Nhap ID phuong tien de tim kiem ");


    const Vehicle* found = manager.findVehicleById(id);


    if (found == nullptr) {
        cout << "Khong tim thay phuong tien.\n";
        return;
    }


    cout << "\nTHONG TIN PHUONG TIEN:\n";
    cout << left
         << setw(15) << "Vehicle ID"
         << setw(18) << "Status"
         << setw(18) << "Capacity"
         << setw(15) << "Kilometer" << '\n';
    cout << string(66, '-') << '\n';


    displayOneVehicle(*found);
}
// ==================== MENU ====================


void showMenu() {
    cout << "\n========== VEHICLE MANAGEMENT ==========\n";
    cout << "1. XEM TAT CA PHUONG TIEN HIEN CO\n";
    cout << "2. THEM PHUONG TIEN\n";
    cout << "3. CHINH SUA PHUONG TIEN\n";
    cout << "4. XOA PHUONG TIEN\n";
    cout << "5. TIM KIEM PHUONG TIEN THEO ID\n";
    cout << "6. LOC PHUONG TIEN THEO TRANG THAI\n";
    cout << "7. LUU DU LIEU\n";
    cout << "0. LUU DU LIEU VA THOAT\n";
    cout << "========================================\n";
    cout << "Moi dua ra lua chon cua ban: ";
}
int main() {
    FileManager& fileManager = FileManager::getInstance();
    VehicleManager vehicleManager;


    try {
        loadVehicles(vehicleManager, fileManager);
    }
    catch (const exception& e) {
        cout << "ERROR: KHONG THE TAI DU LIEU!: " << e.what() << '\n';
        return 1;
    }
    int choice;


    do {
        showMenu();


        if (!(cin >> choice)) {
            cout << "Lua chon khong hop le. Vui long chon lai!\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        cin.ignore(10000, '\n');


        try {
            switch (choice) {
                case 1:
                    vehicleManager.displayAll();
                    break;


                case 2: {
                    Vehicle vehicle = inputNewVehicle();
                    vehicleManager.addVehicle(vehicle);
                    cout << "Them phuong tien thanh cong!.\n";
                    saveVehicles(vehicleManager, fileManager);
                    break;
                }
                case 3: {
                    string id = inputNonEmptyString("Nhap ID phuong tien can chinh sua: ");


                    if (vehicleManager.findVehicleById(id) == nullptr) {
                        cout << "KHONG TIM THAY PHUONG TIEN.\n";
                        break;
                    }


                    cout << "NHAP THONG TIN CAN SUA (LUU Y ID LA CO DINH KHONG THE SUA)\n";
                    Vehicle updatedVehicle = inputNewVehicle(id);


                    if (vehicleManager.editVehicle(id, updatedVehicle)) {
                        cout << "Phuong tien duoc cap nhat thanh cong!\n";
                        saveVehicles(vehicleManager, fileManager);
                    }
                    else {
                        cout << "Cap nhat phuong tien that bai.\n";
                    }
                    break;
                }
                case 4: {
                    string id = inputNonEmptyString("Nhap ID phuong tien muon xoa: ");


                    if (vehicleManager.deleteVehicle(id)) {
                        cout << "Xoa phuong tien thanh cong\n";
                        saveVehicles(vehicleManager, fileManager);
                    }
                    else {
                        cout << "Khong tim thay phuong tien can xoa.\n";
                    }
                    break;
                }


                case 5:
                    searchVehicle(vehicleManager);
                    break;


                case 6: {
                    string status = inputStatus();
                    vehicleManager.displayByStatus(status);
                    break;
                }


                case 7:
                    saveVehicles(vehicleManager, fileManager);
                    break;


                case 0:
                    saveVehicles(vehicleManager, fileManager);
                    cout << "DANG THOAT CHUONG TRINH\n";
                    break;
                default:
                    cout << "Lua chon khong hop le. Vui long chon lai!! \n";
            }
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << '\n';
        }


    } while (choice != 0);


    return 0;
}