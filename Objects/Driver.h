
#pragma once

#include "Employee.h"

class Driver : public Employee
{
public:
    // Constructor
    Driver();

    Driver(
        const string& employee_id,
        const string& employee_name,
        const string& employee_address
    );

    // Constructor tuong thich voi cach khoi tao cu.
    // Salary truyen vao phai bang luong co dinh cua Driver.
    Driver(
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
