
#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Assignment
{
private:
    string assignment_id;
    vector<string> employee_ids;
    string vehicle_id;
    string shift_id;
    string assignment_date;
    string assignment_status;

public:
    // Constructor
    Assignment();

    Assignment(
        const string& assignment_id,
        const vector<string>& employee_ids,
        const string& vehicle_id,
        const string& shift_id,
        const string& assignment_date,
        const string& assignment_status
    );

    // Getter / Setter
    string getAssignment_id() const;
    void setAssignment_id(const string& assignment_id);

    // Quan ly danh sach nhan vien
    const vector<string>& getEmployee_ids() const;

    int getEmployeeCount() const;
    string getEmployee_id(int index) const;

    bool hasEmployee(const string& employee_id) const;
    bool addEmployee(const string& employee_id);
    bool removeEmployee(const string& employee_id);

    // Quan ly phuong tien
    string getVehicle_id() const;
    void setVehicle_id(const string& vehicle_id);

    // Quan ly ca lam
    string getShift_id() const;
    void setShift_id(const string& shift_id);

    // Quan ly ngay lam
    string getAssignment_date() const;
    void setAssignment_date(const string& assignment_date);

    // Quan ly trang thai
    string getAssignment_status() const;
    void setAssignment_status(const string& assignment_status);

    static bool isValidStatus(const string& status);

    // Hien thi
    void Show() const;
};
