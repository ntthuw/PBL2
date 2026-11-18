
#include "Manager.h"
#include <stdexcept>

// Constructor mac dinh
Manager::Manager()
    : Employee()
{
    position = "Manager";
}

// Constructor moi
Manager::Manager(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address
)
    : Employee(
        employee_id,
        employee_name,
        employee_address,
        "Manager"
    )
{
    validate();
}

// Constructor tuong thich voi code cu
Manager::Manager(
    const string& employee_id,
    const string& employee_name,
    const string& employee_address,
    long long employee_salary
)
    : Manager(
        employee_id,
        employee_name,
        employee_address
    )
{
    if (employee_salary != calculate_salary())
    {
        throw invalid_argument(
            "Manager base salary must be 16000000 VND!"
        );
    }
}

// Clone
Employee* Manager::clone() const
{
    return new Manager(*this);
}

// Luong co ban Manager
long long Manager::calculate_salary() const
{
    return 16000000LL;
}

// Kiem tra du lieu Manager
void Manager::validate() const
{
    Employee::validate();

    if (position != "Manager")
    {
        throw invalid_argument("Invalid Manager position!");
    }
}

// Hien thi
void Manager::Show() const
{
    Employee::Show();
}
