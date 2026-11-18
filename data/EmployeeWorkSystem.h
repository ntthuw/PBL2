#pragma once

#include "Managers/EmployeeManager.h"
#include "Managers/AssignmentManager.h"
#include "Managers/AttendanceManager.h"
#include "Managers/VehicleManager.h"

#include <string>
#include <vector>

using namespace std;

class EmployeeWorkSystem
{
private:
    EmployeeManager& employeeManager;
    AssignmentManager& assignmentManager;
    AttendanceManager& attendanceManager;
    VehicleManager& vehicleManager;

    bool canModifyAssignment(const string& assignmentId) const;

    void validateTeamComposition(
        const vector<string>& employeeIds
    ) const;

    void validateEmployeeAvailability(
        const string& employeeId,
        const string& date,
        const string& shiftId,
        const string& excludeAssignmentId = ""
    ) const;

    void validateVehicleAvailability(
        const string& vehicleId,
        const string& date,
        const string& shiftId,
        const string& excludeAssignmentId = ""
    ) const;

    bool getShiftTime(
        const string& shiftId,
        int& startHour,
        int& startMinute,
        int& endHour,
        int& endMinute
    ) const;

    bool hasShiftStarted(
        const string& date,
        const string& shiftId
    ) const;

    bool hasShiftEnded(
        const string& date,
        const string& shiftId
    ) const;

    void updateVehicleStatus(const string& assignmentId);
    void releaseVehicle(const string& vehicleId);

public:
    EmployeeWorkSystem(
        EmployeeManager& employeeManager,
        AssignmentManager& assignmentManager,
        AttendanceManager& attendanceManager,
        VehicleManager& vehicleManager
    );

    EmployeeWorkSystem(const EmployeeWorkSystem&) = delete;
    EmployeeWorkSystem& operator=(const EmployeeWorkSystem&) = delete;

    void syncVehicleStatuses();

    // Employee
    void addEmployee(const Employee& employee);

    bool editEmployee(
        const string& employeeId,
        const Employee& updatedEmployee
    );

    bool deleteEmployee(const string& employeeId);
    bool deleteVehicle(const string& vehicleId);

    Employee* findEmployeeById(const string& employeeId);

    const Employee* findEmployeeById(
        const string& employeeId
    ) const;

    // Assignment
    void createAssignment(const Assignment& assignment);

    bool editAssignment(
        const string& assignmentId,
        const Assignment& updatedAssignment
    );

    bool deleteAssignment(const string& assignmentId);
    bool confirmAssignment(const string& assignmentId);
    bool cancelAssignment(const string& assignmentId);
    bool completeAssignment(const string& assignmentId);

    // Assignment members
    bool addEmployeeToAssignment(
        const string& assignmentId,
        const string& employeeId
    );

    bool removeEmployeeFromAssignment(
        const string& assignmentId,
        const string& employeeId
    );

    // Assignment vehicle
    bool assignVehicle(
        const string& assignmentId,
        const string& vehicleId
    );

    bool removeVehicleFromAssignment(const string& assignmentId);

    // Attendance - Employee: tu dong lay ngay/gio hien tai cua may tinh.
    // Nhan vien chi duoc thao tac trong ngay Assignment va moi thao tac 1 lan.
    bool checkInByEmployee(
        const string& employeeId,
        const string& assignmentId
    );

    bool checkOutByEmployee(
        const string& employeeId,
        const string& assignmentId
    );

    // Attendance - Manager: time theo dinh dang HH:MM, khong gioi han
    // ngay/gio thao tac. Cho phep ghi nhan lan dau hoac hieu chinh.
    bool recordCheckInByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& time
    );

    bool recordCheckOutByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& time
    );

    // Chi hieu chinh khi ban ghi tuong ung da ton tai.
    bool correctCheckInByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& time
    );

    bool correctCheckOutByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& time
    );

    // Tuong thich nguoc voi code/demo cu: mac dinh la Employee.
    bool checkIn(
        const string& employeeId,
        const string& assignmentId
    );

    bool checkOut(
        const string& employeeId,
        const string& assignmentId
    );

    bool finalizeAttendance(const string& assignmentId);

    // Accessors
    EmployeeManager& getEmployeeManager();
    AssignmentManager& getAssignmentManager();
    AttendanceManager& getAttendanceManager();
    VehicleManager& getVehicleManager();
};
