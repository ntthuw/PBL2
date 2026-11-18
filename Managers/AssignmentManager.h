
#pragma once

#include "Assignment.h"
#include <string>

using namespace std;

class AssignmentManager
{
private:
    Assignment* arr;
    int size;
    int capacity;

    // Mo rong mang khi can thiet
    void resize();

public:
    // Constructor / Destructor
    AssignmentManager();
    AssignmentManager(const AssignmentManager& other);
    AssignmentManager& operator=(const AssignmentManager& other);
    ~AssignmentManager();

    // Getter
    int getSize() const;
    int getCapacity() const;

    Assignment* getArr();
    const Assignment* getArr() const;

    // Nap chong toan tu
    AssignmentManager& operator+=(const Assignment& assignment);
    AssignmentManager& operator-=(const string& assignmentId);

    Assignment& operator[](int index);
    const Assignment& operator[](int index) const;

    // Quan ly Assignment
    void addAssignment(const Assignment& assignment);

    bool editAssignment(
        const string& assignmentId,
        const Assignment& updatedAssignment
    );

    bool deleteAssignment(const string& assignmentId);

    // Tim kiem va hien thi
    Assignment* findAssignmentById(const string& assignmentId);
    const Assignment* findAssignmentById(const string& assignmentId) const;

    void displayAll() const;

    // Kiem tra trung lich phuong tien
    bool checkVehicleConflict(
        const string& vehicleId,
        const string& date,
        const string& shiftId,
        const string& excludeAssignmentId = ""
    ) const;

    // Kiem tra trung lich nhan vien
    bool checkEmployeeConflict(
        const string& employeeId,
        const string& date,
        const string& shiftId,
        const string& excludeAssignmentId = ""
    ) const;

    // Kiem tra trung lich tong hop
    bool checkConflict(const Assignment& assignment) const;

    // Kiem tra ma Assignment
    bool isAssignmentIdExist(const string& assignmentId) const;
};
