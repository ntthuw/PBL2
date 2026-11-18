
#pragma once

#include "Attendance.h"
#include <string>

using namespace std;

class AttendanceManager
{
private:
    Attendance* arr;
    int size;
    int capacity;

    void resize();

    string generateAttendanceId() const;

public:
    // Constructor / Destructor
    AttendanceManager();
    AttendanceManager(const AttendanceManager& other);
    AttendanceManager& operator=(const AttendanceManager& other);
    ~AttendanceManager();

    // Getter
    int getSize() const;
    int getCapacity() const;

    Attendance* getArr();
    const Attendance* getArr() const;

    // Nap chong toan tu
    AttendanceManager& operator+=(const Attendance& attendance);
    AttendanceManager& operator-=(const string& attendanceId);

    Attendance& operator[](int index);
    const Attendance& operator[](int index) const;

    // Them Attendance tu EmployeeWorkSystem
    void addAttendance(const Attendance& attendance);

    // Check-in / Check-out
    void checkIn(
        const string& employeeId,
        const string& assignmentId,
        const string& date,
        const string& time,
        const string& status
    );

    void checkOut(
        const string& employeeId,
        const string& assignmentId,
        const string& date,
        const string& time
    );

    // Manager: ghi nhan hoac hieu chinh thoi gian Check-in.
    // status phai la OnTime hoac Late, do EmployeeWorkSystem xac dinh.
    void setCheckInByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& date,
        const string& time,
        const string& status
    );

    // Manager: ghi nhan hoac hieu chinh thoi gian Check-out.
    // status phai la OnTime hoac Late sau khi doi chieu gio Check-in.
    void setCheckOutByManager(
        const string& employeeId,
        const string& assignmentId,
        const string& date,
        const string& time,
        const string& status
    );

    // Cap nhat trang thai Attendance
    bool markAbsent(
        const string& employeeId,
        const string& assignmentId
    );

    bool markMissingCheckOut(
        const string& employeeId,
        const string& assignmentId
    );

    void cancelAttendanceForAssignment(
        const string& assignmentId
    );

    // Xoa Attendance khi dong bo Assignment
    bool deleteAttendance(const string& attendanceId);

    bool removeAttendanceForEmployee(
        const string& employeeId,
        const string& assignmentId
    );

    // Tim kiem
    Attendance* findAttendanceById(
        const string& attendanceId
    );

    const Attendance* findAttendanceById(
        const string& attendanceId
    ) const;

    Attendance* findAttendance(
        const string& employeeId,
        const string& assignmentId
    );

    const Attendance* findAttendance(
        const string& employeeId,
        const string& assignmentId
    ) const;

    // Hien thi
    void findByEmployeeId(const string& employeeId) const;
    void displayAll() const;
    void displayAttendanceTable() const;
    void displayByDate(const string& date) const;

    // Kiem tra lich su cham cong
    bool hasAttendanceForAssignment(
        const string& assignmentId
    ) const;

    bool hasAttendanceForEmployee(
        const string& employeeId
    ) const;
};
