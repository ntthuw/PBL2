
#include "Driver.h"
#include <stdexcept>

// Constructor mac dinh
Driver::Driver()
    : Employee()
{
    position = "Driver";
}

// Constructor moi
Driver::Driver(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address
)
    : Employee(
        employee_id,
        employee_name,
        employee_address,
        "Driver"
    )
{
    validate();
}

// Constructor tuong thich voi code cu
Driver::Driver(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address,
    long long employee_salary
)
    : Driver(
        employee_id,
        employee_name,
        employee_address
    )
{
    if (employee_salary != calculate_salary())
    {
        throw invalid_argument(
            "Driver base salary must be 12000000 VND!"
        );
    }
}

// Clone
Employee* Driver::clone() const
{
    return new Driver(*this);
}

// Luong co ban Driver
long long Driver::calculate_salary() const
{
    return 12000000LL;
}

// Kiem tra du lieu Driver
void Driver::validate() const
{
    Employee::validate();

    if (position != "Driver")
    {
        throw invalid_argument("Invalid Driver position!");
    }
}

// Hien thi
void Driver::Show() const
{
    Employee::Show();
}
