#pragma once

#include <iostream>
#include <string>

using namespace std;

class Employee
{
protected:
    string employee_id;
    string employee_name;
    string employee_address;
    string position;
    string employee_status;

public:
    // Constructor
    Employee();

    Employee(
        const string& employee_id,
        const string& employee_name,
        const string& employee_address,
        const string& position
    );

    // Destructor
    virtual ~Employee();

    // Getter / Setter
    string get_id() const;
    void set_id(const string& employee_id);

    string get_name() const;
    void set_name(const string& employee_name);

    string get_address() const;
    void set_address(const string& employee_address);

    // Luong co ban co dinh theo vi tri
    long long get_salary() const;
    void set_salary(long long employee_salary);

    string get_position() const;

    // Trang thai nhan vien
    string get_status() const;
    void set_status(const string& employee_status);
    bool isActive() const;

    // Kiem tra du lieu
    virtual void validate() const;

    // Sao chep dung kieu doi tuong dan xuat
    virtual Employee* clone() const = 0;

    // Tra ve luong co ban, khong bao gom thuong theo thang
    virtual long long calculate_salary() const = 0;

    // Hien thi
    virtual void Show() const;
};
