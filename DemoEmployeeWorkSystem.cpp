#include "EmployeeWorkSystem.h"
#include "Object/Driver.h"
#include "Object/Janitor.h"
#include "Object/Manager.h"
#include "Object/Vehicle.h"
#include "Object/Assignment.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <limits>

using namespace std;

namespace
{
    string trim(const string& value)
    {
        const string whitespace = " \t\r\n";
        const size_t first = value.find_first_not_of(whitespace);
        if (first == string::npos) return "";
        const size_t last = value.find_last_not_of(whitespace);
        return value.substr(first, last - first + 1);
    }

    vector<string> splitLine(const string& line)
    {
        vector<string> fields;
        string field;
        stringstream input(line);
        while (getline(input, field, '|')) fields.push_back(trim(field));
        if (!line.empty() && line.back() == '|') fields.push_back("");
        return fields;
    }

    vector<vector<string> > readTable(const string& fileName, size_t expectedFields)
    {
        ifstream input(fileName.c_str());
        if (!input.is_open())
            throw runtime_error("Cannot open data file: " + fileName);

        vector<vector<string> > rows;
        string line;
        size_t lineNumber = 0;
        while (getline(input, line))
        {
            ++lineNumber;
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            vector<string> fields = splitLine(line);
            if (lineNumber == 1 && !fields.empty())
            {
                // Header is recognized by its column name, not by its position alone.
                if (fields[0] == "EmployeeID" || fields[0] == "VehicleID" || fields[0] == "AssignmentID")
                    continue;
            }
            if (fields.size() != expectedFields)
            {
                ostringstream message;
                message << fileName << ": invalid number of fields at line " << lineNumber;
                throw runtime_error(message.str());
            }
            rows.push_back(fields);
        }
        return rows;
    }

    string dateAfterDays(int days)
    {
        time_t now = time(nullptr);
        tm localDate = {};
#ifdef _WIN32
        localtime_s(&localDate, &now);
#else
        tm* temp = localtime(&now);
        if (temp == nullptr) throw runtime_error("Cannot read local date.");
        localDate = *temp;
#endif
        localDate.tm_hour = 12;
        localDate.tm_min = 0;
        localDate.tm_sec = 0;
        localDate.tm_mday += days;
        localDate.tm_isdst = -1;
        if (mktime(&localDate) == static_cast<time_t>(-1))
            throw runtime_error("Cannot calculate date token.");
        ostringstream output;
        output << setfill('0') << setw(4) << localDate.tm_year + 1900 << '-'
               << setw(2) << localDate.tm_mon + 1 << '-'
               << setw(2) << localDate.tm_mday;
        return output.str();
    }

    string resolveDate(const string& token)
    {
        if (token == "TODAY") return dateAfterDays(0);
        if (token.compare(0, 6, "TODAY+") == 0)
        {
            const string number = token.substr(6);
            if (number.empty()) throw invalid_argument("Invalid date token: " + token);
            for (size_t i = 0; i < number.size(); ++i)
                if (number[i] < '0' || number[i] > '9')
                    throw invalid_argument("Invalid date token: " + token);
            return dateAfterDays(stoi(number));
        }
        return token;
    }

    void loadEmployees(EmployeeWorkSystem& system)
    {
        const vector<vector<string> > rows = readTable("data/employee.txt", 5);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const string& id = rows[i][0];
            const string& name = rows[i][1];
            const string& address = rows[i][2];
            const string& position = rows[i][3];
            const string& status = rows[i][4];
            Employee* employee = nullptr;
            if (position == "Driver") employee = new Driver(id, name, address);
            else if (position == "Janitor") employee = new Janitor(id, name, address);
            else if (position == "Manager") employee = new Manager(id, name, address);
            else throw invalid_argument("Unknown employee position: " + position);
            try
            {
                employee->set_status(status);
                system.addEmployee(*employee);
            }
            catch (...)
            {
                delete employee;
                throw;
            }
            delete employee;
        }
    }

    void loadVehicles(EmployeeWorkSystem& system)
    {
        const vector<vector<string> > rows = readTable("data/vehicle.txt", 4);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            size_t usedCapacity = 0, usedKm = 0;
            const double capacity = stod(rows[i][2], &usedCapacity);
            const double kilometer = stod(rows[i][3], &usedKm);
            if (usedCapacity != rows[i][2].size() || usedKm != rows[i][3].size())
                throw invalid_argument("Invalid numeric value in vehicle.txt.");
            Vehicle vehicle(rows[i][0], rows[i][1], capacity, kilometer);
            system.getVehicleManager().addVehicle(vehicle);
        }
        system.syncVehicleStatuses();
    }

    void loadAssignments(EmployeeWorkSystem& system)
    {
        const vector<vector<string> > rows = readTable("data/assignment.txt", 7);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const string id = rows[i][0];
            const string date = resolveDate(rows[i][1]);
            const string shift = rows[i][2];
            const string vehicleId = rows[i][3];
            Assignment draft(id, vector<string>(), "", shift, date, "Planned");
            system.createAssignment(draft);
            if (!system.addEmployeeToAssignment(id, rows[i][4]) ||
                !system.addEmployeeToAssignment(id, rows[i][5]) ||
                !system.addEmployeeToAssignment(id, rows[i][6]))
                throw runtime_error("Cannot build team for Assignment " + id);
            if (!system.assignVehicle(id, vehicleId))
                throw runtime_error("Cannot assign vehicle to Assignment " + id);
            if (!system.confirmAssignment(id))
                throw runtime_error("Cannot confirm Assignment " + id + ". Check team, vehicle, date and shift.");
        }
        system.syncVehicleStatuses();
    }

    string ask(const string& prompt);

    bool isValidDateFormat(const string& value)
    {
        if (value.size() != 10 || value[4] != '-' || value[7] != '-') return false;
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (i == 4 || i == 7) continue;
            if (value[i] < '0' || value[i] > '9') return false;
        }

        const int year = stoi(value.substr(0, 4));
        const int month = stoi(value.substr(5, 2));
        const int day = stoi(value.substr(8, 2));
        if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) return false;

        tm date = {};
        date.tm_year = year - 1900;
        date.tm_mon = month - 1;
        date.tm_mday = day;
        date.tm_hour = 12;
        date.tm_isdst = -1;
        if (mktime(&date) == static_cast<time_t>(-1)) return false;

        return date.tm_year == year - 1900 && date.tm_mon == month - 1 && date.tm_mday == day;
    }

    bool isValidShift(const string& shift)
    {
        return shift == "CA01" || shift == "CA02" || shift == "CA03";
    }

    string askNonEmpty(const string& prompt)
    {
        while (true)
        {
            const string value = ask(prompt);
            if (!value.empty()) return value;
            cout << "Du lieu khong duoc de trong. Vui long nhap lai.\n";
        }
    }

    string askValidDate(const string& prompt)
    {
        while (true)
        {
            const string value = ask(prompt);
            if (isValidDateFormat(value)) return value;
            cout << "Ngay khong hop le. Vui long nhap theo dinh dang YYYY-MM-DD.\n";
        }
    }

    string askValidShift(const string& prompt)
    {
        while (true)
        {
            const string value = ask(prompt);
            if (isValidShift(value)) return value;
            cout << "Ma ca khong hop le. Chi chap nhan CA01, CA02 hoac CA03.\n";
        }
    }

    string ask(const string& prompt)
    {
        cout << prompt;
        string value;
        getline(cin, value);
        return trim(value);
    }

    int askInt(const string& prompt)
    {
        while (true)
        {
            const string value = ask(prompt);
            try
            {
                size_t used = 0;
                const int result = stoi(value, &used);
                if (used == value.size()) return result;
            }
            catch (const exception&) {}
            cout << "Lua chon khong hop le. Vui long nhap so.\n";
        }
    }

    void pauseScreen()
    {
        cout << "\nNhan Enter de tiep tuc...";
        string ignored;
        getline(cin, ignored);
    }

    void showEmployeeMenu()
    {
        cout << "\n========== QUAN LY NHAN VIEN =========="
             << "\n1. Hien thi danh sach"
             << "\n2. Them nhan vien"
             << "\n3. Sua nhan vien"
             << "\n4. Xoa nhan vien"
             << "\n5. Tim nhan vien theo ID"
             << "\n0. Quay lai\n";
    }

    void employeeMenu(EmployeeWorkSystem& system)
    {
        while (true)
        {
            showEmployeeMenu();
            const int choice = askInt("Lua chon: ");
            try
            {
                if (choice == 0) return;
                if (choice == 1) system.getEmployeeManager().displayAll();
                else if (choice == 2)
                {
                    string id;
                    while (true)
                    {
                        id = askNonEmpty("Employee ID: ");
                        if (system.findEmployeeById(id) == nullptr) break;
                        cout << "Loi: Employee ID " << id << " da ton tai. Vui long nhap ID khac.\n";
                    }
                    const string name = askNonEmpty("Ho ten: ");
                    const string address = askNonEmpty("Dia chi: ");
                    int type = 0;
                    while (true)
                    {
                        cout << "1. Driver\n2. Janitor\n3. Manager\n";
                        type = askInt("Chuc vu: ");
                        if (type >= 1 && type <= 3) break;
                        cout << "Chuc vu khong hop le. Vui long chon lai.\n";
                    }
                    Employee* employee = nullptr;
                    if (type == 1) employee = new Driver(id, name, address);
                    else if (type == 2) employee = new Janitor(id, name, address);
                    else employee = new Manager(id, name, address);
                    try { system.addEmployee(*employee); }
                    catch (...) { delete employee; throw; }
                    delete employee;
                    cout << "Them nhan vien thanh cong.\n";
                }
                else if (choice == 3)
                {
                    const string id = ask("Employee ID can sua: ");
                    const Employee* current = system.findEmployeeById(id);
                    if (current == nullptr) { cout << "Khong tim thay nhan vien.\n"; continue; }
                    const string nameInput = ask("Ho ten moi (Enter de giu nguyen): ");
                    const string addressInput = ask("Dia chi moi (Enter de giu nguyen): ");
                    const string statusInput = ask("Trang thai Active/Inactive (Enter de giu nguyen): ");
                    const string name = nameInput.empty() ? current->get_name() : nameInput;
                    const string address = addressInput.empty() ? current->get_address() : addressInput;
                    const string status = statusInput.empty() ? current->get_status() : statusInput;
                    Employee* updated = nullptr;
                    if (current->get_position() == "Driver") updated = new Driver(id, name, address);
                    else if (current->get_position() == "Janitor") updated = new Janitor(id, name, address);
                    else updated = new Manager(id, name, address);
                    try
                    {
                        updated->set_status(status);
                        if (!system.editEmployee(id, *updated))
                            cout << "Khong the sua: nhan vien co the dang nam trong Assignment hoac du lieu khong hop le.\n";
                        else cout << "Cap nhat nhan vien thanh cong.\n";
                    }
                    catch (...) { delete updated; throw; }
                    delete updated;
                }
                else if (choice == 4)
                {
                    const string id = ask("Employee ID can xoa: ");
                    if (system.deleteEmployee(id)) cout << "Xoa nhan vien thanh cong.\n";
                    else cout << "Khong the xoa. Nhan vien co the da co lich su cham cong/Assignment.\n";
                }
                else if (choice == 5)
                {
                    const string id = ask("Employee ID: ");
                    const Employee* employee = system.findEmployeeById(id);
                    if (employee == nullptr) cout << "Khong tim thay nhan vien.\n";
                    else employee->Show();
                }
                else cout << "Lua chon khong ton tai.\n";
            }
            catch (const exception& error) { cout << "Loi: " << error.what() << '\n'; }
            pauseScreen();
        }
    }

    void showAssignmentMenu()
    {
        cout << "\n========== QUAN LY CONG VIEC / ASSIGNMENT =========="
             << "\n1. Hien thi danh sach Assignment"
             << "\n2. Tao Assignment Planned"
             << "\n3. Them nhan vien vao Assignment"
             << "\n4. Go nhan vien khoi Assignment"
             << "\n5. Gan phuong tien"
             << "\n6. Go phuong tien"
             << "\n7. Xac nhan Assignment"
             << "\n8. Sua ngay/ca Assignment"
             << "\n9. Huy Assignment"
             << "\n10. Xoa Assignment Planned"
             << "\n11. Tim Assignment theo ID"
             << "\n12. Loc Assignment theo ngay"
             << "\n13. Loc Assignment theo ca"
             << "\n14. Loc Assignment theo nhan vien"
             << "\n15. Hoan thanh Assignment (sau khi het ca)"
             << "\n0. Quay lai\n";
    }

    void assignmentMenu(EmployeeWorkSystem& system)
    {
        while (true)
        {
            showAssignmentMenu();
            const int choice = askInt("Lua chon: ");
            try
            {
                AssignmentManager& manager = system.getAssignmentManager();
                if (choice == 0) return;
                if (choice == 1) manager.displayAll();
                else if (choice == 2)
                {
                    string id;
                    AssignmentManager& assignmentManager = system.getAssignmentManager();
                    while (true)
                    {
                        id = askNonEmpty("Assignment ID: ");
                        if (assignmentManager.findAssignmentById(id) == nullptr) break;
                        cout << "Loi: Assignment ID " << id << " da ton tai. Vui long nhap ID khac.\n";
                    }
                    const string date = askValidDate("Ngay (YYYY-MM-DD): ");
                    const string shift = askValidShift("Ca (CA01/CA02/CA03): ");
                    Assignment draft(id, vector<string>(), "", shift, date, "Planned");
                    system.createAssignment(draft);
                    cout << "Tao Assignment Planned thanh cong. Hay them 1 Driver, 2 Janitors, gan xe va xac nhan.\n";
                }
                else if (choice == 3)
                {
                    string aid;
                    Assignment* target = nullptr;
                    while (true)
                    {
                        aid = askNonEmpty("Assignment ID: ");
                        target = manager.findAssignmentById(aid);
                        if (target != nullptr) break;
                        cout << "Khong tim thay Assignment " << aid << ". Vui long nhap lai.\n";
                    }
                    bool continueAdding = true;
                    while (continueAdding)
                    {
                        const string eid = askNonEmpty("Employee ID can them (0 de ket thuc): ");
                        if (eid == "0") break;
                        if (system.findEmployeeById(eid) == nullptr)
                        {
                            cout << "Khong tim thay nhan vien " << eid << ". Vui long thu lai.\n";
                            continue;
                        }
                        try
                        {
                            if (system.addEmployeeToAssignment(aid, eid))
                                cout << "Da them nhan vien " << eid << ".\n";
                            else
                            {
                                cout << "Khong the them nhan vien: Assignment khong ton tai hoac khong con o thoi diem cho phep chinh sua.\n";
                                break;
                            }
                        }
                        catch (const exception& ex)
                        {
                            // Nhap lai Employee ID neu vi pham dieu kien them thanh vien.
                            cout << "Loi: " << ex.what() << "\n";
                            continue;
                        }
                        cout << "Nhan Enter de tiep tuc them nhan vien hoac nhap 0 de ket thuc.\n";
                        const string next = ask("Tiep tuc (Enter/0): ");
                        if (next == "0") continueAdding = false;
                    }
                }
                else if (choice == 4)
                {
                    const string aid = ask("Assignment ID: ");
                    const string eid = ask("Employee ID can go: ");
                    cout << (system.removeEmployeeFromAssignment(aid, eid) ? "Da go nhan vien.\n" : "Khong the go nhan vien.\n");
                }
                else if (choice == 5)
                {
                    string aid;
                    while (true)
                    {
                        aid = askNonEmpty("Assignment ID: ");
                        if (manager.findAssignmentById(aid) != nullptr) break;
                        cout << "Khong tim thay Assignment " << aid << ". Vui long nhap lai.\n";
                    }
                    const string vid = askNonEmpty("Vehicle ID: ");
                    cout << (system.assignVehicle(aid, vid) ? "Da gan phuong tien.\n" : "Khong the gan phuong tien. Kiem tra xe ton tai, trang thai bao tri va xung dot lich.\n");
                }
                else if (choice == 6)
                {
                    const string aid = ask("Assignment ID: ");
                    cout << (system.removeVehicleFromAssignment(aid) ? "Da go phuong tien.\n" : "Khong the go phuong tien.\n");
                }
                else if (choice == 7)
                {
                    const string aid = ask("Assignment ID can xac nhan: ");
                    cout << (system.confirmAssignment(aid) ? "Xac nhan Assignment thanh cong.\n" : "Khong the xac nhan. Kiem tra du 1 Driver, 2 Janitors, xe va lich.\n");
                }
                else if (choice == 8)
                {
                    const string aid = askNonEmpty("Assignment ID: ");
                    Assignment* current = manager.findAssignmentById(aid);
                    if (current == nullptr) { cout << "Khong tim thay Assignment.\n"; continue; }
                    string dateInput;
                    while (true)
                    {
                        dateInput = ask("Ngay moi YYYY-MM-DD (Enter giu nguyen): ");
                        if (dateInput.empty() || isValidDateFormat(dateInput)) break;
                        cout << "Ngay khong hop le. Vui long nhap lai theo YYYY-MM-DD.\n";
                    }
                    string shiftInput;
                    while (true)
                    {
                        shiftInput = ask("Ca moi CA01/CA02/CA03 (Enter giu nguyen): ");
                        if (shiftInput.empty() || isValidShift(shiftInput)) break;
                        cout << "Ma ca khong hop le. Vui long nhap lai.\n";
                    }
                    Assignment updated(aid, current->getEmployee_ids(), current->getVehicle_id(),
                                       shiftInput.empty() ? current->getShift_id() : shiftInput,
                                       dateInput.empty() ? current->getAssignment_date() : dateInput,
                                       current->getAssignment_status());
                    cout << (system.editAssignment(aid, updated) ? "Cap nhat Assignment thanh cong.\n" : "Khong the cap nhat Assignment.\n");
                }
                else if (choice == 9)
                {
                    const string aid = ask("Assignment ID can huy: ");
                    cout << (system.cancelAssignment(aid) ? "Da huy Assignment.\n" : "Khong the huy (co the ca da bat dau/ket thuc).\n");
                }
                else if (choice == 10)
                {
                    const string aid = ask("Assignment ID can xoa: ");
                    cout << (system.deleteAssignment(aid) ? "Da xoa Assignment.\n" : "Khong the xoa Assignment.\n");
                }
                else if (choice == 11)
                {
                    const string aid = ask("Assignment ID: ");
                    const Assignment* item = manager.findAssignmentById(aid);
                    if (item == nullptr) cout << "Khong tim thay Assignment.\n";
                    else item->Show();
                }
                else if (choice >= 12 && choice <= 14)
                {
                    const string key = ask(choice == 12 ? "Ngay YYYY-MM-DD: " : (choice == 13 ? "Ma ca: " : "Employee ID: "));
                    bool found = false;
                    for (int i = 0; i < manager.getSize(); ++i)
                    {
                        const Assignment& item = manager[i];
                        bool match = choice == 12 ? item.getAssignment_date() == key :
                                     (choice == 13 ? item.getShift_id() == key : item.hasEmployee(key));
                        if (match) { item.Show(); found = true; }
                    }
                    if (!found) cout << "Khong tim thay Assignment phu hop.\n";
                }
                else if (choice == 15)
                {
                    const string aid = ask("Assignment ID: ");
                    cout << (system.completeAssignment(aid) ? "Da hoan thanh va chot Attendance.\n" : "Chua the hoan thanh (Assignment phai Assigned va da het ca).\n");
                }
                else cout << "Lua chon khong ton tai.\n";
            }
            catch (const exception& error) { cout << "Loi: " << error.what() << '\n'; }
            pauseScreen();
        }
    }

    bool isValidTimeInput(const string& value)
    {
        if (value.size() != 5 || value[2] != ':') return false;
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (i == 2) continue;
            if (value[i] < '0' || value[i] > '9') return false;
        }
        const int hour = stoi(value.substr(0, 2));
        const int minute = stoi(value.substr(3, 2));
        return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
    }

    string askValidTime(const string& prompt)
    {
        while (true)
        {
            const string value = ask(prompt);
            if (isValidTimeInput(value)) return value;
            cout << "Gio khong hop le. Vui long nhap theo dinh dang HH:MM (00:00 - 23:59).\n";
        }
    }

    string askAttendanceEmployeeId(EmployeeWorkSystem& system)
    {
        while (true)
        {
            const string employeeId = askNonEmpty("Employee ID: ");
            if (system.findEmployeeById(employeeId) != nullptr)
                return employeeId;
            cout << "Khong tim thay nhan vien " << employeeId << ". Vui long nhap lai.\n";
        }
    }

    string askAttendanceAssignmentId(EmployeeWorkSystem& system)
    {
        while (true)
        {
            const string assignmentId = askNonEmpty("Assignment ID: ");
            if (system.getAssignmentManager().findAssignmentById(assignmentId) != nullptr)
                return assignmentId;
            cout << "Khong tim thay Assignment " << assignmentId << ". Vui long nhap lai.\n";
        }
    }

    void showRecordedAttendanceTime(
        EmployeeWorkSystem& system,
        const string& employeeId,
        const string& assignmentId,
        bool isCheckIn
    )
    {
        const Attendance* attendance = system.getAttendanceManager().findAttendance(
            employeeId,
            assignmentId
        );

        if (attendance == nullptr) return;

        cout << (isCheckIn ? "Gio Check-in da luu: " : "Gio Check-out da luu: ")
             << (isCheckIn ? attendance->getCheck_in() : attendance->getCheck_out())
             << '\n';
    }

    void showAttendanceMenu()
    {
        cout << "\n========== QUAN LY CHAM CONG / ATTENDANCE =========="
             << "\n--- Employee (demo luong nhan vien) ---"
             << "\n1. Employee Check-in (tu dong lay gio hien tai)"
             << "\n2. Employee Check-out (tu dong lay gio hien tai)"
             << "\n--- Manager (demo luong quan ly, chua thay the AuthManager) ---"
             << "\n3. Manager ghi nhan/cap nhat Check-in"
             << "\n4. Manager ghi nhan/cap nhat Check-out"
             << "\n5. Manager chi hieu chinh Check-in da co"
             << "\n6. Manager chi hieu chinh Check-out da co"
             << "\n--- Tra cuu ---"
             << "\n7. Tim cham cong theo nhan vien"
             << "\n8. Loc cham cong theo ngay"
             << "\n9. Hien thi danh sach cham cong"
             << "\n0. Quay lai\n";
    }

    void attendanceMenu(EmployeeWorkSystem& system)
    {
        while (true)
        {
            showAttendanceMenu();
            const int choice = askInt("Lua chon: ");
            try
            {
                AttendanceManager& manager = system.getAttendanceManager();
                if (choice == 0) return;

                if (choice == 1 || choice == 2)
                {
                    const string employeeId = askAttendanceEmployeeId(system);
                    const string assignmentId = askAttendanceAssignmentId(system);

                    const bool success = (choice == 1)
                        ? system.checkInByEmployee(employeeId, assignmentId)
                        : system.checkOutByEmployee(employeeId, assignmentId);

                    if (success)
                    {
                        cout << (choice == 1
                            ? "Employee Check-in thanh cong.\n"
                            : "Employee Check-out thanh cong.\n");
                        // Hien thi dung thoi gian da duoc he thong ghi vao Attendance.
                        showRecordedAttendanceTime(
                            system, employeeId, assignmentId, choice == 1
                        );
                    }
                    else
                    {
                        cout << "Khong the thuc hien. Kiem tra Assignment Assigned, "
                             << "nhan vien thuoc Assignment, dung ngay va chua cham cong lan nay.\n";
                    }
                }
                else if (choice >= 3 && choice <= 6)
                {
                    cout << "Luu y: day la luong demo Manager de kiem thu nghiep vu; "
                         << "phan quyen that se do AuthManager dam nhiem.\n";

                    const string employeeId = askAttendanceEmployeeId(system);
                    const string assignmentId = askAttendanceAssignmentId(system);
                    const string time = askValidTime("Nhap gio thuc te (HH:MM): ");

                    bool success = false;
                    bool isCheckIn = (choice == 3 || choice == 5);

                    if (choice == 3)
                        success = system.recordCheckInByManager(employeeId, assignmentId, time);
                    else if (choice == 4)
                        success = system.recordCheckOutByManager(employeeId, assignmentId, time);
                    else if (choice == 5)
                        success = system.correctCheckInByManager(employeeId, assignmentId, time);
                    else
                        success = system.correctCheckOutByManager(employeeId, assignmentId, time);

                    if (success)
                    {
                        cout << "Manager " << (isCheckIn ? "Check-in" : "Check-out")
                             << (choice <= 4 ? " da duoc ghi nhan/cap nhat.\n"
                                             : " da duoc hieu chinh.\n");
                        showRecordedAttendanceTime(
                            system, employeeId, assignmentId, isCheckIn
                        );
                    }
                    else
                    {
                        cout << "Khong the thuc hien. Kiem tra Attendance, trang thai Assignment, "
                             << "Check-in truoc Check-out va thao tac hieu chinh.\n";
                    }
                }
                else if (choice == 7)
                    manager.findByEmployeeId(askNonEmpty("Employee ID: "));
                else if (choice == 8)
                {
                    const string date = askValidDate("Ngay YYYY-MM-DD: ");
                    manager.displayByDate(date);
                }
                else if (choice == 9)
                    manager.displayAttendanceTable();
                else
                    cout << "Lua chon khong ton tai.\n";
            }
            catch (const exception& error)
            {
                cout << "Loi: " << error.what() << '\n';
            }
            pauseScreen();
        }
    }

}

int main()
{
    try
    {
        EmployeeManager employeeManager;
        AssignmentManager assignmentManager;
        AttendanceManager attendanceManager;
        VehicleManager vehicleManager;
        EmployeeWorkSystem system(employeeManager, assignmentManager, attendanceManager, vehicleManager);

        // Chi doc du lieu nguon. Moi thay doi trong luc chay chi nam trong RAM.
        loadEmployees(system);
        loadVehicles(system);
        loadAssignments(system);

        cout << "\nDa nap du lieu thanh cong. Attendance duoc tao tu Assignment.\n";
        cout << "So nhan vien: " << employeeManager.getSize()
             << " | So phuong tien: " << vehicleManager.getSize()
             << " | So Assignment: " << assignmentManager.getSize()
             << " | So Attendance: " << attendanceManager.getSize() << '\n';

        while (true)
        {
            cout << "\n========== EMPLOYEE WORK SYSTEM =========="
                 << "\n1. Quan ly nhan vien"
                 << "\n2. Quan ly cong viec (Assignment)"
                 << "\n3. Quan ly cham cong (Attendance)"
                 << "\n0. Thoat\n";
            const int choice = askInt("Lua chon: ");
            if (choice == 0)
            {
                cout << "Ket thuc.\n";
                break;
            }
            if (choice == 1) employeeMenu(system);
            else if (choice == 2) assignmentMenu(system);
            else if (choice == 3) attendanceMenu(system);
            else cout << "Lua chon khong ton tai.\n";
        }
    }
    catch (const exception& error)
    {
        cerr << "LOI KHOI TAO DEMO: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
