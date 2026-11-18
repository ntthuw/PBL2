
#pragma once

#include "Employee.h"

class Manager : public Employee
{
public:
    // Constructor
    Manager();

    Manager(
        const string& employee_id,
        const string& employee_name,
        const string& employee_address
    );

    // Constructor tuong thich voi cach khoi tao cu.
    // Salary truyen vao phai bang luong co dinh cua Manager.
    Manager(
        const string& employee_id,
        const string& employee_name,
        const string& employee_address,
        long long employee_salary
    );

    // Ham ao
    Employee* clone() const override;
    long long calculate_salary() const override;
    void validate() const override;
    void Show() const override;
};
